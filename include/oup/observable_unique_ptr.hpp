#ifndef OBSERVABLE_UNIQUE_PTR_INCLUDED
#define OBSERVABLE_UNIQUE_PTR_INCLUDED

#include <cstddef>
#include <type_traits>
#include <utility>
#include <new>

namespace oup {

template<typename T>
class observer_ptr;

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

namespace details {
template<typename T, typename Deleter = oup::default_delete<T>>
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
    *         if D is convertible to Deleter and U* is convertible to T*.
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
        observable_unique_ptr_base(manager.block, value) {
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
        observable_unique_ptr_base(manager.block, value, std::move(del)) {
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
    *         if D is convertible to Deleter and U* is convertible to T*.
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
    *   \note Using the return value of this function if has_deleter() returns 'false' will cause
    *         undefined behavior.
    */
    Deleter& get_deleter() noexcept {
        return ptr_deleter;
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    *   \note Using the return value of this function if has_deleter() returns 'false' will cause
    *         undefined behavior.
    */
    const Deleter& get_deleter() const noexcept {
        return ptr_deleter;
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observable_unique_ptr_base& other) noexcept {
        using std::swap;
        swap(block, other.block);
        swap(ptr_deleter, other.ptr_deleter);
    }

    /// Replaces the managed object with a null pointer.
    /** \param ptr A nullptr_t instance
    */
    void reset(std::nullptr_t ptr = nullptr) noexcept {
        if (ptr_deleter.data) {
            delete_and_pop_ref_();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* get() const noexcept {
        return ptr_deleter.data;
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if expired() is 'true' will leave to undefined behavior.
    */
    T& operator*() const noexcept {
        return *ptr_deleter.data;
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return ptr_deleter.data;
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return ptr_deleter.data != nullptr;
    }
};
}

/// Unique-ownership smart pointer, can be observed by observer_ptr, ownership can be released.
/** This smart pointer mimics the interface of std::unique_ptr, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object, but allows the ownership to
*   be released, e.g., to transfer the ownership to another owner.
*
*   The main difference with std::unique_ptr is that it allows creating
*   observer_ptr instances to observe the lifetime of the pointed object,
*   as one would do with std::shared_ptr and std::weak_ptr. The price to pay,
*   compared to a standard std::unique_ptr, is the additional heap allocation
*   of the reference-counting control block. Because observable_unique_ptr
*   can be released (see release()), this cannot be optimized. If releasing
*   is not a needed feature, consider using observable_sealed_ptr instead.
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, observer_ptr cannot extend
*      the lifetime of the pointed object, hence observable_unique_ptr provides
*      less thread-safety compared to std::shared_ptr.
*    - observable_unique_ptr does not support arrays.
*    - observable_unique_ptr does not allow custom allocators.
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

    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr;

    // Friend is required for CRTP
    friend base;

public:
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
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value) :
        base(value != nullptr ? allocate_block_() : nullptr, value) {}

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param del The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value, Deleter del) :
        base(value != nullptr ? allocate_block_() : nullptr, value, std::move(del)) {}

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
    *         if D is convertible to Deleter and U* is convertible to T*.
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
    *         is default constructed.
    */
    template<typename U, typename D>
    observable_unique_ptr(observable_unique_ptr<U,D>&& manager, T* value) noexcept :
        base(std::move(manager), value) {}

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \param del The deleter to use in the new pointer
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U, typename D>
    observable_unique_ptr(observable_unique_ptr<U,D>&& manager, T* value, Deleter del) noexcept :
        base(std::move(manager), value, del) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_unique_ptr& operator=(observable_unique_ptr&& value) noexcept {
        base::operator=(std::move(value));
        return *this;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership. The source deleter
    *         is moved. This operator only takes part in overload resolution
    *         if D is convertible to Deleter and U* is convertible to T*.
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
    /** \param ptr A nullptr_t instance
    */
    void reset(T* ptr) noexcept {
        // Copy old pointer
        T* old_ptr = base::ptr_deleter.data;
        control_block_type* old_block = base::block;

        // Assign the new one
        base::block = ptr != nullptr ? allocate_block_() : nullptr;
        base::ptr_deleter.data = ptr;

        // Delete the old pointer
        // (this follows std::unique_ptr specs)
        if (old_ptr) {
            delete_and_pop_ref_(old_block, old_ptr, base::ptr_deleter);
        }
    }

    /// Releases ownership of the managed object and mark observers as expired.
    /** \return A pointer to the un-managed object
    *   \note The returned pointer, if not nullptr, becomes owned by the caller and
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

/// Unique-ownership smart pointer, can be observed by observer_ptr, ownership cannot be released
/** This smart pointer mimics the interface of std::unique_ptr, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object. Once ownership is acquired, it
*   cannot be released. If this becomes necessary, consider using observable_unique_ptr
*   instead.
*
*   The main difference with std::unique_ptr is that it allows creating
*   observer_ptr instances to observe the lifetime of the pointed object,
*   as one would do with std::shared_ptr and std::weak_ptr. The price to pay,
*   compared to a standard std::unique_ptr, is the additional heap allocation
*   of the reference-counting control block, which make_observable_sealed()
*   will optimize as a single heap allocation with the pointed object (as
*   std::make_shared() does for std::shared_ptr).
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, observer_ptr cannot extend
*      the lifetime of the pointed object, hence observable_sealed_ptr provides
*      less thread-safety compared to std::shared_ptr.
*    - observable_sealed_ptr does not support arrays.
*    - observable_sealed_ptr does not allow custom allocators.
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
    observable_sealed_ptr(control_block_type* ctrl, T* value) noexcept :
        base(ctrl, value, oup::placement_delete<T>{}) {}

    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    // Friendship is required for conversions.
    template<typename U>
    friend class observable_sealed_ptr;

    // Friend is required for CRTP
    friend base;

public:
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
    /** \note If you need to do this, use observable_unique_ptr instead.
    */
    explicit observable_sealed_ptr(T*) = delete;

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_sealed_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_sealed_ptr(observable_sealed_ptr&& value) noexcept = default;

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_sealed_ptr is created, the source
    *         pointer is set to null and looses ownership. This constructor
    *         only takes part in overload resolution if U* is convertible to T*.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observable_sealed_ptr(observable_sealed_ptr<U>&& value) noexcept :
        base(std::move(value)) {}

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this observable_sealed_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_sealed_ptr(observable_sealed_ptr<U>&& manager, T* value) noexcept :
        base(std::move(manager), value) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_sealed_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_sealed_ptr& operator=(observable_sealed_ptr&& value) noexcept {
        base::operator=(std::move(value));
        return *this;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_sealed_ptr is created, the source
    *         pointer is set to null and looses ownership. This operator only takes
    *         part in overload resolution if U* is convertible to T*.
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

/// Create a new observable_unique_ptr with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new observable_unique_ptr
*   \note Custom deleters are not supported by this function. If you require
*         a custom deleter, please use the observable_unique_ptr constructors
*         directly. Compared to make_observable_sealed(), this function
*         does not allocate the pointed object and the control block in a single
*         buffer, as that would prevent writing observable_unique_ptr::release().
*         If releasing the pointer is not needed, consider using observable_sealed_ptr
*         instead.
*/
template<typename T, typename ... Args>
observable_unique_ptr<T> make_observable_unique(Args&& ... args) {
    return observable_unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/// Create a new observable_sealed_ptr with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new observable_sealed_ptr
*   \note This function is the only way to create an observable_sealed_ptr.
*         It will allocate the pointed object and the control block in a
*         single buffer for better performance.
*/
template<typename T, typename ... Args>
observable_sealed_ptr<T> make_observable_sealed(Args&& ... args) {
    using block_type = typename observable_sealed_ptr<T>::control_block_type;

    // Allocate memory
    constexpr std::size_t block_size = sizeof(block_type);
    constexpr std::size_t object_size = sizeof(T);
    std::byte* buffer = new std::byte[block_size + object_size];

    try {
        // Construct control block and object
        block_type* block = new (buffer) block_type;
        T* ptr = new (buffer + block_size) T(std::forward<Args>(args)...);

        // Make owner pointer
        return observable_sealed_ptr<T>(block, ptr);
    } catch (...) {
        // Exception thrown during object construction,
        // clean up memory and let exception propagate
        delete[] buffer;
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

/// Non-owning smart pointer that observes an observable_unique_ptr or an observable_sealed_ptr.
/** \see observable_unique_ptr
*   \see observable_sealed_ptr
*/
template<typename T>
class observer_ptr {
private:
    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    using control_block = details::control_block;

    control_block* block = nullptr;
    T* data = nullptr;

    void pop_ref_() noexcept {
        --block->refcount;
        if (block->refcount == 0) {
            delete block;
        }
    }

public:
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

    /// Create a weak pointer from an owning pointer.
    template<typename U, typename D, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(const observable_unique_ptr<U,D>& owner) noexcept :
        block(owner.block), data(owner.ptr_deleter.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Create a weak pointer from an owning pointer.
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(const observable_sealed_ptr<U>& owner) noexcept :
        block(owner.block), data(owner.ptr_deleter.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    observer_ptr(const observer_ptr& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(const observer_ptr<U>& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After this observer_ptr is created, the source
    *         pointer is set to null.
    */
    observer_ptr(observer_ptr&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After this observer_ptr is created, the source
    *         pointer is set to null. This constructor only takes part in
    *         overload resolution if D is convertible to Deleter and U* is
    *         convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr(observer_ptr<U>&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Point to another owning pointer.
    /** \param owner The new owner pointer to observe
    *   \note This operator only takes part in  overload resolution if D
    *         is convertible to Deleter and U* is convertible to T*.
    */
    template<typename U, typename D, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(const observable_unique_ptr<U,D>& owner) noexcept {
        if (data) {
            pop_ref_();
        }

        block = owner.block;
        data = owner.ptr_deleter.data;
        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Point to another owning pointer.
    /** \param owner The new owner pointer to observe
    *   \note This operator only takes part in  overload resolution if D
    *         is convertible to Deleter and U* is convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(const observable_sealed_ptr<U>& owner) noexcept {
        if (data) {
            pop_ref_();
        }

        block = owner.block;
        data = owner.ptr_deleter.data;
        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    observer_ptr& operator=(const observer_ptr& value) noexcept {
        if (data) {
            pop_ref_();
        }

        block = value.block;
        data = value.data;
        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    *   \note This operator only takes part in overload resolution if D
    *         is convertible to Deleter and U* is convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(const observer_ptr<U>& value) noexcept {
        if (data) {
            pop_ref_();
        }

        block = value.block;
        data = value.data;
        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source
    *         pointer is set to null and looses ownership.
    */
    observer_ptr& operator=(observer_ptr&& value) noexcept {
        if (data) {
            pop_ref_();
        }

        block = value.block;
        value.block = nullptr;
        data = value.data;
        value.data = nullptr;

        return *this;
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source
    *         pointer is set to null and looses ownership.
    *         This operator only takes part in overload resolution if D
    *         is convertible to Deleter and U* is convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    observer_ptr& operator=(observer_ptr<U>&& value) noexcept {
        if (data) {
            pop_ref_();
        }

        block = value.block;
        value.block = nullptr;
        data = value.data;
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

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
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
    *   \note Using this function if expired() is 'true' will leave to undefined behavior.
    */
    T& operator*() const noexcept {
        return *get();
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return get();
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    bool expired() const noexcept {
        return block == nullptr || block->expired();
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return block != nullptr && !block->expired();
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observer_ptr& other) noexcept {
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

}

#endif
