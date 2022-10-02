#include "snatch.hpp"

#include "cxxopts.hpp"

#include <cstring>
#include <set>

namespace {
using namespace testing;
using namespace testing::impl;

template<typename F>
bool run_tests(registry& r, F&& predicate) {
    bool        success         = true;
    std::size_t run_count       = 0;
    std::size_t fail_count      = 0;
    std::size_t assertion_count = 0;

    for (std::size_t i = 0; i < r.test_count; ++i) {
        test_case& t = r.test_list[i];
        if (!predicate(t)) {
            continue;
        }

        r.run(t);

        ++run_count;
        assertion_count += t.tests;
        if (t.state == test_state::failed) {
            ++fail_count;
            success = false;
        }
    }

    std::printf("==========================================\n");
    if (success) {
        std::printf(
            "%ssuccess:%s all tests passed (%ld test cases, %ld assertions)\n", color::pass_start,
            color::reset, run_count, assertion_count);
    } else {
        std::printf(
            "%serror:%s some tests failed (%ld out of %ld test cases, %ld assertions)\n",
            color::fail_start, color::reset, fail_count, run_count, assertion_count);
    }

    return success;
}

template<typename F>
void list_tests(const registry& r, F&& predicate) {
    for (std::size_t i = 0; i < r.test_count; ++i) {
        const auto& t = r.test_list[i];
        if (!predicate(t)) {
            continue;
        }

        if (!t.type.empty()) {
            std::printf(
                "%.*s [%.*s]\n", static_cast<int>(t.name.length()), t.name.data(),
                static_cast<int>(t.type.length()), t.type.data());
        } else {
            std::printf("%.*s\n", static_cast<int>(t.name.length()), t.name.data());
        }
    }
}

template<typename F>
void for_each_tag(std::string_view s, F&& callback) {
    std::string_view delim    = ",";
    std::size_t      pos      = s.find(delim);
    std::size_t      last_pos = 0u;

    while (pos != std::string_view::npos) {
        std::size_t cur_size = pos - last_pos;
        if (cur_size != 0) {
            callback(s.substr(last_pos, cur_size));
        }
        last_pos = pos + delim.size();
        pos      = s.find(delim, last_pos);
    }

    callback(s.substr(last_pos));
}

std::size_t get_full_name_length(const test_case& t) {
    // +3 for " []" surrounding the type
    return t.name.length() + t.type.length() + 3;
}

std::string_view
make_full_name(std::array<char, max_test_name_length>& buffer, const test_case& t) {
    std::memcpy(buffer.data(), t.name.data(), t.name.length());
    std::memcpy(buffer.data() + t.name.length(), " [", 2);
    std::memcpy(buffer.data() + t.name.length() + 2, t.type.data(), t.type.length());
    std::memcpy(buffer.data() + t.name.length() + t.type.length() + 2, "]", 1);
    return std::string_view(buffer.data(), get_full_name_length(t));
}

template<typename T>
constexpr const char* get_format_code() {
    if constexpr (std::is_same_v<T, const void*>) {
        return "%p";
    } else if constexpr (std::is_same_v<T, std::size_t>) {
        return "%zu";
    } else if constexpr (std::is_same_v<T, std::ptrdiff_t>) {
        return "%td";
    } else {
        static_assert(std::is_same_v<T, T>, "unsupported type");
    }
}

template<typename T>
void append_impl(expression& exp, T value) {
    std::size_t length =
        std::snprintf(exp.data.data() + exp.data_length, 0, get_format_code<T>(), value);

    if (length > exp.available()) {
        exp.failed = true;
        return;
    }

    std::snprintf(exp.data.data() + exp.data_length, exp.available(), get_format_code<T>(), value);

    exp.data_length += length;
}
} // namespace

