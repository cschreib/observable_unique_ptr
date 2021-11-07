#include "speed_benchmark_common.hpp"
#include <cmath>

template<typename B, typename F>
auto run_benchmark_for(F&& func) {
    B bench{};

    double elapsed = 0.0;
    double elapsed_square = 0.0;
    double count = 0.0;
    double attempts = 0.0;
    constexpr std::size_t num_iter = 1'000'000;

    while (elapsed*num_iter < 1.0) {
        auto prev = timer::now();

        for (std::size_t i = 0; i < num_iter; ++i) {
            func(bench);
        }

        auto now = timer::now();

        double spent = std::chrono::duration_cast<std::chrono::duration<double>>(now - prev).count()/num_iter;
        elapsed += spent;
        elapsed_square += spent*spent;
        attempts += 1.0;
    }

    double stddev = std::sqrt(elapsed_square/attempts - (elapsed/attempts)*(elapsed/attempts))/std::sqrt(attempts);

    return std::make_pair(elapsed/attempts, stddev);
}

template<typename B, typename F>
auto run_benchmark(F&& func) {
    using ref_type = benchmark<std::unique_ptr<typename B::element_type>>;

    auto result = run_benchmark_for<B>(func);
    auto result_ref = run_benchmark_for<ref_type>(func);

    double ratio = result.first/result_ref.first;
    double rel_err = result.second/result.first;
    double rel_err_ref = result_ref.second/result_ref.first;
    double ratio_stddev = std::sqrt(rel_err*rel_err + rel_err_ref*rel_err_ref)*ratio;

    return std::make_pair(result, std::make_pair(ratio, ratio_stddev));
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
    #define report(which) std::cout << " - " << #which << ": " << \
        which.first.first*1e6 << " +/- " << which.first.second*1e6 << "us " << \
        "(x" << which.second.first << " +/- " << which.second.second << ")" << std::endl

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
    do_benchmarks<float>("float");
    do_benchmarks<std::string>("string");
    do_benchmarks<std::array<int,65'536>>("big_array");
    return 0;
}
