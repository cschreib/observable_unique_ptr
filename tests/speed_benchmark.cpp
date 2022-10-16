#include "speed_benchmark_common.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>

template<typename B, typename F>
auto run_benchmark_for(F&& func) {
    B bench{};

    double                elapsed        = 0.0;
    double                elapsed_square = 0.0;
    double                attempts       = 0.0;
    constexpr std::size_t num_iter       = 1'000'000;
    constexpr double      min_time       = 0.2;

    while (elapsed * num_iter < min_time) {
        auto prev = timer::now();

        for (std::size_t i = 0; i < num_iter; ++i) {
            func(bench);
        }

        auto now = timer::now();

        double spent =
            std::chrono::duration_cast<std::chrono::duration<double>>(now - prev).count() /
            num_iter;
        elapsed += spent;
        elapsed_square += spent * spent;
        attempts += 1.0;
    }

    double stddev =
        std::sqrt(elapsed_square / attempts - (elapsed / attempts) * (elapsed / attempts)) /
        std::sqrt(attempts);

    return std::make_pair(elapsed / attempts, stddev);
}

template<typename B, typename F>
auto run_benchmark(F&& func) {
    using ref_type = benchmark<std::unique_ptr<typename B::element_type>>;

    auto result     = run_benchmark_for<B>(func);
    auto result_ref = run_benchmark_for<ref_type>(func);

    double ratio        = result.first / result_ref.first;
    double rel_err      = result.second / result.first;
    double rel_err_ref  = result_ref.second / result_ref.first;
    double ratio_stddev = std::sqrt(rel_err * rel_err + rel_err_ref * rel_err_ref) * ratio;

    return std::make_pair(result, std::make_pair(ratio, ratio_stddev));
}

std::unordered_map<std::string, std::unordered_map<std::string, std::vector<double>>> results;

template<typename T>
struct get_type_name;

template<typename T>
struct get_type_name<std::shared_ptr<T>> {
    static constexpr const char* value = "weak/shared";
};

template<typename T>
struct get_type_name<oup::observable_unique_ptr<T>> {
    static constexpr const char* value = "observer/obs_unique";
};

template<typename T>
struct get_type_name<oup::observable_sealed_ptr<T>> {
    static constexpr const char* value = "observer/obs_sealed";
};

template<typename T, typename R>
void do_report(const char* name, const R& which) {
    std::cout << " - " << name << ": " << which.first.first * 1e6 << " +/- "
              << which.first.second * 1e6 << "us "
              << "(x" << which.second.first << " +/- " << which.second.second << ")" << std::endl;

    results[name][get_type_name<T>::value].push_back(which.second.first);
}

double median(std::vector<double> v) {
    const auto n = static_cast<std::ptrdiff_t>(v.size() / 2);
    std::nth_element(v.begin(), v.begin() + n, v.end());
    return *(v.begin() + n);
}

std::string round1(double v) {
    std::ostringstream str;
    str << std::fixed << std::setprecision(1);
    str << std::round(v * 10.0) / 10.0;

    auto res = str.str();
    if (res.find_first_of('.') == std::string::npos) {
        res += ".0";
    }

    return res;
}

template<typename T>
void do_benchmarks_for_ptr(const char* type_name, const char* ptr_name) {
    using B = benchmark<T>;

    auto construct_destruct_owner_empty =
        run_benchmark<B>([](auto& b) { return b.construct_destruct_owner_empty(); });
    auto construct_destruct_owner =
        run_benchmark<B>([](auto& b) { return b.construct_destruct_owner(); });
    auto construct_destruct_owner_factory =
        run_benchmark<B>([](auto& b) { return b.construct_destruct_owner_factory(); });
    auto dereference_owner = run_benchmark<B>([](auto& b) { return b.dereference_owner(); });
    auto construct_destruct_weak_empty =
        run_benchmark<B>([](auto& b) { return b.construct_destruct_weak_empty(); });
    auto construct_destruct_weak =
        run_benchmark<B>([](auto& b) { return b.construct_destruct_weak(); });
    auto construct_destruct_weak_copy =
        run_benchmark<B>([](auto& b) { return b.construct_destruct_weak_copy(); });
    auto dereference_weak = run_benchmark<B>([](auto& b) { return b.dereference_weak(); });

    std::cout << ptr_name << "<" << type_name << ">:" << std::endl;

#define report(which) do_report<T>(#which, which)
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
    do_benchmarks<std::array<int, 65'536>>("big_array");

    std::vector<std::pair<std::string, std::string>> rows = {
        {"Create owner empty", "construct_destruct_owner_empty"},
        {"Create owner", "construct_destruct_owner"},
        {"Create owner factory", "construct_destruct_owner_factory"},
        {"Dereference owner", "dereference_owner"},
        {"Create observer empty", "construct_destruct_weak_empty"},
        {"Create observer", "construct_destruct_weak"},
        {"Create observer copy", "construct_destruct_weak_copy"},
        {"Dereference observer", "dereference_weak"},
    };

    std::vector<std::string> cols = {"weak/shared", "observer/obs_unique", "observer/obs_sealed"};

    std::cout << "| Pointer | raw/unique | ";
    for (const auto& t : cols) {
        std::cout << t << " | ";
    }
    std::cout << std::endl;

    std::cout << "|---|---|";
    for (const auto& t [[maybe_unused]] : cols) {
        std::cout << "---|";
    }
    std::cout << std::endl;

    for (const auto& r : rows) {
        std::cout << "| " << r.first << " | 1 | ";
        for (const auto& t : cols) {
            if (r.second == "construct_destruct_owner" && t == "observer/obs_sealed") {
                std::cout << "N/A | ";
            } else {
                std::cout << round1(median(results[r.second][t])) << " | ";
            }
        }
        std::cout << std::endl;
    }

    return 0;
}
