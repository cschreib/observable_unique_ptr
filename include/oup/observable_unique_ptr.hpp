#ifndef OBSERVABLE_UNIQUE_PTR_INCLUDED
#define OBSERVABLE_UNIQUE_PTR_INCLUDED

#include <cstddef>
#include <type_traits>
#include <utility>
#include <new>
#include <exception>

namespace oup {

/// Exception thrown for failed observer_from_this().
struct bad_observer_from_this : std::exception {
    const char* what() const noexcept override {
        return "observer_from_this() called with uninitialised control block";
    }
};

template<typename T, typename Policy>
class basic_observer_ptr;

template<typename T, typename Policy>
class basic_enable_observer_from_this;

namespace details {
    template<typename Policy>
    struct control_block {
        using refcount_type = typename Policy::refcount_type;

        enum flag_elements {
            flag_none = 0,
            flag_expired = 1
        };

        refcount_type refcount = 1;
        int flags = flag_none;

        void add_ref() noexcept { ++refcount; }

        void remove_ref() noexcept { --refcount; }

        bool has_no_ref() const noexcept { return refcount == 0; }

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

    // Constructor tag.
    struct acquire_tag {};
}

/// Simple default deleter
/** \note This is almost identical to std::default_delete, but prevents including the "memory"
*         header. A key difference is that the deleter is not templated; it's call operator
*         is templated, which means that the same deleter (type) can be used for any pointer type.
*/
struct default_delete
{
    template<typename T>
    void operator()(T* p) const {
        static_assert(!std::is_same_v<T, void>, "cannot delete a pointer to an incomplete type");
        static_assert(sizeof(T) > 0, "cannot delete a pointer to an incomplete type");
        delete p;
    }
};

/// Deleter for data allocated with placement new
struct placement_delete
{
    template<typename T>
    void operator()(T* p) const {
        static_assert(!std::is_same_v<T, void>, "cannot delete a pointer to an incomplete type");
        static_assert(sizeof(T) > 0, "cannot delete a pointer to an incomplete type");
        p->~T();
    }
};

/// Default control block policy
/** This defines the behavior and implementation details of the control block.
*   The control block is the object that takes care of counting the number of
*   existing observer pointers, and whether the observed pointer has expired.
*/
struct default_control_block_policy {
    using refcount_type = int;
};

/// Unique ownership (with release) policy
/** \see observable_unique_ptr
*/
struct unique_policy {
    static constexpr bool allow_release = true;
    using control_block_policy = default_control_block_policy;
};

/// Unique ownership (without release) policy
/** \see observable_sealed_ptr
*/
struct sealed_policy {
    static constexpr bool allow_release = false;
    using control_block_policy = default_control_block_policy;
};

template<typename T, typename Policy>
class basic_enable_observer_from_this;

namespace details {
    template<typename Policy>
    struct enable_observer_from_this_base {
        using control_block = details::control_block<typename Policy::control_block_policy>;

        mutable control_block* this_control_block = nullptr;

        virtual ~enable_observer_from_this_base() {
            if (this_control_block) {
                pop_ref_();
                this_control_block = nullptr;
            }
        }

        void pop_ref_() noexcept {
            this_control_block->remove_ref();
            if (this_control_block->has_no_ref()) {
                delete this_control_block;
            }
        }

        void set_control_block_(control_block* b) noexcept {
            if (b == this_control_block || this_control_block != nullptr) {
                return;
            }

            this_control_block = b;

            if (this_control_block) {
                this_control_block->add_ref();
            }
        }

