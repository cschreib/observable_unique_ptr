#ifndef OBSERVABLE_UNIQUE_PTR_INCLUDED
#define OBSERVABLE_UNIQUE_PTR_INCLUDED

#include <cstddef>
#include <type_traits>
#include <utility>
#include <new>

namespace oup {

template<typename T>
class observer_ptr;

template<typename T>
class enable_observer_from_this;

namespace details {

struct control_block {
    enum flag_elements {
        flag_none = 0,
        flag_expired = 1
    };

    int refcount = 1;
    int flags = flag_none;

    bool expired() const noexcept { return flags & flag_expired; }

    void set_expired() noexcept { flags = flags | flag_expired; }
};

// This class enables optimizing the space taken by the Deleter object
// when the deleter is stateless (has no member variable). It relies
// on empty base class optimization. In C++20, this could be simplified
// by [[no_unique_address]].
template<typename T, typename Deleter>
struct ptr_and_deleter : Deleter {
    T* data = nullptr;
};

}

/// Simple default deleter
/** \note This is identical to std::default_delete, but prevents including the "memory" header.
*/
template<typename T>
struct default_delete
{
    /// Default constructor
    constexpr default_delete() noexcept = default;

    /// Converting constructor.
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    default_delete(const default_delete<U>&) noexcept {}

    void operator()(T* p) const {
        static_assert(!std::is_same_v<T, void>, "cannot delete a pointer to an incomplete type");
        static_assert(sizeof(T) > 0, "cannot delete a pointer to an incomplete type");
        delete p;
    }
};

/// Deleter for data allocated with placement new
template<typename T>
struct placement_delete
{
    /// Default constructor
    constexpr placement_delete() noexcept = default;

    /// Converting constructor.
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    placement_delete(const placement_delete<U>&) noexcept {}

    void operator()(T* p) const {
        static_assert(!std::is_same_v<T, void>, "cannot delete a pointer to an incomplete type");
        static_assert(sizeof(T) > 0, "cannot delete a pointer to an incomplete type");
        p->~T();
    }
};

template<typename T>
class enable_observer_from_this;

namespace details {

struct enable_observer_from_this_base {
    using control_block = details::control_block;

    mutable control_block* this_control_block = nullptr;

    void pop_ref_() noexcept {
        --this_control_block->refcount;
        if (this_control_block->refcount == 0) {
            delete this_control_block;
        }
    }

    void set_control_block_(control_block* b) noexcept {
        if (this_control_block) {
            pop_ref_();
        }

        this_control_block = b;
        if (this_control_block) {
            ++this_control_block->refcount;
        }
    }

    // Friendship is required for assignment of the observer.
    template<typename U, typename D>
    friend class observable_unique_ptr_base;
};

template<typename T, typename Deleter>
class observable_unique_ptr_base {
protected:
    using control_block_type = details::control_block;

    control_block_type* block = nullptr;
    details::ptr_and_deleter<T, Deleter> ptr_deleter;

    static void pop_ref_(control_block_type* block) noexcept {
        --block->refcount;
        if (block->refcount == 0) {
            delete block;
        }
    }

    static void delete_and_pop_ref_(control_block_type* block, T* data, Deleter& deleter) noexcept {
        deleter(data);

        block->set_expired();

        pop_ref_(block);
    }

    void pop_ref_() noexcept {
        pop_ref_(block);
    }

    void delete_and_pop_ref_() noexcept {
        delete_and_pop_ref_(block, ptr_deleter.data, ptr_deleter);
    }

    /// Fill in the observer pointer for objects inheriting from `enable_observer_from_this`.
    /** \note It is important to preserve the type of the pointer as supplied by the user.
    *         It might be of a derived type that inherits from `enable_observer_from_this`, while
    *         the base type `T` might not. We still want to fill in the observer pointer if we can.
    */
    template<typename U>
    void set_this_observer_(U* p) noexcept {
        if constexpr (std::is_convertible_v<U*,const details::enable_observer_from_this_base*>) {
            if (p) {
                p->this_control_block = block;
                ++block->refcount;
            }
        }
    }

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \note This is used by make_observable_unique().
    */
    observable_unique_ptr_base(control_block_type* ctrl, T* value) noexcept :
        block(ctrl), ptr_deleter{Deleter{}, value} {}

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \param del The deleter to use
    *   \note This is used by make_observable_sealed().
    */
    observable_unique_ptr_base(control_block_type* ctrl, T* value, Deleter del) noexcept :
        block(ctrl), ptr_deleter{std::move(del), value} {}

    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr_base;

    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr;

