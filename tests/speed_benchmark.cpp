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

    void construct_destruct_owner_empty() {
        auto p = owner_type{};
        use_object(p);
    }

    void construct_destruct_owner() {
        auto p = traits::make_ptr();
        use_object(p);
    }

    void construct_destruct_owner_factory() {
        auto p = traits::make_ptr_factory();
        use_object(p);
    }

    void construct_destruct_weak_empty() {
        auto p = weak_type{};
        use_object(p);
    }

    void construct_destruct_weak() {
        auto wp = traits::make_weak(owner);
        use_object(wp);
    }

    void dereference_owner() {
        use_object(*owner);
    }

    void dereference_weak() {
        traits::deref_weak(weak, [](auto& o) { use_object(o); });
    }
    void construct_destruct_weak_copy() {
        auto wp = weak;
        use_object(wp);
    }
};

using timer = std::chrono::high_resolution_clock;

template<typename B, typename F>
double run_benchmark_for(F&& func) {
    B bench{};

    auto prev = timer::now();
    double elapsed = 0.0;
    double count = 0.0;
    constexpr std::size_t num_iter = 10'000;

    while (elapsed < 1.0) {
        for (std::size_t i = 0; i < num_iter; ++i) {
            func(bench);
        }

        auto now = timer::now();
        elapsed += std::chrono::duration_cast<std::chrono::duration<double>>(now - prev).count();
        count += static_cast<double>(num_iter);
        std::swap(now, prev);
    }

    return elapsed/count;
}

template<typename B, typename F>
auto run_benchmark(F&& func) {
    using ref_type = benchmark<std::unique_ptr<typename B::element_type>>;
    double result = run_benchmark_for<B>(func);
    double result_ref = run_benchmark_for<ref_type>(func);
    return std::make_pair(result, result/result_ref);
}

template<typename T>
void do_benchmarks_for_ptr(const char* type_name, const char* ptr_name) {
    using B = benchmark<T>;

    auto construct_destruct_owner_empty = run_benchmark<B>([](auto& b) { return b.construct_destruct_owner_empty(); });
    auto construct_destruct_owner = run_benchmark<B>([](auto& b) { return b.construct_destruct_owner(); });
    auto construct_destruct_owner_factory = run_benchmark<B>([](auto& b) { return b.construct_destruct_owner_factory(); });
    auto dereference_owner = run_benchmark<B>([](auto& b) { return b.dereference_owner(); });
    auto construct_destruct_weak_empty = run_benchmark<B>([](auto& b) { return b.construct_destruct_weak_empty(); });
    auto construct_destruct_weak = run_benchmark<B>([](auto& b) { return b.construct_destruct_weak(); });
    auto construct_destruct_weak_copy = run_benchmark<B>([](auto& b) { return b.construct_destruct_weak_copy(); });
    auto dereference_weak = run_benchmark<B>([](auto& b) { return b.dereference_weak(); });

    std::cout << ptr_name << "<" << type_name << ">:" << std::endl;
    #define report(which) std::cout << " - " << #which << ": " << which.first*1e6 << "us (x" << which.second << ")" << std::endl

    report(construct_destruct_owner_empty);
    report(construct_destruct_owner);
    report(construct_destruct_owner_factory);
    report(dereference_owner);
    report(construct_destruct_weak_empty);
    report(construct_destruct_weak);
    report(construct_destruct_weak_copy);
    report(dereference_weak);

    #undef report
    std::cout << std::endl;
}

template<typename T>
void do_benchmarks(const char* type_name) {
    do_benchmarks_for_ptr<std::shared_ptr<T>>(type_name, "shared_ptr");
    do_benchmarks_for_ptr<oup::observable_unique_ptr<T>>(type_name, "observable_unique_ptr");
    do_benchmarks_for_ptr<oup::observable_sealed_ptr<T>>(type_name, "observable_sealed_ptr");
}

int main() {
    do_benchmarks<int>("int");
    do_benchmarks<std::string>("string");
    do_benchmarks<std::array<int,65'536>>("big_array");
    return 0;
}