namespace testing::impl::color {
const char* error_start      = "\x1b[1;31m";
const char* warning_start    = "\x1b[1;33m";
const char* status_start     = "\x1b[1;36m";
const char* fail_start       = "\x1b[1;31m";
const char* skipped_start    = "\x1b[1;33m";
const char* pass_start       = "\x1b[1;32m";
const char* highlight1_start = "\x1b[1;35m";
const char* highlight2_start = "\x1b[1;36m";
const char* reset            = "\x1b[0m";
} // namespace testing::impl::color

namespace testing::impl {
std::string_view expression::str() const {
    return std::string_view(data.data(), data_length);
}

std::size_t expression::available() const {
    return data.size() - data_length;
}

void expression::append_str(const char* str, std::size_t length) {
    if (failed) {
        return;
    }

    if (length > available()) {
        failed = true;
        return;
    }

    std::memcpy(data.data() + data_length, str, length);

    data_length += length;
}

void expression::append_str(const char* str) {
    append_str(str, std::strlen(str));
}

void expression::append_impl(const void* ptr) {
    ::append_impl(*this, ptr);
}

void expression::append_impl(std::nullptr_t) {
    append_str("nullptr");
}

void expression::append_impl(std::size_t i) {
    ::append_impl(*this, i);
}

void expression::append_impl(std::ptrdiff_t i) {
    ::append_impl(*this, i);
}

void expression::append_impl(bool value) {
    append_str(value ? "true" : "false");
}

void expression::append_impl(const std::string& value) {
    append_str(value.c_str(), value.length());
}

void expression::append_impl(std::string_view value) {
    append_str(value.data(), value.length());
}

void expression::append_impl(const char* value) {
    append_str(value);
}
} // namespace testing::impl