    // Friendship is required for conversions.
    template<typename U>
    friend class observable_sealed_ptr;

public:
    /// Type of the pointed object
    using element_type = T;

    /// Type of the matching observer pointer
    using observer_type = observer_ptr<T>;

    /// Pointer type
    using pointer = element_type*;

    /// Deleter type
    using deleter_type = Deleter;

    /// Default constructor (null pointer).
    observable_unique_ptr_base() noexcept = default;

    /// Construct a null pointer.
    observable_unique_ptr_base(std::nullptr_t) noexcept {}

    /// Construct a null pointer with custom deleter.
    observable_unique_ptr_base(std::nullptr_t, Deleter deleter) noexcept :
        ptr_deleter{std::move(deleter), nullptr} {}

    /// Destructor, releases owned object if any
    ~observable_unique_ptr_base() noexcept {
        if (ptr_deleter.data) {
            delete_and_pop_ref_();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved.
    */
    observable_unique_ptr_base(observable_unique_ptr_base&& value) noexcept :
        observable_unique_ptr_base(value.block, value.ptr_deleter.data, std::move(value.ptr_deleter)) {
        value.block = nullptr;
        value.ptr_deleter.data = nullptr;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved. This constructor only takes part in overload resolution
    *         if `D` is convertible to `Deleter` and `U*` is convertible to `T*`.
    */
    template<typename U, typename D>
    observable_unique_ptr_base(observable_unique_ptr_base<U,D>&& value) noexcept :
        observable_unique_ptr_base(value.block, value.ptr_deleter.data, std::move(value.ptr_deleter)) {
        value.block = nullptr;
        value.ptr_deleter.data = nullptr;
    }

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership. The deleter
    *         is default constructed.
    */
    template<typename U, typename D>
    observable_unique_ptr_base(observable_unique_ptr_base<U,D>&& manager, T* value) noexcept :
        observable_unique_ptr_base(value != nullptr ? manager.block : nullptr, value) {
        if (manager.ptr_deleter.data != nullptr && value == nullptr) {
            manager.delete_and_pop_ref_();
        }

        manager.block = nullptr;
        manager.ptr_deleter.data = nullptr;
    }

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \param del The deleter to use in the new pointer
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U, typename D>
    observable_unique_ptr_base(observable_unique_ptr_base<U,D>&& manager, T* value, Deleter del) noexcept :
        observable_unique_ptr_base(value != nullptr ? manager.block : nullptr, value, std::move(del)) {
        if (manager.ptr_deleter.data != nullptr && value == nullptr) {
            manager.delete_and_pop_ref_();
        }

        manager.block = nullptr;
        manager.ptr_deleter.data = nullptr;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_unique_ptr_base& operator=(observable_unique_ptr_base&& value) noexcept {
        if (ptr_deleter.data) {
            delete_and_pop_ref_();
        }

        block = value.block;
        value.block = nullptr;
        ptr_deleter.data = value.ptr_deleter.data;
        value.ptr_deleter.data = nullptr;
        static_cast<Deleter&>(ptr_deleter) = std::move(static_cast<Deleter&>(value.ptr_deleter));

        return *this;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved. This operator only takes part in overload resolution
    *         if `D` is convertible to `Deleter` and `U*` is convertible to `T*`.
    */
    template<typename U, typename D>
    observable_unique_ptr_base& operator=(observable_unique_ptr_base<U,D>&& value) noexcept {
        if (ptr_deleter.data) {
            delete_and_pop_ref_();
        }

        block = value.block;
        value.block = nullptr;
        ptr_deleter.data = value.ptr_deleter.data;
        value.ptr_deleter.data = nullptr;
        static_cast<Deleter&>(ptr_deleter) = std::move(static_cast<Deleter&>(ptr_deleter));

        return *this;
    }

    // Non-copyable
    observable_unique_ptr_base(const observable_unique_ptr_base&) = delete;
    observable_unique_ptr_base& operator=(const observable_unique_ptr_base&) = delete;

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    */
    Deleter& get_deleter() noexcept {
        return ptr_deleter;
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    */
    const Deleter& get_deleter() const noexcept {
        return ptr_deleter;
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observable_unique_ptr_base& other) noexcept {
        if (&other == this) {
            return;
        }

        using std::swap;
        swap(block, other.block);
        swap(ptr_deleter, other.ptr_deleter);
    }

    /// Replaces the managed object with a null pointer.
    /** \param ptr A `nullptr_t` instance
    */
    void reset(std::nullptr_t ptr = nullptr) noexcept {
        if (ptr_deleter.data) {
            delete_and_pop_ref_();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return `nullptr` if `expired()` is `true`, or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object.
    *         Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset until
    *         you are done using the raw pointer.
    */
    T* get() const noexcept {
        return ptr_deleter.data;
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if `expired()` is `true` will lead to undefined behavior.
    *         This does not extend the lifetime of the pointed object.
    *         Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset until
    *         you are done using the raw pointer.
    */
    T& operator*() const noexcept {
        return *ptr_deleter.data;
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return `nullptr` if no object is owned, or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object.
    *         Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return ptr_deleter.data;
    }

    /// Check if this pointer points to a valid object.
    /** \return `true` if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return ptr_deleter.data != nullptr;
    }
};

}

/// Unique-ownership smart pointer, can be observed by `observer_ptr`, ownership can be released.
/** This smart pointer mimics the interface of `std::unique_ptr`, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object, but allows the ownership to
*   be released, e.g., to transfer the ownership to another owner.
*
*   The main difference with `std::unique_ptr` is that it allows creating
*   `observer_ptr` instances to observe the lifetime of the pointed object,
*   as one would do with `std::shared_ptr` and `std::weak_ptr`. The price to pay,
*   compared to a standard `std::unique_ptr`, is the additional heap allocation
*   of the reference-counting control block. Because `observable_unique_ptr`
*   can be released (see `release()`), this cannot be optimized. If releasing
*   is not a needed feature, consider using `observable_sealed_ptr` instead.
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, `observer_ptr` cannot extend
*      the lifetime of the pointed object, hence `observable_unique_ptr` provides
*      less thread-safety compared to std::shared_ptr.
*    - `observable_unique_ptr` does not support arrays.
*    - `observable_unique_ptr` does not allow custom allocators.
*/
template<typename T, typename Deleter = oup::default_delete<T>>
class observable_unique_ptr :
    details::observable_unique_ptr_base<T,Deleter> {
private:
    using base = details::observable_unique_ptr_base<T,Deleter>;
    using control_block_type = typename base::control_block_type;

    static control_block_type* allocate_block_() {
        return new control_block_type;
    }

    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr;

    // Friend is required for CRTP
    friend base;

public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    using typename base::element_type;
    using typename base::observer_type;
    using typename base::pointer;
    using typename base::deleter_type;

    /// Default constructor (null pointer).
    observable_unique_ptr() noexcept = default;

    /// Construct a null pointer.
    observable_unique_ptr(std::nullptr_t) noexcept : base() {}

    /// Construct a null pointer with custom deleter.
    observable_unique_ptr(std::nullptr_t, Deleter deleter) noexcept :
        base(nullptr, std::move(deleter)) {}

    /// Destructor, releases owned object if any
    ~observable_unique_ptr() noexcept = default;

    /// Explicit ownership capture of a raw pointer.
    /** \param value The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after the
    *         `observable_unique_ptr` is created. If possible, prefer
    *         using `make_observable_unique()` instead of this constructor.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*,T*>>>
    explicit observable_unique_ptr(U* value) :
        base(value != nullptr ? allocate_block_() : nullptr, value) {
        base::set_this_observer_(value);
    }

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param del The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         `observable_unique_ptr` is created. If possible, prefer
    *         using `make_observable_unique()` instead of this constructor.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*,T*>>>
    explicit observable_unique_ptr(U* value, Deleter del) :
        base(value != nullptr ? allocate_block_() : nullptr, value, std::move(del)) {
        base::set_this_observer_(value);
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved.
    */
    observable_unique_ptr(observable_unique_ptr&& value) noexcept = default;

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved. This constructor only takes part in overload resolution
    *         if `D` is convertible to `Deleter` and `U*` is convertible to `T*`.
    */
    template<typename U, typename D, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_convertible_v<D, Deleter>>>
    observable_unique_ptr(observable_unique_ptr<U,D>&& value) noexcept :
        base(std::move(value)) {}

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership. The deleter
    *         is default constructed. The raw pointer `value`
    *         must be obtained by casting the raw pointer managed by `manager`
    *         (const cast, dynamic cast, etc), such that calling `value` has
    *         the same effect as deleting the pointer owned by `manager`.
    */
    template<typename U, typename D, typename V, typename enable =
        std::enable_if_t<std::is_convertible_v<V*,T*>>>
    observable_unique_ptr(observable_unique_ptr<U,D>&& manager, V* value) noexcept :
        base(std::move(manager), value) {
        base::set_this_observer_(value);
    }

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \param del The deleter to use in the new pointer
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership. The raw pointer `value`
    *         must be obtained by casting the raw pointer managed by `manager`
    *         (const cast, dynamic cast, etc), such that deleting `value` has
    *         the same effect as deleting the pointer owned by `manager`.
    */
    template<typename U, typename D, typename V, typename enable =
        std::enable_if_t<std::is_convertible_v<V*,T*>>>
    observable_unique_ptr(observable_unique_ptr<U,D>&& manager, V* value, Deleter del) noexcept :
        base(std::move(manager), value, del) {
        base::set_this_observer_(value);
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this `observable_unique_ptr` is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_unique_ptr& operator=(observable_unique_ptr&& value) noexcept {
        base::operator=(std::move(value));
        return *this;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this `observable_unique_ptr` is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved. This operator only takes part in overload resolution
    *         if `D` is convertible to `Deleter` and `U*` is convertible to `T*`.
    */
    template<typename U, typename D, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_convertible_v<D, Deleter>>>
    observable_unique_ptr& operator=(observable_unique_ptr<U,D>&& value) noexcept {
        base::operator=(std::move(value));
        return *this;
    }

    // Non-copyable
    observable_unique_ptr(const observable_unique_ptr&) = delete;
    observable_unique_ptr& operator=(const observable_unique_ptr&) = delete;

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observable_unique_ptr& other) noexcept {
        base::swap(other);
    }

    using base::get_deleter;
    using base::reset;
    using base::get;
    using base::operator *;
    using base::operator ->;
    using base::operator bool;

    /// Replaces the managed object.
    /** \param ptr The new object to manage (can be `nullptr`, then this is equivalent to `reset()`)
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*,T*>>>
    void reset(U* ptr) {
        // Copy old pointer
        T* old_ptr = base::ptr_deleter.data;
        control_block_type* old_block = base::block;

        // Assign the new one
        base::block = ptr != nullptr ? allocate_block_() : nullptr;
        base::ptr_deleter.data = ptr;

        // Delete the old pointer
        // (this follows `std::unique_ptr` specs)
        if (old_ptr) {
            base::delete_and_pop_ref_(old_block, old_ptr, base::ptr_deleter);
        }

        base::set_this_observer_(ptr);
    }

    /// Releases ownership of the managed object and mark observers as expired.
    /** \return A pointer to the un-managed object
    *   \note The returned pointer, if not `nullptr`, becomes owned by the caller and
    *         must be either manually deleted, or managed by another shared pointer.
    *         Existing observer pointers will see the object as expired.
    */
    T* release() noexcept {
        T* old_ptr = base::ptr_deleter.data;
        if (base::ptr_deleter.data) {
            base::block->set_expired();

            base::pop_ref_();
            base::block = nullptr;
            base::ptr_deleter.data = nullptr;
        }

        return old_ptr;
    }

    template<typename U, typename ... Args>
    friend observable_unique_ptr<U> make_observable_unique(Args&& ... args);
};

/// Unique-ownership smart pointer, can be observed by `observer_ptr`, ownership cannot be released
/** This smart pointer mimics the interface of `std::unique_ptr`, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object. Once ownership is acquired, it
*   cannot be released. If this becomes necessary, consider using observable_unique_ptr
*   instead.
*
*   The main difference with `std::unique_ptr` is that it allows creating
*   `observer_ptr` instances to observe the lifetime of the pointed object,
*   as one would do with `std::shared_ptr` and `std::weak_ptr`. The price to pay,
*   compared to a standard `std::unique_ptr`, is the additional heap allocation
*   of the reference-counting control block, which `make_observable_sealed()`
*   will optimize as a single heap allocation with the pointed object (as
*   `std::make_shared()` does for `std::shared_ptr`).
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, `observer_ptr` cannot extend
*      the lifetime of the pointed object, hence `observable_sealed_ptr` provides
*      less thread-safety compared to `std::shared_ptr`.
*    - `observable_sealed_ptr` does not support arrays.
*    - `observable_sealed_ptr` does not allow custom allocators.
*/
template<typename T>
class observable_sealed_ptr :
    details::observable_unique_ptr_base<T,oup::placement_delete<T>> {
private:
    using base = details::observable_unique_ptr_base<T,oup::placement_delete<T>>;
    using control_block_type = typename base::control_block_type;

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \note This is used by make_observable_sealed().
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*,T*>>>
    observable_sealed_ptr(control_block_type* ctrl, U* value) noexcept :
        base(ctrl, value, oup::placement_delete<T>{}) {
        base::set_this_observer_(value);
    }

    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    // Friendship is required for conversions.
    template<typename U>
    friend class observable_sealed_ptr;

    // Friend is required for CRTP
    friend base;

public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    using typename base::element_type;
    using typename base::observer_type;
    using typename base::pointer;
    using typename base::deleter_type;

    /// Default constructor (null pointer).
    observable_sealed_ptr() noexcept = default;

    /// Construct a null pointer.
    observable_sealed_ptr(std::nullptr_t) noexcept : base() {}

    /// Destructor, releases owned object if any
    ~observable_sealed_ptr() noexcept = default;

    /// Explicit ownership capture of a raw pointer is forbidden.
    /** \note If you need to do this, use `observable_unique_ptr` instead.
    */
    template<typename U>
    explicit observable_sealed_ptr(U*) = delete;

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this `observable_sealed_ptr` is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_sealed_ptr(observable_sealed_ptr&& value) noexcept = default;

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this `observable_sealed_ptr` is created, the source
    *         pointer is set to null and looses ownership. This constructor
    *         only takes part in overload resolution if `U*` is convertible to `T*`.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observable_sealed_ptr(observable_sealed_ptr<U>&& value) noexcept :
        base(std::move(value)) {}

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this `observable_sealed_ptr` is created, the source
    *         pointer is set to null and looses ownership. The raw pointer `value`
    *         must be obtained by casting the raw pointer managed by `manager`
    *         (const cast, dynamic cast, etc), such that deleting `value` has
    *         the same effect as deleting the pointer owned by `manager`.
    */
    template<typename U, typename V, typename enable =
        std::enable_if_t<std::is_convertible_v<V*,T*>>>
    observable_sealed_ptr(observable_sealed_ptr<U>&& manager, V* value) noexcept :
        base(std::move(manager), value) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this `observable_sealed_ptr` is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_sealed_ptr& operator=(observable_sealed_ptr&& value) noexcept {
        base::operator=(std::move(value));
        return *this;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this `observable_sealed_ptr` is created, the source
    *         pointer is set to null and looses ownership. This operator only takes
    *         part in overload resolution if `U*` is convertible to `T*`.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observable_sealed_ptr& operator=(observable_sealed_ptr<U>&& value) noexcept {
        base::operator=(std::move(value));
        return *this;
    }

    // Non-copyable
    observable_sealed_ptr(const observable_sealed_ptr&) = delete;
    observable_sealed_ptr& operator=(const observable_sealed_ptr&) = delete;

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observable_sealed_ptr& other) noexcept {
        base::swap(other);
    }

    using base::get_deleter;
    using base::reset;
    using base::get;
    using base::operator *;
    using base::operator ->;
    using base::operator bool;

    template<typename U, typename ... Args>
    friend observable_sealed_ptr<U> make_observable_sealed(Args&& ... args);
};

/// Create a new `observable_unique_ptr` with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new observable_unique_ptr
*   \note Custom deleters are not supported by this function. If you require
*         a custom deleter, please use the `observable_unique_ptr` constructors
*         directly. Compared to `make_observable_sealed()`, this function
*         does not allocate the pointed object and the control block in a single
*         buffer, as that would prevent writing `observable_unique_ptr::release()`.
*         If releasing the pointer is not needed, consider using `observable_sealed_ptr`
*         instead.
*/
template<typename T, typename ... Args>
observable_unique_ptr<T> make_observable_unique(Args&& ... args) {
    return observable_unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/// Create a new `observable_sealed_ptr` with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new observable_sealed_ptr
*   \note This function is the only way to create an `observable_sealed_ptr`.
*         It will allocate the pointed object and the control block in a
*         single buffer for better performance.
*/
template<typename T, typename ... Args>
observable_sealed_ptr<T> make_observable_sealed(Args&& ... args) {
    using block_type = typename observable_sealed_ptr<T>::control_block_type;
    using decayed_type = std::decay_t<T>;

    // Allocate memory
    constexpr std::size_t block_size = sizeof(block_type);
    constexpr std::size_t object_size = sizeof(T);
    std::byte* buffer = reinterpret_cast<std::byte*>(operator new(block_size + object_size));

    try {
        // Construct control block and object
        block_type* block = new (buffer) block_type;
        decayed_type* ptr = new (buffer + block_size) decayed_type(std::forward<Args>(args)...);

        // Make owner pointer
        return observable_sealed_ptr<T>(block, ptr);
    } catch (...) {
        // Exception thrown during object construction,
        // clean up memory and let exception propagate
        delete buffer;
        throw;
    }
}

template<typename T, typename Deleter>
bool operator== (const observable_unique_ptr<T,Deleter>& value, std::nullptr_t) noexcept {
    return value.get() == nullptr;
}

template<typename T, typename Deleter>
bool operator== (std::nullptr_t, const observable_unique_ptr<T,Deleter>& value) noexcept {
    return value.get() == nullptr;
}

template<typename T, typename Deleter>
bool operator!= (const observable_unique_ptr<T,Deleter>& value, std::nullptr_t) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename Deleter>
bool operator!= (std::nullptr_t, const observable_unique_ptr<T,Deleter>& value) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename U, typename Deleter>
bool operator== (const observable_unique_ptr<T,Deleter>& first,
    const observable_unique_ptr<U,Deleter>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U, typename Deleter>
bool operator!= (const observable_unique_ptr<T,Deleter>& first,
    const observable_unique_ptr<U,Deleter>& second) noexcept {
    return first.get() != second.get();
}

template<typename T>
bool operator== (const observable_sealed_ptr<T>& value, std::nullptr_t) noexcept {
    return value.get() == nullptr;
}

template<typename T>
bool operator== (std::nullptr_t, const observable_sealed_ptr<T>& value) noexcept {
    return value.get() == nullptr;
}

template<typename T>
bool operator!= (const observable_sealed_ptr<T>& value, std::nullptr_t) noexcept {
    return value.get() != nullptr;
}

template<typename T>
bool operator!= (std::nullptr_t, const observable_sealed_ptr<T>& value) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename U>
bool operator== (const observable_sealed_ptr<T>& first,
    const observable_sealed_ptr<U>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U>
bool operator!= (const observable_sealed_ptr<T>& first,
    const observable_sealed_ptr<U>& second) noexcept {
    return first.get() != second.get();
}

/// Non-owning smart pointer that observes an `observable_unique_ptr` or an `observable_sealed_ptr`.
/** \see observable_unique_ptr
*   \see observable_sealed_ptr
*/
template<typename T>
class observer_ptr {
private:
    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;
    // Friendship is required for enable_observer_from_this.
    template<typename U, typename D>
    friend class details::observable_unique_ptr_base;
    // Friendship is required for enable_observer_from_this.
    template<typename U>
    friend class enable_observer_from_this;

    using control_block = details::control_block;

    control_block* block = nullptr;
    T* data = nullptr;

    void pop_ref_() noexcept {
        --block->refcount;
        if (block->refcount == 0) {
            delete block;
        }
    }

    void set_data_(control_block* b, T* d) noexcept {
        if (data) {
            pop_ref_();
        }

        block = b;
        data = d;
    }

    // For enable_observer_from_this
    observer_ptr(control_block* b, T* d) noexcept : block(b), data(d) {
        if (block) {
            ++block->refcount;
        }
    }

public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    /// Type of the pointed object
    using element_type = T;

    /// Default constructor (null pointer).
    observer_ptr() = default;

    /// Default constructor (null pointer).
    observer_ptr(std::nullptr_t) noexcept {}

    /// Destructor
    ~observer_ptr() noexcept {
        if (data) {
            pop_ref_();
            block = nullptr;
            data = nullptr;
        }
    }

    /// Create an observer pointer from an owning pointer.
    template<typename U, typename D, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(const observable_unique_ptr<U,D>& owner) noexcept :
        block(owner.block), data(owner.ptr_deleter.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Create an observer pointer from an owning pointer.
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(const observable_sealed_ptr<U>& owner) noexcept :
        block(owner.block), data(owner.ptr_deleter.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Copy an existing `observer_ptr` instance
    /** \param value The existing observer pointer to copy
    */
    observer_ptr(const observer_ptr& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Copy an existing `observer_ptr` instance
    /** \param value The existing observer pointer to copy
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(const observer_ptr<U>& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Copy an existing `observer_ptr` instance with explicit casting
    /** \param manager The observer pointer to copy the observed data from
    *   \param value The casted pointer value to observe
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership. The deleter
    *         is default constructed. The raw pointer `value` may or may
    *         not be related to the raw pointer observed by `manager`.
    *         This could be a pointer to any other object which is known to
    *         have the same lifetime.
    */
    template<typename U>
    observer_ptr(const observer_ptr<U>& manager, T* value) noexcept :
        block(value != nullptr ? manager.block : nullptr), data(value) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Move from an existing `observer_ptr` instance
    /** \param value The existing observer pointer to move from
    *   \note After this `observer_ptr` is created, the source
    *         pointer is set to null.
    */
    observer_ptr(observer_ptr&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing `observer_ptr` instance
    /** \param value The existing observer pointer to move from
    *   \note After this `observer_ptr` is created, the source
    *         pointer is set to null. This constructor only takes part in
    *         overload resolution if U* is convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(observer_ptr<U>&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing `observer_ptr` instance with explicit casting
    /** \param manager The observer pointer to copy the observed data from
    *   \param value The casted pointer value to observe
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership. The deleter
    *         is default constructed. The raw pointer `value` may or may
    *         not be related to the raw pointer observed by `manager`.
    *         This could be a pointer to any other object which is known to
    *         have the same lifetime.
    */
    template<typename U>
    observer_ptr(observer_ptr<U>&& manager, T* value) noexcept :
        block(value != nullptr ? manager.block : nullptr), data(value) {
        if (manager.data != nullptr && value == nullptr) {
            manager.pop_ref_();
        }

        manager.block = nullptr;
        manager.data = nullptr;
    }

    /// Point to another owning pointer.
    /** \param owner The new owner pointer to observe
    *   \note This operator only takes part in  overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename D, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(const observable_unique_ptr<U,D>& owner) noexcept {
        set_data_(owner.block, owner.ptr_deleter.data);

        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Point to another owning pointer.
    /** \param owner The new owner pointer to observe
    *   \note This operator only takes part in  overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(const observable_sealed_ptr<U>& owner) noexcept {
        set_data_(owner.block, owner.ptr_deleter.data);

        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Copy an existing `observer_ptr` instance
    /** \param value The existing weak pointer to copy
    */
    observer_ptr& operator=(const observer_ptr& value) noexcept {
        if (&value == this) {
            return *this;
        }

        set_data_(value.block, value.data);

        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Copy an existing `observer_ptr` instance
    /** \param value The existing weak pointer to copy
    *   \note This operator only takes part in overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(const observer_ptr<U>& value) noexcept {
        if (&value == this) {
            return *this;
        }

        set_data_(value.block, value.data);

        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Move from an existing `observer_ptr` instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source pointer is set to null.
    */
    observer_ptr& operator=(observer_ptr&& value) noexcept {
        set_data_(value.block, value.data);

        value.block = nullptr;
        value.data = nullptr;

        return *this;
    }

    /// Move from an existing `observer_ptr` instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source pointer is set to null.
    *         This operator only takes part in overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(observer_ptr<U>&& value) noexcept {
        set_data_(value.block, value.data);

        value.block = nullptr;
        value.data = nullptr;

        return *this;
    }

    /// Set this pointer to null.
    void reset() noexcept {
        if (data) {
            pop_ref_();
            block = nullptr;
            data = nullptr;
        }
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return `nullptr` if `expired()` is `true`, or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning pointer
    *         will not be reset until you are done using the raw pointer.
    */
    T* get() const noexcept {
        return expired() ? nullptr : data;
    }

    /// Get a non-owning raw pointer to the pointed object, possibly dangling.
    /** \return The pointed object, which may be a dangling pointer if the object has been deleted
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning pointer
    *         will not be reset until you are done using the raw pointer. In addition,
    *         this function will not check if the pointer has expired (i.e., if the object
    *         has been deleted), so the returned pointer may be dangling.
    *         Only use this function if you know the object cannot have been deleted.
    */
    T* raw_get() const noexcept {
        return data;
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if `expired()` is `true` will lead to undefined behavior.
    */
    T& operator*() const noexcept {
        return *get();
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return `nullptr` if `expired()` is `true`, or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning pointer
    *         will not be reset until you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return get();
    }

    /// Check if this pointer points to a valid object.
    /** \return `true` if the pointed object is valid, 'false' otherwise
    */
    bool expired() const noexcept {
        return block == nullptr || block->expired();
    }

    /// Check if this pointer points to a valid object.
    /** \return `true` if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return block != nullptr && !block->expired();
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observer_ptr& other) noexcept {
        if (&other == this) {
            return;
        }

        using std::swap;
        swap(block, other.block);
        swap(data, other.data);
    }
};


template<typename T>
bool operator== (const observer_ptr<T>& value, std::nullptr_t) noexcept {
    return value.expired();
}

template<typename T>
bool operator== (std::nullptr_t, const observer_ptr<T>& value) noexcept {
    return value.expired();
}

template<typename T>
bool operator!= (const observer_ptr<T>& value, std::nullptr_t) noexcept {
    return !value.expired();
}

template<typename T>
bool operator!= (std::nullptr_t, const observer_ptr<T>& value) noexcept {
    return !value.expired();
}

template<typename T, typename U, typename enable =
    std::enable_if_t<std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>>>
bool operator== (const observer_ptr<T>& first, const observer_ptr<U>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U, typename enable =
    std::enable_if_t<std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>>>
bool operator!= (const observer_ptr<T>& first, const observer_ptr<U>& second) noexcept {
    return first.get() != second.get();
}

/// Enables creating an observer pointer from `this`.
/** If an object must be able to create an observer pointer to itself,
*   without having direct access to the owner pointer (unique or sealed),
*   then the object's class can inherit from enable_observer_from_this.
*   This provides the `observer_from_this()` member function, which returns
*   a new observer pointer to the object. For this mechanism to work,
*   the class must inherit publicly from enable_observer_from_this,
*   and the object must be owned by a unique or sealed pointer when
*   calling observer_from_this(). If the latter condition is not satisfied,
*   i.e., the object was allocated on the stack, or is owned by another
*   type of smart pointer, then `observer_from_this()` will return nullptr.
*
*   **Corner cases.**
*    - If a class `A` inherits from both another class `B` and `enable_observer_from_this<A>`,
*      and it is owned by an `observable_unique_ptr<B>`. The function `observer_from_this()`
*      will always return `nullptr` if ownership is acquired from a p`B*`, but will
*      return a valid pointer if ownership is acquired from a `A*`. Therefore,
*      make sure to always acquire ownership on the most derived type, or simply use the
*      factory function `make_observable_unique()` which will enforce this automatically.
*
*      ```
*           struct B {
*               virtual ~B() = default;
*           };
*
*           struct A : B, enable_observer_from_this<A> {};
*
*
*           observable_unique_ptr<B> good1(new A);
*           dynamic_cast<A*>(good1.get())->observer_from_this(); // valid A*
*
*           observable_unique_ptr<B> good2(make_observable_unique<A>());
*           dynamic_cast<A*>(good2.get())->observer_from_this(); // valid A*
*
*           // Bad: do not do this
*           observable_unique_ptr<B> bad(static_cast<B*>(new A));
*           dynamic_cast<A*>(bad.get())->observer_from_this(); // nullptr
*      ```
*
*    - Multiple inheritance. If a class `A` inherits from both another class `B` and
*      `enable_observer_from_this<A>`, and if `B` also inherits from
*      `enable_observer_from_this<B>`, then `observer_from_this()` will be an ambiguous
*      call. But it can be resolved, and (contrary to `std::shared_ptr` and
*      `std::enable_shared_from_this`) will return a valid pointer:
*
*      ```
*           struct B : enable_observer_from_this<B> {
*               virtual ~B() = default;
*           };
*
*           struct A : B, enable_observer_from_this<A> {};
*
*           observable_sealed_ptr<A> ptr = make_observable_sealed<A>();
*           ptr->enable_observer_from_this<A>::observer_from_this(); // valid A*
*           ptr->enable_observer_from_this<B>::observer_from_this(); // valid B*
*      ```
*/
template<typename T>
class enable_observer_from_this : public virtual details::enable_observer_from_this_base {
protected:
    enable_observer_from_this() noexcept = default;

    enable_observer_from_this(const enable_observer_from_this&) noexcept {
        // Do not copy the other object's observer, this would be an
        // invalid reference.
    };

    enable_observer_from_this(enable_observer_from_this&&) noexcept {
        // Do not move the other object's observer, this would be an
        // invalid reference.
    };

    ~enable_observer_from_this() noexcept {
        if (this_control_block) {
            pop_ref_();
        }

        this_control_block = nullptr;
    }

    enable_observer_from_this& operator=(const enable_observer_from_this&) noexcept {
        // Do not copy the other object's observer, this would be an
        // invalid reference.
        return *this;
    };

    enable_observer_from_this& operator=(enable_observer_from_this&&) noexcept {
        // Do not move the other object's observer, this would be an
        // invalid reference.
        return *this;
    };

public:

    using observer_element_type = T;

    /// Return an observer pointer to 'this'.
    /** \return A new observer pointer pointing to 'this'.
    *   \note If 'this' is not owned by a unique or sealed pointer, i.e., if
    *   the object was allocated on the stack, or if it is owned by another
    *   type of smart pointer, then this function will return nullptr.
    */
    observer_ptr<T> observer_from_this() {
        static_assert(std::is_base_of_v<enable_observer_from_this,std::decay_t<T>>,
            "T must inherit from enable_observer_from_this<T>");

        return observer_ptr<T>{this_control_block,
            this_control_block ? static_cast<T*>(this) : nullptr};
    }

    /// Return a const observer pointer to 'this'.
    /** \return A new observer pointer pointing to 'this'.
    *   \note If 'this' is not owned by a unique or sealed pointer, i.e., if
    *   the object was allocated on the stack, or if it is owned by another
    *   type of smart pointer, then this function will return nullptr.
    */
    observer_ptr<const T> observer_from_this() const {
        static_assert(std::is_base_of_v<enable_observer_from_this,std::decay_t<T>>,
            "T must inherit from enable_observer_from_this<T>");

        return observer_ptr<const T>{this_control_block,
            this_control_block ? static_cast<const T*>(this) : nullptr};
    }
};

}

#endif
