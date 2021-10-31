#ifndef OBSERVABLE_UNIQUE_PTR_INCLUDED
#define OBSERVABLE_UNIQUE_PTR_INCLUDED

#include <memory>

namespace oup {

/// Simple deleter, suitable for objects allocated with new.
struct default_deleter {
    template<typename T>
    void operator() (T* ptr) { delete ptr; }

    void operator() (std::nullptr_t) {}
};

/// std::unique_ptr that can be observed by std::weak_ptr
/** This smart pointer mimics the interface of std::unique_ptr, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object.
*
*   The main difference with std::unique_ptr is that it allows creating
*   std::weak_ptr instances to observe the lifetime of the pointed object,
*   as one would do with std::shared_ptr. The price to pay, compared to a
*   standard std::unique_ptr, is the additional heap allocation of the
*   reference-counting control block, which utils::make_observable_unique()
*   will optimise as a single heap allocation with the pointed object (as
*   std::make_shared() does for std::shared_ptr).
*
*   Other notable differences (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - observable_unique_ptr does not support arrays.
*    - observable_unique_ptr does not allow custom allocators.
*    - observable_unique_ptr does not have a release() function to let go of
*      the ownership.
*    - observable_unique_ptr allows moving from other observable_unique_ptr
*      only if the deleter type is exactly the same, while std::unique_ptr
*      allows moving from a convertible deleter.
*    - observable_unique_ptr may not own a deleter instance; if in doubt, check
*      has_deleter() before calling get_deleter(), or use try_get_deleter().
*    - a moved-from observable_unique_ptr will not own a deleter instance.
*/
template<typename T, typename Deleter = default_deleter>
class observable_unique_ptr : private std::shared_ptr<T> {
private:
    /// Construct from shared_ptr.
    /** \param value The shared_ptr to take ownership from
    *   \note This is private since the use of std::shared_ptr is
    *         an implementation detail.
    */
    explicit observable_unique_ptr(std::shared_ptr<T>&& value) noexcept :
        std::shared_ptr<T>(std::move(value)) {}

    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr;

public:
    // Import members from std::shared_ptr
    using typename std::shared_ptr<T>::element_type;
    using typename std::shared_ptr<T>::weak_type;

    using std::shared_ptr<T>::get;
    using std::shared_ptr<T>::operator*;
    using std::shared_ptr<T>::operator->;
    using std::shared_ptr<T>::operator bool;

    // Define member types for compatibility with std::unique_ptr
    using pointer = element_type*;
    using deleter_type = Deleter;

    /// Default constructor (null pointer).
    observable_unique_ptr() noexcept :
        std::shared_ptr<T>(nullptr, Deleter{}) {}

    /// Construct a null pointer.
    observable_unique_ptr(std::nullptr_t) noexcept :
        std::shared_ptr<T>(nullptr, Deleter{}) {}

    /// Construct a null pointer with custom deleter.
    observable_unique_ptr(std::nullptr_t, Deleter deleter) noexcept :
        std::shared_ptr<T>(nullptr, std::move(deleter)) {}

    /// Explicit ownership capture of a raw pointer.
    /** \param value The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value) : std::shared_ptr<T>(value, Deleter{}) {}

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param deleter The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value, Deleter deleter) :
        std::shared_ptr<T>(value, std::move(deleter)) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr(observable_unique_ptr<U,Deleter>&& value) noexcept :
        std::shared_ptr<T>(std::move(static_cast<std::shared_ptr<U>&>(value))) {}

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr(observable_unique_ptr<U,Deleter>&& manager, T* value) noexcept :
        std::shared_ptr<T>(std::move(manager), value) {
        manager.std::template shared_ptr<U>::reset();
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr& operator=(observable_unique_ptr<U,Deleter>&& value) noexcept {
        std::shared_ptr<T>::operator=(std::move(static_cast<std::shared_ptr<U>&>(value)));
        return *this;
    }

    // Movable
    observable_unique_ptr(observable_unique_ptr&&) noexcept = default;
    observable_unique_ptr& operator=(observable_unique_ptr&&) noexcept = default;

    // Non-copyable
    observable_unique_ptr(const observable_unique_ptr&) = delete;
    observable_unique_ptr& operator=(const observable_unique_ptr&) = delete;

    /// Checks if this pointer has a custom deleter.
    /** \return 'true' if a custom deleter is used, 'false' otherwise.
    */
    bool has_deleter() const noexcept {
        if constexpr (std::is_same_v<Deleter, oup::default_deleter>) {
            return false;
        } else {
            return std::get_deleter<Deleter>(*this) != nullptr;
        }
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    *   \note Using the return value of this function if has_deleter() returns 'false' will cause
    *         undefined behavior.
    */
    Deleter& get_deleter() noexcept {
        return *std::get_deleter<Deleter>(*this);
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    *   \note Using the return value of this function if has_deleter() returns 'false' will cause
    *         undefined behavior.
    */
    const Deleter& get_deleter() const noexcept {
        return *std::get_deleter<Deleter>(*this);
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter, or nullptr if no deleter exists
    */
    Deleter* try_get_deleter() noexcept {
        return std::get_deleter<Deleter>(*this);
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter, or nullptr if no deleter exists
    */
    const Deleter* try_get_deleter() const noexcept {
        return std::get_deleter<Deleter>(*this);
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observable_unique_ptr& other) noexcept {
        std::shared_ptr<T>::swap(other);
    }

    /// Replaces the managed object with a null pointer.
    /** \param ptr A nullptr_t instance
    */
    void reset(std::nullptr_t ptr = nullptr) noexcept {
        if constexpr (std::is_same_v<Deleter, oup::default_deleter>) {
            operator=(observable_unique_ptr{ptr});
        } else {
            if (auto* deleter = try_get_deleter()) {
                operator=(observable_unique_ptr{ptr, Deleter{*deleter}});
            } else {
                operator=(observable_unique_ptr{ptr, Deleter{}});
            }
        }
    }

    /// Replaces the managed object.
    /** \param p A nullptr_t instance
    */
    void reset(T* ptr) noexcept {
        if constexpr (std::is_same_v<Deleter, oup::default_deleter>) {
            operator=(observable_unique_ptr{ptr});
        } else {
            if (auto* deleter = try_get_deleter()) {
                operator=(observable_unique_ptr{ptr, Deleter{*deleter}});
            } else {
                operator=(observable_unique_ptr{ptr, Deleter{}});
            }
        }
    }

    /// Replaces the managed object (with custom deleter).
    /** \param ptr A pointer to the new object to own
    *   \param deleter The new custom deleter instance to use
    *   \note After this call, any previously owner object will be deleted
    */
    void reset(T* ptr, Deleter deleter) noexcept {
        operator=(observable_unique_ptr{ptr, std::move(deleter)});
    }

    template<typename U, typename ... Args>
    friend observable_unique_ptr<U> make_observable_unique(Args&& ... args);
};

/// Create a new observable_unique_ptr with a newly constructed object.
/** \param args Arguments to construt the new object
*   \return The new observable_unique_ptr
*/
template<typename T, typename ... Args>
observable_unique_ptr<T> make_observable_unique(Args&& ... args) {
    return observable_unique_ptr<T>(std::make_shared<T>(std::forward<Args>(args)...));
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
    const observable_unique_ptr<U,Deleter> second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U, typename Deleter>
bool operator!= (const observable_unique_ptr<T,Deleter>& first,
    const observable_unique_ptr<U,Deleter>& second) noexcept {
    return first.get() != second.get();
}

}

#endif