namespace testing {
void registry::register_test(
    std::string_view name, std::string_view tags, std::string_view type, test_ptr func) {

    if (test_count == max_test_cases) {
        std::printf(
            "%serror:%s max number of test cases reached; "
            "please increase 'max_test_cases' (currently %ld)\n.",
            color::error_start, color::reset, max_test_cases);
        std::terminate();
    }

    test_list[test_count] = test_case{name, tags, type, func};

    if (get_full_name_length(test_list[test_count]) > max_test_name_length) {
        std::printf(
            "%serror:%s max length of test name reached; "
            "please increase 'max_test_name_length' (currently %ld)\n.",
            color::error_start, color::reset, max_test_name_length);
        std::terminate();
    }

    ++test_count;
}

void registry::print_location(
    const test_case& current_case, const char* filename, int line_number) const {

    std::printf(
        "running test case \"%s%.*s%s\"\n"
        "          at %s:%d\n"
        "          for type %s%.*s%s\n",
        color::highlight1_start, static_cast<int>(current_case.name.length()),
        current_case.name.data(), color::reset, filename, line_number, color::highlight1_start,
        static_cast<int>(current_case.type.length()), current_case.type.data(), color::reset);
}

void registry::print_failure() const {
    std::printf("%sfailed:%s ", color::fail_start, color::reset);
}
void registry::print_skip() const {
    std::printf("%sskipped:%s ", color::skipped_start, color::reset);
}

void registry::print_details(const char* message) const {
    std::printf("          %s%s%s\n", color::highlight2_start, message, color::reset);
}

void registry::print_details_expr(
    const char* check, const char* exp_str, const expression& exp) const {
    std::printf("          %s%s(%s)%s", color::highlight2_start, check, exp_str, color::reset);
    if (!exp.failed) {
        auto str = exp.str();
        std::printf(
            ", got %s%.*s%s\n", color::highlight2_start, static_cast<int>(str.length()), str.data(),
            color::reset);
    } else {
        std::printf("\n");
    }
}

void registry::run(test_case& t) {
    if (verbose) {
        std::printf("%sstarting:%s %s", color::status_start, color::reset, color::highlight1_start);
        if (!t.type.empty()) {
            std::printf(
                "%.*s [%.*s]", static_cast<int>(t.name.length()), t.name.data(),
                static_cast<int>(t.type.length()), t.type.data());
        } else {
            std::printf("%.*s", static_cast<int>(t.name.length()), t.name.data());
        }
        std::printf("%s.\n", color::reset);
    }

    t.tests = 0;
    t.state = test_state::success;

    try {
        t.func(t);
    } catch (const test_state& s) {
        t.state = s;
    } catch (...) {
        t.state = test_state::failed;
    }

    if (verbose) {
        std::printf("%sfinished:%s %s", color::status_start, color::reset, color::highlight1_start);
        if (!t.type.empty()) {
            std::printf(
                "%.*s [%.*s]", static_cast<int>(t.name.length()), t.name.data(),
                static_cast<int>(t.type.length()), t.type.data());
        } else {
            std::printf("%.*s", static_cast<int>(t.name.length()), t.name.data());
        }
        std::printf("%s.\n", color::reset);
    }
}

void registry::set_state(test_case& t, test_state s) {
    if (static_cast<std::underlying_type_t<test_state>>(t.state) <
        static_cast<std::underlying_type_t<test_state>>(s)) {
        t.state = s;
    }
}

bool registry::run_all_tests() {
    return run_tests(*this, [](const test_case&) { return true; });
}

bool registry::run_tests_matching_name(std::string_view name) {
    std::array<char, max_test_name_length> buffer;
    return run_tests(*this, [&](const test_case& t) {
        std::string_view v = make_full_name(buffer, t);

        // TODO: use regex here?
        return v.find(name) != t.name.npos;
    });
}

bool registry::run_tests_with_tag(std::string_view tag) {
    return run_tests(*this, [&](const test_case& t) {
        bool selected = false;
        for_each_tag(t.tags, [&](std::string_view v) {
            if (v == tag) {
                selected = true;
            }
        });
        return selected;
    });
}

void registry::list_all_tags() const {
    std::set<std::string_view> tags;
    for (std::size_t i = 0; i < test_count; ++i) {
        const auto& t = test_list[i];

        for_each_tag(t.tags, [&](std::string_view v) { tags.insert(v); });
    }

    for (auto c : tags) {
        std::printf("%.*s\n", static_cast<int>(c.length()), c.data());
    }
}

void registry::list_all_tests() const {
    list_tests(*this, [](const test_case&) { return true; });
}

void registry::list_tests_with_tag(std::string_view tag) const {
    list_tests(*this, [&](const test_case& t) {
        bool selected = false;
        for_each_tag(t.tags, [&](std::string_view v) {
            if (v == tag) {
                selected = true;
            }
        });
        return selected;
    });
}

registry tests;
} // namespace testing

int main(int argc, char* argv[]) {
    cxxopts::Options options(argv[0], "Snatch test runner");

    // clang-format off
    options.add_options()
        ("tests", "A regex to select which test cases to run", cxxopts::value<std::string>())
        ("l,list-tests", "List tests by name")
        ("list-tags", "List tags by name")
        ("list-tests-with-tag", "List tests by name with a given tag", cxxopts::value<std::string>())
        ("t,tags", "Use tags for filtering, not name")
        ("v,verbose", "Enable detailed output")
        ("h,help", "Print help");
    // clang-format on

    options.parse_positional({"tests"});

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("list-tests") > 0) {
        testing::tests.list_all_tests();
        return 0;
    }

    if (result.count("list-tests-with-tag") > 0) {
        testing::tests.list_tests_with_tag(result["list-tests-with-tag"].as<std::string>());
        return 0;
    }

    if (result.count("list-tags") > 0) {
        testing::tests.list_all_tags();
        return 0;
    }

    if (result.count("verbose") > 0) {
        testing::tests.verbose = true;
    }

    bool success = true;
    if (result.count("tests") > 0) {
        if (result.count("tags") > 0) {
            success = testing::tests.run_tests_with_tag(result["tests"].as<std::string>());
        } else {
            success = testing::tests.run_tests_matching_name(result["tests"].as<std::string>());
        }
    } else {
        success = testing::tests.run_all_tests();
    }

    return success ? 0 : 1;
}
