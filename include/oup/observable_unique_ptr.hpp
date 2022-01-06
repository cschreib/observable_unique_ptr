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

template<typename T, typename Deleter, typename Policy>
class basic_observable_ptr;

template<typename T, typename Policy>
class basic_observer_ptr;

template<typename T, typename Policy>
class basic_enable_observer_from_this;

template<typename T, typename Policy, typename ... Args>
auto make_observable(Args&& ... args);

namespace details {
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

/// Default observer policy
/** This defines the behavior and implementation details of observer pointers.
*   This includes the implementation of the control block. The control block is the object
*   that takes care of counting the number of existing observer pointers, and whether the
*   observed pointer has expired.
*/
struct default_observer_policy {
    using refcount_type = int; // TODO: change this into a request on max number of observer
};

/// Unique ownership (with release) policy
/** \see observable_unique_ptr
*/
struct unique_policy {
    static constexpr bool is_sealed = false;
    static constexpr bool allow_eoft_in_constructor = true;
    static constexpr bool allow_eoft_multiple_inheritance = true;
    static constexpr bool eoft_constructor_takes_control_block = false;
    using observer_policy = default_observer_policy;
};

/// Unique ownership (without release) policy
/** \see observable_sealed_ptr
*/
struct sealed_policy {
    static constexpr bool is_sealed = true;
    static constexpr bool allow_eoft_in_constructor = true;
    static constexpr bool allow_eoft_multiple_inheritance = true;
    static constexpr bool eoft_constructor_takes_control_block = true;
    using observer_policy = default_observer_policy;
};

template<typename Policy>
struct policy_queries {
    // Check for incompatibilities in policy
    static_assert(!(Policy::is_sealed && Policy::allow_eoft_in_constructor &&
        !Policy::eoft_constructor_takes_control_block),
        "enable_observer_from_this() must take a control block in its constructor if the "
        "policy is sealed and requires support for observer_from_this() in constructors.");

    static constexpr bool eoft_base_is_virtual() noexcept {
        return Policy::allow_eoft_multiple_inheritance &&
            !Policy::eoft_constructor_takes_control_block;
    }

    static constexpr bool eoft_base_constructor_needs_block() noexcept {
        return Policy::eoft_constructor_takes_control_block;
    }

    static constexpr bool eoft_constructor_allocates() noexcept {
        return !Policy::is_sealed && Policy::allow_eoft_in_constructor &&
            !Policy::eoft_constructor_takes_control_block;
    }

    static constexpr bool eoft_allow_default_constructor() noexcept {
        return !eoft_base_constructor_needs_block();
    }

    static constexpr bool owner_allow_release() noexcept {
        return !Policy::is_sealed;
    }

    static constexpr bool make_observer_single_allocation() noexcept {
        return Policy::is_sealed;
    }
};

namespace details {
    template<typename Policy>
    struct enable_observer_from_this_base;
}

/// Implementation-defined class holding reference counts and expired flag.
/** All the details about this class are private, and only accessible to
*   `oup::` classes. As a user, you may only use this class to forward it
*   to `oup::` classes as required.
*/
template<typename Policy>
class basic_control_block final {
    template<typename T, typename D, typename P>
    friend class oup::basic_observable_ptr;

    template<typename T, typename P>
    friend class oup::basic_observer_ptr;

    template<typename P>
    friend struct details::enable_observer_from_this_base;

    template<typename U, typename P, typename ... Args>
    friend auto oup::make_observable(Args&& ... args);

    using refcount_type = typename Policy::refcount_type;

    enum flag_elements {
        flag_none = 0,
        flag_expired = 1
    };

    refcount_type refcount = 1;
    int flags = flag_none;

    basic_control_block() noexcept = default;
    basic_control_block(const basic_control_block&) = delete;
    basic_control_block(basic_control_block&&) = delete;
    basic_control_block& operator=(const basic_control_block&) = delete;
    basic_control_block& operator=(basic_control_block&&) = delete;

    void push_ref() noexcept {
        ++refcount;
    }

    void pop_ref() noexcept {
        --refcount;
        if (has_no_ref()) {
            delete this;
        }
    }

    bool has_no_ref() const noexcept {
        return refcount == 0;
    }

    bool expired() const noexcept {
        return (flags & flag_expired) != 0;
    }

    void set_not_expired() noexcept {
        flags = flags & ~flag_expired;
    }

    void set_expired() noexcept {
        flags = flags | flag_expired;
    }
};

namespace details {

    template<typename Policy>
    struct enable_observer_from_this_base {
        /// Policy for the control block
        using observer_policy = typename Policy::observer_policy;

        /// Type of the control block
        using control_block_type = basic_control_block<observer_policy>;

        /// Policy query helper
        using queries = policy_queries<Policy>;

        mutable control_block_type* this_control_block = nullptr;

        enable_observer_from_this_base() noexcept(queries::eoft_constructor_allocates()) {
            if constexpr (queries::eoft_constructor_allocates()) {
                this_control_block = new control_block_type;
            }
        }

        explicit enable_observer_from_this_base(control_block_type& block) noexcept :
            this_control_block(&block) {
            block.push_ref();
        }