        // Friendship is required for assignment of the observer.
        template<typename U, typename D>
        friend class basic_observable_ptr;
    };
}

/// Generic class for observable owning pointers.
/** This is a generic class, configurable with policies. See @ref observable_unique_ptr and
*   @ref observable_sealed_ptr for more user-friendly usage and pre-configured policies.
*   The available policies are:
*    - Policy::allow_release: This must evaluate to a constexpr boolean value, which is
*       `true` if a raw pointer can be acquired and released from this smart pointer, or
*       `false` if a raw pointer is forever "sealed" into this smart pointer. When this
*       policy is set to `false`, the control block and the managed object are allocated
*       separately into a single buffer.
*    - Policy::control_block_policy::refcount_type: This must be an integer type
*       (signed or unsigned) holding the number of observer references. The larger the
*       type, the more concurrent references to the same object can exist, but the larger
*       the memory overhead.
*
*   This smart pointer is meant to be used alongside @ref basic_observer_ptr, which is able
*   to observe the lifetime of the stored raw pointer, without ownership.
*/
template<typename T, typename Deleter, typename Policy>
class basic_observable_ptr {
protected:
    using control_block_policy = typename Policy::control_block_policy;
    using control_block_type = details::control_block<control_block_policy>;

    control_block_type* block = nullptr;
    details::ptr_and_deleter<T, Deleter> ptr_deleter;

    static control_block_type* allocate_block_() {
        return new control_block_type;
    }

