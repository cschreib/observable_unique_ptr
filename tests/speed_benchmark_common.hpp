#include <memory>
#include <iostream>
#include <chrono>
#include <array>
#include <string>
#include <oup/observable_unique_ptr.hpp>

// External functions, the compiler cannot see through. Prevents optimisations.
template<typename T>
void use_object(T&) noexcept;

template<typename T>
struct pointer_traits;

template<typename T>
struct pointer_traits<std::unique_ptr<T>> {
    using element_type = T;
    using ptr_type = std::unique_ptr<T>;
    using weak_type = T*;

    static ptr_type make_ptr() noexcept { return ptr_type(new element_type); }
    static ptr_type make_ptr_factory() noexcept { return std::make_unique<element_type>(); }
    static weak_type make_weak(ptr_type& p) noexcept { return p.get(); }
    template<typename F>
    static void deref_weak(weak_type& p, F&& func) noexcept { return func(*p); }
};

template<typename T>
struct pointer_traits<std::shared_ptr<T>> {
    using element_type = T;
    using ptr_type = std::shared_ptr<T>;
    using weak_type = std::weak_ptr<T>;

    static ptr_type make_ptr() noexcept { return ptr_type(new element_type); }
    static ptr_type make_ptr_factory() noexcept { return std::make_shared<element_type>(); }
    static weak_type make_weak(ptr_type& p) noexcept { return weak_type(p); }
    template<typename F>
    static void deref_weak(weak_type& p, F&& func) noexcept { if (auto s = p.lock()) func(*s); }
};

template<typename T>
struct pointer_traits<oup::observable_unique_ptr<T>> {
    using element_type = T;
    using ptr_type = oup::observable_unique_ptr<T>;
    using weak_type = oup::observer_ptr<T>;

    static ptr_type make_ptr() noexcept { return ptr_type(new element_type); }
    static ptr_type make_ptr_factory() noexcept { return oup::make_observable_unique<element_type>(); }
    static weak_type make_weak(ptr_type& p) noexcept { return weak_type(p); }
    template<typename F>
    static void deref_weak(weak_type& p, F&& func) noexcept { return func(*p); }
};

template<typename T>
struct pointer_traits<oup::observable_sealed_ptr<T>> {
    using element_type = T;
    using ptr_type = oup::observable_sealed_ptr<T>;
    using weak_type = oup::observer_ptr<T>;

    static ptr_type make_ptr() noexcept { return oup::make_observable_sealed<element_type>(); }
    static ptr_type make_ptr_factory() noexcept { return oup::make_observable_sealed<element_type>(); }
    static weak_type make_weak(ptr_type& p) noexcept { return weak_type(p); }
    template<typename F>
    static void deref_weak(weak_type& p, F&& func) noexcept { return func(*p); }
};

template<typename T>
struct benchmark {
    using traits = pointer_traits<T>;
    using element_type = typename traits::element_type;
    using owner_type = typename traits::ptr_type;
    using weak_type = typename traits::weak_type;

    owner_type owner;
    weak_type weak;

    benchmark() : owner(traits::make_ptr()), weak(traits::make_weak(owner)) {}

    void construct_destruct_owner_empty();

    void construct_destruct_owner();

    void construct_destruct_owner_factory();

    void construct_destruct_weak_empty();

    void construct_destruct_weak();

    void construct_destruct_weak_copy();

    void dereference_owner();

    void dereference_weak();
};

using timer = std::chrono::high_resolution_clock;