        enable_observer_from_this_base(const enable_observer_from_this_base&) = delete;
        enable_observer_from_this_base(enable_observer_from_this_base&&) = delete;
        enable_observer_from_this_base& operator=(const enable_observer_from_this_base&) = delete;
        enable_observer_from_this_base& operator=(enable_observer_from_this_base&&) = delete;

        virtual ~enable_observer_from_this_base() noexcept {
            if (this_control_block) {
                clear_control_block_();
            }
        }

    private:
        void set_control_block_(control_block_type* b) noexcept {
            this_control_block = b;
            this_control_block->push_ref();
        }

        void clear_control_block_() noexcept {
            this_control_block->set_expired();
            this_control_block->pop_ref();
            this_control_block = nullptr;
        }

        // Friendship is required for assignment of the observer.
        template<typename U, typename P, typename ... Args>
        friend auto oup::make_observable(Args&& ... args);
    };
}

/// Check if a given type T inherits from basic_enable_observer_from_this
// template<typename T, typename Policy>
// constexpr bool has_enable_observer_from_this = std::is_convertible_v<
//     T*, const details::enable_observer_from_this_base<Policy>*>;
template<typename T, typename Policy>
constexpr bool has_enable_observer_from_this = std::is_base_of_v<
    details::enable_observer_from_this_base<Policy>, T>;

/// Generic class for observable owning pointers.
/** This is a generic class, configurable with policies. See @ref observable_unique_ptr and
*   @ref observable_sealed_ptr for more user-friendly usage and pre-configured policies.
*   The available policies are:
*    - `Policy::is_sealed`: This must evaluate to a constexpr boolean value, which is
*       `false` if a raw pointer can be acquired and released from this smart pointer, or
*       `true` if a raw pointer is forever "sealed" into this smart pointer. When this
*       policy is set to `true`, the control block and the managed object are allocated
*       separately into a single buffer. This allows memory optimisations, but introduces
*       multiple limitations on the API (no @ref release or @ref reset, and cannot create
*       @ref observer_ptr from the object's constructor if inheriting from
*       @ref basic_enable_observer_from_this).
*    -  `Policy::is_eoft_base_virtual`: This must evaluate to a constexpr boolean value,
*       which is `true` if @ref basic_enable_observer_from_this is allowed to use virtual
*       inheritance in its implementation, and `false` otherwise. Using virtual inheritance
*       offers more convenience, such as natural support for multiple inheritance of
*       @ref basic_enable_observer_from_this.
*       TODO: split this policy into:
*         - allow_eoft_multiple_inheritance
*         - allow_eoft_in_constructor
*         - then define the rest from that; policies should dictate requirements on API, not impl
*    -  `Policy::observer_policy::refcount_type`: This must be an integer type
*       (signed or unsigned) holding the number of observer references. The larger the
*       type, the more concurrent references to the same object can exist, but the larger
*       the memory overhead.
*
*   This smart pointer is meant to be used alongside @ref basic_observer_ptr, which is able
*   to observe the lifetime of the stored raw pointer, without ownership.
*
*   \see observable_unique_ptr
*   \see observable_sealed_ptr
*   \see observer_ptr
*   \see basic_enable_observer_from_this
*   \see enable_observer_from_this_unique
*   \see enable_observer_from_this_sealed
*/
template<typename T, typename Deleter, typename Policy>
class basic_observable_ptr final {
public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    /// Policy for this smart pointer
    using policy = Policy;

    /// Policy query helper
    using queries = policy_queries<policy>;

    /// Policy for the control block
    using observer_policy = typename Policy::observer_policy;

    /// Type of the control block
    using control_block_type = basic_control_block<observer_policy>;

    /// Type of the pointed object
    using element_type = T;

    /// Type of the matching observer pointer
    using observer_type = basic_observer_ptr<T, observer_policy>;

    /// Pointer type
    using pointer = element_type*;

    /// Deleter type
    using deleter_type = Deleter;

protected:
    control_block_type* block = nullptr;
    details::ptr_and_deleter<T, Deleter> ptr_deleter;

    static control_block_type* allocate_block_() {
        return new control_block_type;
    }

    static void delete_object_(control_block_type* block, T* data, Deleter& deleter) noexcept {
        deleter(data);
        block->set_expired();
        block->pop_ref();
    }

    void delete_object_() noexcept {
        delete_object_(block, ptr_deleter.data, ptr_deleter);
    }

    void delete_object_if_exists_() noexcept {
        if (ptr_deleter.data) {
            delete_object_();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }
    }

    /// Decide whether to allocate a new control block or not.
    /** \note If the object inherits from @ref basic_enable_observer_from_this, and
    *         `Policy::is_sealed` is false (by construction this will always be the case when this
    *         function is called), then we can just use the control block pointer stored in the
    *         @ref basic_enable_observer_from_this base. Otherwise, we need to allocate a new one.
    */
    template<typename U>
    control_block_type* get_block_from_object_(U* p) {
        if (p == nullptr) {
            return nullptr;
        }

        if constexpr (queries::eoft_constructor_allocates() &&
                      has_enable_observer_from_this<U, Policy>) {
            p->this_control_block->push_ref();
            return p->this_control_block;
        }

        return allocate_block_();
    }

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    */
    template<typename U>
    basic_observable_ptr(control_block_type* ctrl, U* value) noexcept :
        block(ctrl), ptr_deleter{Deleter{}, value} {}

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \param del The deleter to use
    */
    template<typename U>
    basic_observable_ptr(control_block_type* ctrl, U* value, Deleter del) noexcept :
        block(ctrl), ptr_deleter{std::move(del), value} {}