    static void pop_ref_(control_block_type* block) noexcept {
        block->remove_ref();
        if (block->has_no_ref()) {
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

    /// Fill in the observer pointer for objects inheriting from `basic_enable_observer_from_this`.
    /** \note It is important to preserve the type of the pointer as supplied by the user.
    *         It might be of a derived type that inherits from `basic_enable_observer_from_this`, while
    *         the base type `T` might not. We still want to fill in the observer pointer if we can.
    */
    template<typename U>
    void set_this_observer_(U* p) noexcept {
        if constexpr (std::is_convertible_v<U*,const details::enable_observer_from_this_base<Policy>*>) {
            if (p) {
                p->set_control_block_(block);
            }
        }
    }

    /// Fill in the observer pointer for objects inheriting from `basic_enable_observer_from_this`.
    /** \note It is important to preserve the type of the pointer as supplied by the user.
    *         It might be of a derived type that inherits from `basic_enable_observer_from_this`, while
    *         the base type `T` might not. We still want to fill in the observer pointer if we can.
    */
    void reset_this_observer_() noexcept {
        if constexpr (std::is_convertible_v<T*,const details::enable_observer_from_this_base<Policy>*>) {
            ptr_deleter.data->set_control_block_(nullptr);
        }
    }

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \note This is used by make_observable_unique().
    */
    template<typename U>
    basic_observable_ptr(control_block_type* ctrl, U* value) noexcept :
        block(ctrl), ptr_deleter{Deleter{}, value} {}

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \param del The deleter to use
    *   \note This is used by make_observable_sealed().
    */
    template<typename U>
    basic_observable_ptr(control_block_type* ctrl, U* value, Deleter del) noexcept :
        block(ctrl), ptr_deleter{std::move(del), value} {}

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \note This is used by make_observable_unique().
    */
    template<typename U>
    basic_observable_ptr(details::acquire_tag, control_block_type* ctrl, U* value) noexcept :
        block(ctrl), ptr_deleter{Deleter{}, value} {
        set_this_observer_(value);
    }

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \param del The deleter to use
    *   \note This is used by make_observable_sealed().
    */
    template<typename U>
    basic_observable_ptr(details::acquire_tag, control_block_type* ctrl, U* value, Deleter del) noexcept :
        block(ctrl), ptr_deleter{std::move(del), value} {
        set_this_observer_(value);
    }

    // Friendship is required for conversions.
    template<typename U, typename P>
    friend class basic_observer_ptr;

    // Friendship is required for conversions.
    template<typename U, typename D, typename P>
    friend class basic_observable_ptr;

public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    /// Type of the pointed object
    using element_type = T;

    /// Type of the matching observer pointer
    using observer_type = basic_observer_ptr<T, control_block_policy>;

    /// Pointer type
    using pointer = element_type*;

    /// Deleter type
    using deleter_type = Deleter;

    /// Default constructor (null pointer).
    basic_observable_ptr() noexcept = default;

    /// Construct a null pointer.
    basic_observable_ptr(std::nullptr_t) noexcept {}

    /// Construct a null pointer with custom deleter.
    basic_observable_ptr(std::nullptr_t, Deleter deleter) noexcept :
        ptr_deleter{std::move(deleter), nullptr} {}

    /// Destructor, releases owned object if any
    ~basic_observable_ptr() noexcept {
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
    basic_observable_ptr(basic_observable_ptr&& value) noexcept :
        basic_observable_ptr(value.block, value.ptr_deleter.data, std::move(value.ptr_deleter)) {
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
    template<typename U, typename D, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_convertible_v<D, Deleter>>>
    basic_observable_ptr(basic_observable_ptr<U,D,Policy>&& value) noexcept :
        basic_observable_ptr(value.block, value.ptr_deleter.data, std::move(value.ptr_deleter)) {
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
    template<typename U, typename D, typename V, typename enable =
        std::enable_if_t<std::is_convertible_v<V*,T*>>>
    basic_observable_ptr(basic_observable_ptr<U,D,Policy>&& manager, V* value) noexcept :
        basic_observable_ptr(details::acquire_tag{},
            value != nullptr ? manager.block : nullptr, value) {

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
    template<typename U, typename D, typename V, typename enable =
        std::enable_if_t<std::is_convertible_v<V*,T*>>>
    basic_observable_ptr(basic_observable_ptr<U,D,Policy>&& manager, V* value, Deleter del) noexcept :
        basic_observable_ptr(details::acquire_tag{},
            value != nullptr ? manager.block : nullptr, value, std::move(del)) {

        if (manager.ptr_deleter.data != nullptr && value == nullptr) {
            manager.delete_and_pop_ref_();
        }

        manager.block = nullptr;
        manager.ptr_deleter.data = nullptr;
    }

    /// Explicit ownership capture of a raw pointer.
    /** \param value The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after the
    *         `observable_unique_ptr` is created. If possible, prefer
    *         using `make_observable_unique()` instead of this constructor.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*,T*> && Policy::allow_release>>
    explicit basic_observable_ptr(U* value) :
        basic_observable_ptr(details::acquire_tag{},
            value != nullptr ? allocate_block_() : nullptr, value) {}

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param del The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         `basic_observable_ptr` is created. If possible, prefer
    *         using `make_observable_unique()` instead of this constructor.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*,T*> && Policy::allow_release>>
    explicit basic_observable_ptr(U* value, Deleter del) :
        basic_observable_ptr(details::acquire_tag{},
            value != nullptr ? allocate_block_() : nullptr, value, std::move(del)) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership.
    */
    basic_observable_ptr& operator=(basic_observable_ptr&& value) noexcept {
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
    template<typename U, typename D, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_convertible_v<D, Deleter>>>
    basic_observable_ptr& operator=(basic_observable_ptr<U,D,Policy>&& value) noexcept {
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
    basic_observable_ptr(const basic_observable_ptr&) = delete;
    basic_observable_ptr& operator=(const basic_observable_ptr&) = delete;

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
    void swap(basic_observable_ptr& other) noexcept {
        if (&other == this) {
            return;
        }

        using std::swap;
        swap(block, other.block);
        swap(ptr_deleter, other.ptr_deleter);
    }

    /// Replaces the managed object.
    /** \param ptr The new object to manage (can be `nullptr`, then this is equivalent to `reset()`)
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*,T*> && Policy::allow_release>>
    void reset(U* ptr) {
        // Copy old pointer
        T* old_ptr = ptr_deleter.data;
        control_block_type* old_block = block;

        // Assign the new one
        block = ptr != nullptr ? allocate_block_() : nullptr;
        ptr_deleter.data = ptr;

        // Delete the old pointer
        // (this follows `std::unique_ptr` specs)
        if (old_ptr) {
            delete_and_pop_ref_(old_block, old_ptr, ptr_deleter);
        }

        set_this_observer_(ptr);
    }

    /// Replaces the managed object with a null pointer.
    /** \param ptr A `nullptr_t` instance
    */
    void reset(std::nullptr_t ptr = nullptr) noexcept {
        static_cast<void>(ptr); // silence "unused variable" warnings

        if (ptr_deleter.data) {
            delete_and_pop_ref_();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }
    }

    /// Releases ownership of the managed object and mark observers as expired.
    /** \return A pointer to the un-managed object
    *   \note The returned pointer, if not `nullptr`, becomes owned by the caller and
    *         must be either manually deleted, or managed by another shared pointer.
    *         Existing observer pointers will see the object as expired.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && Policy::allow_release>>
    T* release() noexcept {
        T* old_ptr = ptr_deleter.data;
        if (ptr_deleter.data) {
            reset_this_observer_();
            block->set_expired();

            pop_ref_();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }

        return old_ptr;
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
    explicit operator bool() const noexcept {
        return ptr_deleter.data != nullptr;
    }

    template<typename U, typename P, typename ... Args>
    friend auto make_observable(Args&& ... args);
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
template<typename T, typename Policy, typename ... Args>
auto make_observable(Args&& ... args) {
    if constexpr (Policy::allow_release) {
        return basic_observable_ptr<T, default_delete, Policy>(
            new T(std::forward<Args>(args)...));
    } else {
        using control_block_policy = typename Policy::control_block_policy;
        using block_type = details::control_block<control_block_policy>;
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
            return basic_observable_ptr<T, placement_delete, Policy>(
                details::acquire_tag{}, block, ptr);
        } catch (...) {
            // Exception thrown during object construction,
            // clean up memory and let exception propagate
            delete buffer;
            throw;
        }
    }
}

template<typename T, typename Deleter, typename Policy>
bool operator== (const basic_observable_ptr<T,Deleter,Policy>& value, std::nullptr_t) noexcept {
    return value.get() == nullptr;
}

template<typename T, typename Deleter, typename Policy>
bool operator== (std::nullptr_t, const basic_observable_ptr<T,Deleter,Policy>& value) noexcept {
    return value.get() == nullptr;
}

template<typename T, typename Deleter, typename Policy>
bool operator!= (const basic_observable_ptr<T,Deleter,Policy>& value, std::nullptr_t) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename Deleter, typename Policy>
bool operator!= (std::nullptr_t, const basic_observable_ptr<T,Deleter,Policy>& value) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename U, typename Deleter, typename Policy>
bool operator== (const basic_observable_ptr<T,Deleter,Policy>& first,
    const basic_observable_ptr<U,Deleter,Policy>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U, typename Deleter, typename Policy>
bool operator!= (const basic_observable_ptr<T,Deleter,Policy>& first,
    const basic_observable_ptr<U,Deleter,Policy>& second) noexcept {
    return first.get() != second.get();
}

/// Non-owning smart pointer that observes a @ref basic_observable_ptr.
template<typename T, typename Policy>
class basic_observer_ptr {
private:
    // Friendship is required for conversions.
    template<typename U, typename P>
    friend class basic_observer_ptr;
    // Friendship is required for basic_enable_observer_from_this.
    template<typename U, typename D, typename P>
    friend class basic_observable_ptr;
    // Friendship is required for basic_enable_observer_from_this.
    template<typename U, typename P>
    friend class basic_enable_observer_from_this;

    using control_block = details::control_block<Policy>;

    control_block* block = nullptr;
    T* data = nullptr;

    void pop_ref_() noexcept {
        block->remove_ref();
        if (block->has_no_ref()) {
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

    // For basic_enable_observer_from_this
    basic_observer_ptr(control_block* b, T* d) noexcept : block(b), data(d) {
        if (block) {
            block->add_ref();
        }
    }

public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    /// Type of the pointed object
    using element_type = T;

    /// Default constructor (null pointer).
    basic_observer_ptr() = default;

    /// Default constructor (null pointer).
    basic_observer_ptr(std::nullptr_t) noexcept {}

    /// Destructor
    ~basic_observer_ptr() noexcept {
        if (data) {
            pop_ref_();
            block = nullptr;
            data = nullptr;
        }
    }

    /// Create an observer pointer from an owning pointer.
    template<typename U, typename D, typename P, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_same_v<Policy, typename P::control_block_policy>>>
    basic_observer_ptr(const basic_observable_ptr<U,D,P>& owner) noexcept :
        block(owner.block), data(owner.ptr_deleter.data) {
        if (block) {
            block->add_ref();
        }
    }

    /// Copy an existing `basic_observer_ptr` instance
    /** \param value The existing observer pointer to copy
    */
    basic_observer_ptr(const basic_observer_ptr& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            block->add_ref();
        }
    }

    /// Copy an existing `basic_observer_ptr` instance
    /** \param value The existing observer pointer to copy
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr(const basic_observer_ptr<U,Policy>& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            block->add_ref();
        }
    }

    /// Copy an existing `basic_observer_ptr` instance with explicit casting
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
    basic_observer_ptr(const basic_observer_ptr<U,Policy>& manager, T* value) noexcept :
        block(value != nullptr ? manager.block : nullptr), data(value) {
        if (block) {
            block->add_ref();
        }
    }

    /// Move from an existing `basic_observer_ptr` instance
    /** \param value The existing observer pointer to move from
    *   \note After this `basic_observer_ptr` is created, the source
    *         pointer is set to null.
    */
    basic_observer_ptr(basic_observer_ptr&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing `basic_observer_ptr` instance
    /** \param value The existing observer pointer to move from
    *   \note After this `basic_observer_ptr` is created, the source
    *         pointer is set to null. This constructor only takes part in
    *         overload resolution if U* is convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr(basic_observer_ptr<U,Policy>&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing `basic_observer_ptr` instance with explicit casting
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
    basic_observer_ptr(basic_observer_ptr<U,Policy>&& manager, T* value) noexcept :
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
    basic_observer_ptr& operator=(const basic_observable_ptr<U,D,Policy>& owner) noexcept {
        set_data_(owner.block, owner.ptr_deleter.data);

        if (block) {
            block->add_ref();
        }

        return *this;
    }

    /// Copy an existing `basic_observer_ptr` instance
    /** \param value The existing weak pointer to copy
    */
    basic_observer_ptr& operator=(const basic_observer_ptr& value) noexcept {
        if (&value == this) {
            return *this;
        }

        set_data_(value.block, value.data);

        if (block) {
            block->add_ref();
        }

        return *this;
    }

    /// Copy an existing `basic_observer_ptr` instance
    /** \param value The existing weak pointer to copy
    *   \note This operator only takes part in overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr& operator=(const basic_observer_ptr<U,Policy>& value) noexcept {
        set_data_(value.block, value.data);

        if (block) {
            block->add_ref();
        }

        return *this;
    }

    /// Move from an existing `basic_observer_ptr` instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source pointer is set to null.
    */
    basic_observer_ptr& operator=(basic_observer_ptr&& value) noexcept {
        set_data_(value.block, value.data);

        value.block = nullptr;
        value.data = nullptr;

        return *this;
    }

    /// Move from an existing `basic_observer_ptr` instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source pointer is set to null.
    *         This operator only takes part in overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr& operator=(basic_observer_ptr<U,Policy>&& value) noexcept {
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
    explicit operator bool() const noexcept {
        return block != nullptr && !block->expired();
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(basic_observer_ptr& other) noexcept {
        if (&other == this) {
            return;
        }

        using std::swap;
        swap(block, other.block);
        swap(data, other.data);
    }
};


template<typename T, typename Policy>
bool operator== (const basic_observer_ptr<T,Policy>& value, std::nullptr_t) noexcept {
    return value.expired();
}

template<typename T, typename Policy>
bool operator== (std::nullptr_t, const basic_observer_ptr<T,Policy>& value) noexcept {
    return value.expired();
}

template<typename T, typename Policy>
bool operator!= (const basic_observer_ptr<T,Policy>& value, std::nullptr_t) noexcept {
    return !value.expired();
}

template<typename T, typename Policy>
bool operator!= (std::nullptr_t, const basic_observer_ptr<T,Policy>& value) noexcept {
    return !value.expired();
}

template<typename T, typename U, typename Policy, typename enable =
    std::enable_if_t<std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>>>
bool operator== (const basic_observer_ptr<T,Policy>& first, const basic_observer_ptr<U,Policy>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U, typename Policy, typename enable =
    std::enable_if_t<std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>>>
bool operator!= (const basic_observer_ptr<T,Policy>& first, const basic_observer_ptr<U,Policy>& second) noexcept {
    return first.get() != second.get();
}


/// Enables creating an observer pointer from `this`.
/** If an object must be able to create an observer pointer to itself,
*   without having direct access to the owner pointer (unique or sealed),
*   then the object's class can inherit from basic_enable_observer_from_this.
*   This provides the `observer_from_this()` member function, which returns
*   a new observer pointer to the object. For this mechanism to work,
*   the class must inherit publicly from basic_enable_observer_from_this,
*   and the object must be owned by a unique or sealed pointer when
*   calling observer_from_this(). If the latter condition is not satisfied,
*   i.e., the object was allocated on the stack, or is owned by another
*   type of smart pointer, then `observer_from_this()` will return nullptr.
*
*   **Corner cases.**
*    - If a class `A` inherits from both another class `B` and `basic_enable_observer_from_this<A,...>`,
*      and it is owned by a `basic_observable_ptr<B,...>`. The function `observer_from_this()`
*      return a valid pointer if ownership is acquired from a raw `A*`, but will throw
*      if ownership is acquired from a raw `B*`. Therefore, make sure to always acquire ownership
*      on the most derived type, or simply use the factory function `make_observable()` which will
*      enforce this automatically.
*
*      ```
*           struct B {
*               virtual ~B() = default;
*           };
*
*           struct A : B, basic_enable_observer_from_this<A,Policy> {};
*
*
*           basic_observable_ptr<B,Deleter,Policy> good1(new A);
*           dynamic_cast<A*>(good1.get())->observer_from_this(); // valid A*
*
*           observable_unique_ptr<B> good2(make_observable_unique<A>());
*           dynamic_cast<A*>(good2.get())->observer_from_this(); // valid A*
*
*           // Bad: do not do this
*           A* a = new A;
*           B* b = a;
*           observable_unique_ptr<B> bad(b);
*           dynamic_cast<A*>(bad.get())->observer_from_this(); // throws bad_observer_from_this
*      ```
*
*    - Multiple inheritance. If a class `A` inherits from both another class `B` and
*      `basic_enable_observer_from_this<A,...>`, and if `B` also inherits from
*      `basic_enable_observer_from_this<B,...>`, then `observer_from_this()` will be an ambiguous
*      call. But it can be resolved, and (contrary to `std::shared_ptr` and
*      `std::enable_shared_from_this`) will return a valid pointer:
*
*      ```
*           struct B : basic_enable_observer_from_this<B,Policy> {
*               virtual ~B() = default;
*           };
*
*           struct A : B, basic_enable_observer_from_this<A,Policy> {};
*
*           basic_observable_ptr<A,Deleter,Policy> ptr(new A);
*           ptr->basic_enable_observer_from_this<A,Policy>::observer_from_this(); // valid A*
*           ptr->basic_enable_observer_from_this<B,Policy>::observer_from_this(); // valid B*
*      ```
*/
template<typename T, typename Policy>
class basic_enable_observer_from_this : public virtual details::enable_observer_from_this_base<Policy> {
    using control_block_policy = typename Policy::control_block_policy;

protected:
    basic_enable_observer_from_this() noexcept = default;

    basic_enable_observer_from_this(const basic_enable_observer_from_this&) noexcept {
        // Do not copy the other object's observer, this would be an
        // invalid reference.
    };

    basic_enable_observer_from_this(basic_enable_observer_from_this&&) noexcept {
        // Do not move the other object's observer, this would be an
        // invalid reference.
    };

    basic_enable_observer_from_this& operator=(const basic_enable_observer_from_this&) noexcept {
        // Do not copy the other object's observer, this would be an
        // invalid reference.
        return *this;
    };

    basic_enable_observer_from_this& operator=(basic_enable_observer_from_this&&) noexcept {
        // Do not move the other object's observer, this would be an
        // invalid reference.
        return *this;
    };

public:
    /// Type of observer pointers.
    using observer_type = basic_observer_ptr<T, control_block_policy>;
    /// Type of observer pointers (const).
    using const_observer_type = basic_observer_ptr<const T, control_block_policy>;

    /// Return an observer pointer to 'this'.
    /** \return A new observer pointer pointing to 'this'.
    *   \note If 'this' is not owned by a unique or sealed pointer, i.e., if
    *   the object was allocated on the stack, or if it is owned by another
    *   type of smart pointer, then this function will return nullptr.
    */
    observer_type observer_from_this() {
        static_assert(std::is_base_of_v<basic_enable_observer_from_this,std::decay_t<T>>,
            "T must inherit from basic_enable_observer_from_this<T>");

        if (!this->this_control_block) {
            throw bad_observer_from_this{};
        }

        return observer_type{this->this_control_block, static_cast<T*>(this)};
    }

    /// Return a const observer pointer to 'this'.
    /** \return A new observer pointer pointing to 'this'.
    *   \note If 'this' is not owned by a unique or sealed pointer, i.e., if
    *   the object was allocated on the stack, or if it is owned by another
    *   type of smart pointer, then this function will return nullptr.
    */
    const_observer_type observer_from_this() const {
        static_assert(std::is_base_of_v<basic_enable_observer_from_this,std::decay_t<T>>,
            "T must inherit from basic_enable_observer_from_this<T>");

        if (!this->this_control_block) {
            throw bad_observer_from_this{};
        }

        return const_observer_type{this->this_control_block, static_cast<const T*>(this)};
    }
};


/// Perform a `static_cast` for an `observable_unique_ptr`.
/** \param ptr The pointer to cast
*   \note Ownership will be transfered to the returned pointer.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename D, typename P>
basic_observable_ptr<U,D,P> static_pointer_cast(basic_observable_ptr<T,D,P>&& ptr) {
    return basic_observable_ptr<U,D,P>(std::move(ptr), static_cast<U*>(ptr.get()));
}

/// Perform a `static_cast` for an `basic_observer_ptr`.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is not modified.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> static_pointer_cast(const basic_observer_ptr<T,Policy>& ptr) {
    // NB: can use raw_get() as static cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(ptr, static_cast<U*>(ptr.raw_get()));
}

/// Perform a `static_cast` for an `basic_observer_ptr`.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is set to null.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> static_pointer_cast(basic_observer_ptr<T,Policy>&& ptr) {
    // NB: can use raw_get() as static cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(std::move(ptr), static_cast<U*>(ptr.raw_get()));
}

/// Perform a `const_cast` for an `observable_unique_ptr`.
/** \param ptr The pointer to cast
*   \note Ownership will be transfered to the returned pointer.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename D, typename P>
basic_observable_ptr<U,D,P> const_pointer_cast(basic_observable_ptr<T,D,P>&& ptr) {
    return basic_observable_ptr<U,D,P>(std::move(ptr), const_cast<U*>(ptr.get()));
}

/// Perform a `const_cast` for an `basic_observer_ptr`.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is not modified.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> const_pointer_cast(const basic_observer_ptr<T,Policy>& ptr) {
    // NB: can use raw_get() as const cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(ptr, const_cast<U*>(ptr.raw_get()));
}

/// Perform a `const_cast` for an `basic_observer_ptr`.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is set to null.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> const_pointer_cast(basic_observer_ptr<T,Policy>&& ptr) {
    // NB: can use raw_get() as const cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(std::move(ptr), const_cast<U*>(ptr.raw_get()));
}

/// Perform a `dynamic_cast` for an `observable_unique_ptr`.
/** \param ptr The pointer to cast
*   \note Ownership will be transfered to the returned pointer unless the cast
*         fails, in which case ownership remains in the original pointer, std::bad_cast
*         is thrown, and no memory is leaked. If the input pointer is null,
*         the output pointer will also be null.
*/
template<typename U, typename T, typename D, typename P>
basic_observable_ptr<U,D,P> dynamic_pointer_cast(basic_observable_ptr<T,D,P>&& ptr) {
    if (ptr == nullptr) {
        return basic_observable_ptr<U,D,P>{};
    }

    U& casted_object = dynamic_cast<U&>(*ptr.get());
    return basic_observable_ptr<U,D,P>(std::move(ptr), &casted_object);
}

/// Perform a `dynamic_cast` for an `basic_observer_ptr`.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is not modified.
          If the input pointer is null, or if the cast fails, the output pointer
          will be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> dynamic_pointer_cast(const basic_observer_ptr<T,Policy>& ptr) {
    // NB: must use get() as dynamic cast of an expired pointer is UB
    return basic_observer_ptr<U,Policy>(ptr, dynamic_cast<U*>(ptr.get()));
}

/// Perform a `dynamic_cast` for an `basic_observer_ptr`.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is set to null.
          If the input pointer is null, or if the cast fails, the output pointer
          will be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> dynamic_pointer_cast(basic_observer_ptr<T,Policy>&& ptr) {
    // NB: must use get() as dynamic cast of an expired pointer is UB
    return basic_observer_ptr<U,Policy>(std::move(ptr), dynamic_cast<U*>(ptr.get()));
}

/// Unique-ownership smart pointer, can be observed by @ref observer_ptr, ownership can be released.
/** This smart pointer mimics the interface of `std::unique_ptr`, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object, but allows the ownership to
*   be released, e.g., to transfer the ownership to another owner.
*
*   The main difference with `std::unique_ptr` is that it allows creating
*   `basic_observer_ptr` instances to observe the lifetime of the pointed object,
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
template<typename T, typename Deleter = default_delete>
using observable_unique_ptr = basic_observable_ptr<T, Deleter, unique_policy>;

/// Unique-ownership smart pointer, can be observed by @ref observer_ptr, ownership cannot be released.
/** This smart pointer mimics the interface of `std::unique_ptr`, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object. Once ownership is acquired, it
*   cannot be released. If this becomes necessary, consider using observable_unique_ptr
*   instead.
*
*   The main difference with `std::unique_ptr` is that it allows creating
*   `basic_observer_ptr` instances to observe the lifetime of the pointed object,
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
using observable_sealed_ptr = basic_observable_ptr<T, placement_delete, sealed_policy>;

/// Non-owning smart pointer that observes a @ref observable_sealed_ptr or @ref observable_unique_ptr.
template<typename T>
using observer_ptr = basic_observer_ptr<T, default_control_block_policy>;

template<typename T>
using enable_observer_from_this_unique = basic_enable_observer_from_this<T, unique_policy>;

template<typename T>
using enable_observer_from_this_sealed = basic_enable_observer_from_this<T, sealed_policy>;

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
    return make_observable<T, unique_policy>(std::forward<Args>(args)...);
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
    return make_observable<T, sealed_policy>(std::forward<Args>(args)...);
}


}

#endif