    // Friendship is required for conversions.
    template<typename U, typename P>
    friend class basic_observer_ptr;

    // Friendship is required for conversions.
    template<typename U, typename D, typename P>
    friend class basic_observable_ptr;

public:

    /// Default constructor (null pointer).
    basic_observable_ptr() noexcept = default;

    /// Construct a null pointer.
    basic_observable_ptr(std::nullptr_t) noexcept {}

    /// Construct a null pointer with custom deleter.
    basic_observable_ptr(std::nullptr_t, Deleter deleter) noexcept :
        ptr_deleter{std::move(deleter), nullptr} {}

    /// Destructor, releases owned object if any
    ~basic_observable_ptr() noexcept {
        delete_object_if_exists_();
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
        basic_observable_ptr(value != nullptr ? manager.block : nullptr, value) {

        if (value == nullptr) {
            manager.delete_object_if_exists_();
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
        basic_observable_ptr(value != nullptr ? manager.block : nullptr, value, std::move(del)) {

        if (value == nullptr) {
            manager.delete_object_if_exists_();
        }

        manager.block = nullptr;
        manager.ptr_deleter.data = nullptr;
    }

    /// Explicit ownership capture of a raw pointer.
    /** \param value The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after the
    *         @ref observable_unique_ptr is created. If possible, prefer
    *         using @ref make_observable() instead of this constructor.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*,T*> && queries::owner_allow_release()>>
    explicit basic_observable_ptr(U* value) :
        basic_observable_ptr(get_block_from_object_(value), value) {}

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param del The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         @ref basic_observable_ptr is created. If possible, prefer
    *         using @ref make_observable() instead of this constructor.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*,T*> && queries::owner_allow_release()>>
    explicit basic_observable_ptr(U* value, Deleter del) :
        basic_observable_ptr(get_block_from_object_(value), value, std::move(del)) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this smart pointer is created, the source
    *         pointer is set to null and looses ownership.
    */
    basic_observable_ptr& operator=(basic_observable_ptr&& value) noexcept {
        delete_object_if_exists_();

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
        delete_object_if_exists_();

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
    /** \param ptr The new object to manage (can be `nullptr`, then this is equivalent to
    *              @ref reset())
    *   \note This function is enabled only if `Policy::is_sealed` is false.
    */
    template<typename U, typename enable =
        std::enable_if_t<std::is_convertible_v<U*,T*> && queries::owner_allow_release()>>
    void reset(U* ptr) {
        // Copy old pointer
        T* old_ptr = ptr_deleter.data;
        control_block_type* old_block = block;

        // Assign the new one
        block = get_block_from_object_(ptr);
        ptr_deleter.data = ptr;

        // Delete the old pointer
        // (this follows `std::unique_ptr` specs)
        if (old_ptr) {
            delete_object_(old_block, old_ptr, ptr_deleter);
        }
    }

    /// Replaces the managed object with a null pointer.
    /** \param ptr A `nullptr_t` instance
    */
    void reset(std::nullptr_t ptr = nullptr) noexcept {
        static_cast<void>(ptr); // silence "unused variable" warnings

        // Copy old pointer
        T* old_ptr = ptr_deleter.data;
        control_block_type* old_block = block;

        // Assign the new one
        block = nullptr;
        ptr_deleter.data = nullptr;

        // Delete the old pointer
        // (this follows `std::unique_ptr` specs)
        if (old_ptr) {
            delete_object_(old_block, old_ptr, ptr_deleter);
        }
    }

    /// Releases ownership of the managed object.
    /** \return A pointer to the un-managed object
    *   \note The returned pointer, if not `nullptr`, becomes owned by the caller and
    *         must be either manually deleted, or managed by another shared pointer.
    *         This function is enabled only if `Policy::is_sealed` is false.
    *         If the type `T` inherits from @ref basic_enable_observer_from_this,
    *         then existing existing observer pointers will still see the object as
    *         valid until the object is actually deleted by the caller. Otherwise,
    *         existing observer pointers will be immediately marked as expired.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && queries::owner_allow_release()>>
    T* release() noexcept {
        T* old_ptr = ptr_deleter.data;
        if (ptr_deleter.data) {
            if (!has_enable_observer_from_this<T, Policy>) {
                block->set_expired();
            }

            block->pop_ref();
            block = nullptr;
            ptr_deleter.data = nullptr;
        }

        return old_ptr;
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return A pointer to the owned object (or `nullptr` if none)
    *   \note This does not extend the lifetime of the pointed object.
    *         Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset or destroyed until
    *         you are done using the raw pointer.
    */
    T* get() const noexcept {
        return ptr_deleter.data;
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if this pointer owns no object will lead to undefined behavior.
    *   \note This does not extend the lifetime of the pointed object.
    *         Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset or destroyed until
    *         you are done using the raw pointer.
    */
    T& operator*() const noexcept {
        return *ptr_deleter.data;
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return A pointer to the owned object (or `nullptr` if none)
    *   \note This does not extend the lifetime of the pointed object.
    *         Therefore, when calling this function, you must
    *         make sure that the owning pointer will not be reset or destroyed until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return ptr_deleter.data;
    }

    /// Check if this pointer currently owns an object.
    /** \return `true` if an object is owned, 'false' otherwise
    */
    explicit operator bool() const noexcept {
        return ptr_deleter.data != nullptr;
    }

    template<typename U, typename P, typename ... Args>
    friend auto make_observable(Args&& ... args);
};


/// Create a new @ref basic_observable_ptr with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new basic_observable_ptr
*   \note Custom deleters are not supported by this function. If you require
*         a custom deleter, please use the `basic_observable_ptr` constructors
*         directly. If `Policy::is_sealed` is true, this function will allocate the
*         pointed object and the control block in a single buffer. Otherwise, they will be
*         allocated in separate buffers, as that would prevent writing
*         @ref basic_observable_ptr::release(). If releasing the pointer is not needed, consider
*         setting `Policy::is_sealed` to true.
*   \see make_observable_unique
*   \see make_observable_sealed
*/
template<typename T, typename Policy, typename ... Args>
auto make_observable(Args&& ... args) {
    using observer_policy = typename Policy::observer_policy;
    using control_block_type = basic_control_block<observer_policy>;
    using decayed_type = std::decay_t<T>;
    using queries = policy_queries<Policy>;

    if constexpr (!queries::make_observer_single_allocation()) {
        if constexpr (has_enable_observer_from_this<T, Policy> &&
                      queries::eoft_base_constructor_needs_block()) {
            // Allocate control block first
            control_block_type* block = new control_block_type;

            // Allocate object
            decayed_type* ptr = nullptr;
            try {
                ptr = new T(*block, std::forward<Args>(args)...);
            } catch (...) {
                delete block;
                throw;
            }

            return basic_observable_ptr<T, default_delete, Policy>(block, ptr);
        } else {
            return basic_observable_ptr<T, default_delete, Policy>(
                new T(std::forward<Args>(args)...));
        }
    } else {
        // Pre-allocate memory
        constexpr std::size_t block_size = sizeof(control_block_type);
        constexpr std::size_t object_size = sizeof(T);
        std::byte* buffer = reinterpret_cast<std::byte*>(operator new(block_size + object_size));

        try {
            // Construct control block first
            static_assert(!queries::eoft_constructor_allocates(), "library bug");
            control_block_type* block = new (buffer) control_block_type;

            // Construct object
            decayed_type* ptr = nullptr;
            if constexpr (has_enable_observer_from_this<T, Policy> &&
                          queries::eoft_base_constructor_needs_block()) {
                // The object as a constructor that can take a control block; just give it
                ptr = new (buffer + block_size) decayed_type(*block, std::forward<Args>(args)...);

                // Make owner pointer
                return basic_observable_ptr<T, placement_delete, Policy>(block, ptr);
            } else {
                ptr = new (buffer + block_size) decayed_type(std::forward<Args>(args)...);

                // Make owner pointer
                auto sptr = basic_observable_ptr<T, placement_delete, Policy>(block, ptr);

                if constexpr (has_enable_observer_from_this<T, Policy>) {
                    // Notify basic_enable_observer_from_this of the control
                    ptr->set_control_block_(block);
                }

                return sptr;
            }
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
/** \see observer_ptr
*   \see basic_observable_ptr
*   \see observable_unique_ptr
*   \see observable_sealed_ptr
*/
template<typename T, typename Policy>
class basic_observer_ptr final {
public:
    static_assert(!std::is_array_v<T>, "T[] is not supported");

    /// Policy for the control block
    using observer_policy = Policy;

    /// Type of the control block
    using control_block_type = basic_control_block<observer_policy>;

    /// Type of the pointed object
    using element_type = T;

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

    control_block_type* block = nullptr;
    T* data = nullptr;

    void set_data_(control_block_type* b, T* d) noexcept {
        if (data) {
            block->pop_ref();
        }

        block = b;
        data = d;
    }

    // For basic_enable_observer_from_this
    basic_observer_ptr(control_block_type* b, T* d) noexcept : block(b), data(d) {
        if (block) {
            block->push_ref();
        }
    }

public:
    /// Default constructor (null pointer).
    basic_observer_ptr() = default;

    /// Default constructor (null pointer).
    basic_observer_ptr(std::nullptr_t) noexcept {}

    /// Destructor
    ~basic_observer_ptr() noexcept {
        if (data) {
            block->pop_ref();
            block = nullptr;
            data = nullptr;
        }
    }

    /// Create an observer pointer from an owning pointer.
    template<typename U, typename D, typename P, typename enable =
        std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_same_v<Policy, typename P::observer_policy>>>
    basic_observer_ptr(const basic_observable_ptr<U,D,P>& owner) noexcept :
        block(owner.block), data(owner.ptr_deleter.data) {
        if (block) {
            block->push_ref();
        }
    }

    /// Copy an existing @ref basic_observer_ptr instance
    /** \param value The existing observer pointer to copy
    */
    basic_observer_ptr(const basic_observer_ptr& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            block->push_ref();
        }
    }

    /// Copy an existing @ref basic_observer_ptr instance
    /** \param value The existing observer pointer to copy
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr(const basic_observer_ptr<U,Policy>& value) noexcept :
        block(value.block), data(value.data) {
        if (block) {
            block->push_ref();
        }
    }

    /// Copy an existing @ref basic_observer_ptr instance with explicit casting
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
            block->push_ref();
        }
    }

    /// Move from an existing @ref basic_observer_ptr instance
    /** \param value The existing observer pointer to move from
    *   \note After this @ref basic_observer_ptr is created, the source
    *         pointer is set to null.
    */
    basic_observer_ptr(basic_observer_ptr&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing @ref basic_observer_ptr instance
    /** \param value The existing observer pointer to move from
    *   \note After this @ref basic_observer_ptr is created, the source
    *         pointer is set to null. This constructor only takes part in
    *         overload resolution if U* is convertible to T*.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr(basic_observer_ptr<U,Policy>&& value) noexcept : block(value.block), data(value.data) {
        value.block = nullptr;
        value.data = nullptr;
    }

    /// Move from an existing @ref basic_observer_ptr instance with explicit casting
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
            manager.block->pop_ref();
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
            block->push_ref();
        }

        return *this;
    }

    /// Copy an existing @ref basic_observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    basic_observer_ptr& operator=(const basic_observer_ptr& value) noexcept {
        if (&value == this) {
            return *this;
        }

        set_data_(value.block, value.data);

        if (block) {
            block->push_ref();
        }

        return *this;
    }

    /// Copy an existing @ref basic_observer_ptr instance
    /** \param value The existing weak pointer to copy
    *   \note This operator only takes part in overload resolution if
    *         `U*` is convertible to `T*`.
    */
    template<typename U, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    basic_observer_ptr& operator=(const basic_observer_ptr<U,Policy>& value) noexcept {
        set_data_(value.block, value.data);

        if (block) {
            block->push_ref();
        }

        return *this;
    }

    /// Move from an existing @ref basic_observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source pointer is set to null.
    */
    basic_observer_ptr& operator=(basic_observer_ptr&& value) noexcept {
        set_data_(value.block, value.data);

        value.block = nullptr;
        value.data = nullptr;

        return *this;
    }

    /// Move from an existing @ref basic_observer_ptr instance
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
            block->pop_ref();
            block = nullptr;
            data = nullptr;
        }
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return `nullptr` if @ref expired() is `true`, or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning pointer
    *         will not be reset or destroyed until you are done using the raw pointer.
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
    *   \note Using this function if @ref expired() is `true` will lead to undefined behavior.
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning pointer
    *         will not be reset or destroyed until you are done using the raw pointer.
    */
    T& operator*() const noexcept {
        return *get();
    }

    /// Get a non-owning raw pointer to the pointed object, or `nullptr` if deleted.
    /** \return `nullptr` if @ref expired() is `true`, or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning pointer
    *         will not be reset or destroyed until you are done using the raw pointer.
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

namespace details {
    template<bool Virtual, typename T>
    struct inherit_as_virtual;

    template<typename T>
    struct inherit_as_virtual<true,T> : virtual T {
        template<typename ... Args>
        inherit_as_virtual(Args&& ... args) noexcept(noexcept(T(std::forward<Args>(args)...))) :
            T(std::forward<Args>(args)...) {}
    };

    template<typename T>
    struct inherit_as_virtual<false,T> : T {
        template<typename ... Args>
        inherit_as_virtual(Args&& ... args) noexcept(noexcept(T(std::forward<Args>(args)...))) :
            T(std::forward<Args>(args)...) {}
    };
}

/// Enables creating an @ref observer_ptr from `this`.
/** If an object is owned by a @ref basic_observable_ptr and must be able to create an observer
*   pointer to itself, without having direct access to the owner pointer,
*   then the object's class can inherit from @ref basic_enable_observer_from_this.
*   This provides the @ref observer_from_this() member function, which returns
*   a new observer pointer to the object. For this mechanism to work,
*   the class must inherit publicly from @ref basic_enable_observer_from_this.
*
*   If `Policy::is_sealed` is true, then the object must be owned by a @ref basic_observable_ptr
*   instance when calling @ref observer_from_this(). If the latter condition is not satisfied,
*   then @ref observer_from_this() will throw @ref bad_observer_from_this.
*
*   If `Policy::is_eoft_base_virtual` is true, then:
*    - if `Policy::is_sealed` is false, @ref basic_enable_observer_from_this will
*      allocate its own control block in the default constructor, and this control
*      block will be used by @ref basic_observable_ptr. This enables using
*      @ref observer_from_this() at all times, even during the object's constructor,
*      and even if the object is not owned by an instance of @ref basic_observable_ptr.
*      The downside is that the default constructor allocates, and thus cannot be noexcept.
*      The object `T` can have a default constructor.
*    - if `Policy::is_sealed` is true, the control block will already be created by
*      @ref make_observable, so @ref basic_enable_observer_from_this will not allocate its own
*      control block in the default constructor, which is thus noexcept. The object `T` can have a
*      default constructor. This means, however, that @ref observer_from_this() may only be called
*      if `T` was created by @ref make_observable, and only after @ref make_observable has returned,
*      otherwise @ref bad_observer_from_this will be thrown. In paticular, @ref observer_from_this()
*      may not be called from within `T`'s constructor. If this is an issue, consider setting
*      `Policy::is_eoft_base_virtual` to false.
*
*   If `Policy::is_eoft_base_virtual` is false, then `T`'s constructor must take
*   a non-const reference to a control block object as first argument, and forward it to
*   this class. Instances of `T` are then only constructible through @ref make_observable,
*   which will take care of creating one control block and forwarding it to `T`'s constructor.
*   This enables using @ref observer_from_this() at all times, even during the object's constructor,
*   and (if `Policy::is_sealed` is false) even if `T` is later released from its
*   @ref basic_observable_ptr. The downside is that the object `T` cannot have a default
*   constructor, and that child classes of `T` must forward the control block reference in their
*   own constructor. Likewise, for this reason, `T` cannot be copied nor moved.
*
*   **Corner cases.**
*    - Multiple inheritance. If a class `A` inherits from both another class `B` and
*      `basic_enable_observer_from_this<A,...>`, and if `B` also inherits from
*      `basic_enable_observer_from_this<B,...>`, then @ref observer_from_this() will be an
*      ambiguous call. But it can be resolved, and (contrary to `std::shared_ptr` and
*      `std::enable_shared_from_this`) will return a valid pointer regardless of the policy:
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
*
*     - Calling `observer_from_this()` from the object's constructor. Contrary to `std::shared_ptr`,
*       this is valid in all cases, except if both `Policy::is_sealed` and
*       `Policy::is_eoft_base_virtual` are true. Then, @ref bad_observer_from_this will
*       be thrown.
*
*   \see enable_observer_from_this_unique
*   \see enable_observer_from_this_sealed
*   \see basic_observable_ptr
*   \see basic_observer_ptr
*/
template<typename T, typename Policy>
class basic_enable_observer_from_this : public
    details::inherit_as_virtual<policy_queries<Policy>::eoft_base_is_virtual(),
        details::enable_observer_from_this_base<Policy>> {

    /// Policy query helper
    using queries = policy_queries<Policy>;

    using base = details::inherit_as_virtual<queries::eoft_base_is_virtual(),
        details::enable_observer_from_this_base<Policy>>;


protected:
    /// Policy for the control block
    using observer_policy = typename Policy::observer_policy;

    /// Type of the control block
    using control_block_type = basic_control_block<observer_policy>;

    /// Default constructor.
    /** \note This constructor is only enabled if `Policy::is_eoft_base_virtual` is true.
    *         If `Policy::is_sealed` is false, this will allocate the control block,
    *         which prevents this constructor from being `noexcept`.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && queries::eoft_allow_default_constructor()>>
    basic_enable_observer_from_this() noexcept(!queries::eoft_constructor_allocates()) {};

    /// Early assignment of control block.
    /** \param block The pre-allocated control block
    *   \note This constructor allows setting the control block pointer early
    *         in the creation of the object. This enables calling @ref observer_from_this
    *         from the object's constructor. This may only be used if `Policy::is_sealed` is
    *         true, and with @ref make_observable. @ref make_observable will allocate the control
    *         block and, if the object's constructor can take the control block as its first
    *         constructor argument, the control block will be forwarded to the constructor.
    */
    explicit basic_enable_observer_from_this(control_block_type& block) noexcept : base(block) {}

    /// Copy constructor.
    /** \note This constructor is only enabled if `Policy::is_eoft_base_virtual` is true.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && queries::eoft_base_is_virtual()>>
    basic_enable_observer_from_this(const basic_enable_observer_from_this&)
        noexcept(!queries::eoft_constructor_allocates()) {
        // Do not copy the other object's observer, this would be an
        // invalid reference.
    };

    /// Move constructor.
    /** \note This constructor is only enabled if `Policy::is_eoft_base_virtual` is true.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && queries::eoft_base_is_virtual()>>
    basic_enable_observer_from_this(basic_enable_observer_from_this&&)
        noexcept(!queries::eoft_constructor_allocates()) {
        // Do not move the other object's observer, this would be an
        // invalid reference.
    };

    /// Copy assignment operator.
    /** \note This operator is only enabled if `Policy::is_eoft_base_virtual` is true.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && queries::eoft_base_is_virtual()>>
    basic_enable_observer_from_this& operator=(const basic_enable_observer_from_this&) noexcept {
        // Do not copy the other object's observer, this would be an
        // invalid reference.
        return *this;
    };

    /// Move assignment operator.
    /** \note This operator is only enabled if `Policy::is_eoft_base_virtual` is true.
    */
    template<typename U = T, typename enable =
        std::enable_if_t<std::is_same_v<U,T> && queries::eoft_base_is_virtual()>>
    basic_enable_observer_from_this& operator=(basic_enable_observer_from_this&&) noexcept {
        // Do not move the other object's observer, this would be an
        // invalid reference.
        return *this;
    };

public:
    /// Type of observer pointers.
    using observer_type = basic_observer_ptr<T, observer_policy>;
    /// Type of observer pointers (const).
    using const_observer_type = basic_observer_ptr<const T, observer_policy>;

    /// Return an observer pointer to 'this'.
    /** \return A new observer pointer pointing to 'this'.
    *   \note If 'this' is not owned by a unique or sealed pointer, i.e., if
    *   the object was allocated on the stack, or if it is owned by another
    *   type of smart pointer, then this function will return nullptr.
    */
    observer_type observer_from_this() noexcept(queries::eoft_constructor_allocates()) {
        static_assert(std::is_base_of_v<basic_enable_observer_from_this,std::decay_t<T>>,
            "T must inherit from basic_enable_observer_from_this<T>");

        if constexpr (!queries::eoft_constructor_allocates()) {
            // This check is not needed if the constructor allocates; then we
            // always have a valid control block and this cannot fail.
            if (!this->this_control_block) {
                throw bad_observer_from_this{};
            }
        }

        return observer_type{this->this_control_block, static_cast<T*>(this)};
    }

    /// Return a const observer pointer to 'this'.
    /** \return A new observer pointer pointing to 'this'.
    *   \note If 'this' is not owned by a unique or sealed pointer, i.e., if
    *   the object was allocated on the stack, or if it is owned by another
    *   type of smart pointer, then this function will return nullptr.
    */
    const_observer_type observer_from_this() const noexcept(queries::eoft_constructor_allocates()) {

        static_assert(std::is_base_of_v<basic_enable_observer_from_this,std::decay_t<T>>,
            "T must inherit from basic_enable_observer_from_this<T>");

        if constexpr (!queries::eoft_constructor_allocates()) {
            // This check is not needed if the constructor allocates; then we
            // always have a valid control block and this cannot fail.
            if (!this->this_control_block) {
                throw bad_observer_from_this{};
            }
        }

        return const_observer_type{this->this_control_block, static_cast<const T*>(this)};
    }
};


/// Perform a `static_cast` for an @ref basic_observable_ptr.
/** \param ptr The pointer to cast
*   \note Ownership will be transfered to the returned pointer.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename D, typename P>
basic_observable_ptr<U,D,P> static_pointer_cast(basic_observable_ptr<T,D,P>&& ptr) {
    return basic_observable_ptr<U,D,P>(std::move(ptr), static_cast<U*>(ptr.get()));
}

/// Perform a `static_cast` for a @ref basic_observer_ptr.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is not modified.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> static_pointer_cast(const basic_observer_ptr<T,Policy>& ptr) {
    // NB: can use raw_get() as static cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(ptr, static_cast<U*>(ptr.raw_get()));
}

/// Perform a `static_cast` for a @ref basic_observer_ptr.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is set to null.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> static_pointer_cast(basic_observer_ptr<T,Policy>&& ptr) {
    // NB: can use raw_get() as static cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(std::move(ptr), static_cast<U*>(ptr.raw_get()));
}

/// Perform a `const_cast` for an @ref basic_observable_ptr.
/** \param ptr The pointer to cast
*   \note Ownership will be transfered to the returned pointer.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename D, typename P>
basic_observable_ptr<U,D,P> const_pointer_cast(basic_observable_ptr<T,D,P>&& ptr) {
    return basic_observable_ptr<U,D,P>(std::move(ptr), const_cast<U*>(ptr.get()));
}

/// Perform a `const_cast` for a @ref basic_observer_ptr.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is not modified.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> const_pointer_cast(const basic_observer_ptr<T,Policy>& ptr) {
    // NB: can use raw_get() as const cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(ptr, const_cast<U*>(ptr.raw_get()));
}

/// Perform a `const_cast` for a @ref basic_observer_ptr.
/** \param ptr The pointer to cast
*   \note A new observer is returned, the input observer is set to null.
          If the input pointer is null, the output pointer will also be null.
*/
template<typename U, typename T, typename Policy>
basic_observer_ptr<U,Policy> const_pointer_cast(basic_observer_ptr<T,Policy>&& ptr) {
    // NB: can use raw_get() as const cast of an expired pointer is fine
    return basic_observer_ptr<U,Policy>(std::move(ptr), const_cast<U*>(ptr.raw_get()));
}

/// Perform a `dynamic_cast` for a @ref basic_observable_ptr.
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

/// Perform a `dynamic_cast` for a @ref basic_observer_ptr.
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

/// Perform a `dynamic_cast` for a @ref basic_observer_ptr.
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

/// Implementation-defined class holding reference counts and expired flag.
/** All the details about this class are private, and only accessible to
*   `oup::` classes. As a user, you may only use this class to forward it
*   to `oup::` classes as required.
*/
using control_block = basic_control_block<default_observer_policy>;

/// Unique-ownership smart pointer, can be observed by @ref observer_ptr, ownership can be released.
/** This smart pointer mimics the interface of `std::unique_ptr`, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object, but allows the ownership to
*   be released, e.g., to transfer the ownership to another owner.
*
*   The main difference with `std::unique_ptr` is that it allows creating
*   @ref basic_observer_ptr instances to observe the lifetime of the pointed object,
*   as one would do with `std::shared_ptr` and `std::weak_ptr`. The price to pay,
*   compared to a standard `std::unique_ptr`, is the additional heap allocation
*   of the reference-counting control block. Because @ref observable_unique_ptr
*   can be released (see @ref basic_observable_ptr::release()), this cannot be
*   optimized. If releasing is not a needed feature, consider using
*   @ref observable_sealed_ptr instead.
*
*   If you need to create an @ref observer_ptr from a `this` pointer,
*   consider making the object inheriting from @ref enable_observer_from_this_unique.
*   This provides the same functionality as `std::enable_shared_from_this`, with
*   a few additions. Please consult the documentation for @ref enable_observer_from_this_unique
*   for more information.
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, @ref observer_ptr cannot extend
*      the lifetime of the pointed object, hence @ref observable_unique_ptr provides
*      less thread-safety compared to std::shared_ptr.
*    - @ref observable_unique_ptr does not support arrays.
*    - @ref observable_unique_ptr does not allow custom allocators.
*
*   \see basic_observable_ptr
*   \see observer_ptr
*   \see enable_observable_from_this_unique
*   \see make_observable_unique
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
*   @ref basic_observer_ptr instances to observe the lifetime of the pointed object,
*   as one would do with `std::shared_ptr` and `std::weak_ptr`. The price to pay,
*   compared to a standard `std::unique_ptr`, is the additional heap allocation
*   of the reference-counting control block, which @ref make_observable_sealed()
*   will optimize as a single heap allocation with the pointed object (as
*   `std::make_shared()` does for `std::shared_ptr`).
*
*   If you need to create an @ref observer_ptr from a `this` pointer,
*   consider making the object inheriting from @ref enable_observer_from_this_sealed.
*   Compared to @ref enable_observer_from_this_unique, this has some additional
*   limitations. Please consult the documenation for @ref enable_observer_from_this_sealed
*   for more information.
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, @ref observer_ptr cannot extend
*      the lifetime of the pointed object, hence @ref observable_sealed_ptr provides
*      less thread-safety compared to `std::shared_ptr`.
*    - @ref observable_sealed_ptr does not support arrays.
*    - @ref observable_sealed_ptr does not allow custom allocators.
*
*   \see basic_observable_ptr
*   \see observer_ptr
*   \see enable_observable_from_this_sealed
*   \see make_observable_sealed
*/
template<typename T>
using observable_sealed_ptr = basic_observable_ptr<T, placement_delete, sealed_policy>;

/// Non-owning smart pointer that observes a @ref observable_sealed_ptr or @ref observable_unique_ptr.
/** \see basic_observer_ptr
*/
template<typename T>
using observer_ptr = basic_observer_ptr<T, default_observer_policy>;

/// Enables creating an @ref observer_ptr from `this`.
/** If an object owned by a @ref observable_unique_ptr must be able to create an observer
*   pointer to itself, without having direct access to the owner pointer,
*   then the object's class can inherit from @ref enable_observer_from_this_unique.
*   This provides the @ref basic_enable_observer_from_this::observer_from_this() member function,
*   which returns a new observer pointer to the object. For this mechanism to work,
*   the class must inherit publicly from @ref enable_observer_from_this_unique.
*
*   \see basic_enable_observer_from_this
*   \see observable_unique_ptr
*   \see observer_ptr
*/
template<typename T>
using enable_observer_from_this_unique = basic_enable_observer_from_this<T, unique_policy>;

/// Enables creating an @ref observer_ptr from `this`.
/** If an object owned by a @ref observable_unique_ptr must be able to create an observer
*   pointer to itself, without having direct access to the owner pointer,
*   then the object's class can inherit from @ref enable_observer_from_this_sealed.
*   This provides the @ref basic_enable_observer_from_this::observer_from_this() member function,
*   which returns a new observer pointer to the object. For this mechanism to work,
*   the class must inherit publicly from @ref enable_observer_from_this_sealed.
*
*   Because of limitations from the "sealed" memory, for this mechanism to work, all constructors
*   of `T` must take a non-constant reference to a control block object, which will be provided
*   automatically by @ref make_observable_sealed. This means that `T` cannot have a default
*   constructor, nor can it have a copy or move constructor / assignment operator.
*
*   \see basic_enable_observer_from_this
*   \see observable_sealed_ptr
*   \see observer_ptr
*/
template<typename T>
using enable_observer_from_this_sealed = basic_enable_observer_from_this<T, sealed_policy>;

/// Create a new @ref observable_unique_ptr with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new observable_unique_ptr
*   \note Custom deleters are not supported by this function. If you require
*         a custom deleter, please use the @ref observable_unique_ptr constructors
*         directly. Compared to @ref make_observable_sealed(), this function
*         does not allocate the pointed object and the control block in a single
*         buffer, as that would prevent implementing @ref basic_observable_ptr::release().
*         If releasing the pointer is not needed, consider using @ref observable_sealed_ptr
*         instead.
*   \see observable_unique_ptr
*/
template<typename T, typename ... Args>
observable_unique_ptr<T> make_observable_unique(Args&& ... args) {
    return make_observable<T, unique_policy>(std::forward<Args>(args)...);
}

/// Create a new @ref observable_sealed_ptr with a newly constructed object.
/** \param args Arguments to construct the new object
*   \return The new observable_sealed_ptr
*   \note This function is the only way to create an @ref observable_sealed_ptr.
*         It will allocate the pointed object and the control block in a
*         single buffer for better performance.
*   \note If `T` inherits from `enable_observer_from_this_sealed`, then `T`'s
*         constructor must take a non-const reference to a control block as first argument,
*         and the control block will be created and provided by this function.
*         See @ref enable_observer_from_this_sealed.
*   \see observable_sealed_ptr
*/
template<typename T, typename ... Args>
observable_sealed_ptr<T> make_observable_sealed(Args&& ... args) {
    return make_observable<T, sealed_policy>(std::forward<Args>(args)...);
}


}

#endif
