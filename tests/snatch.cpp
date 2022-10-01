#include "snatch.hpp"

#include "cxxopts.hpp"

#include <cstring>
#include <set>

namespace testing {
namespace color {
const char* error_start      = "\e[1;31m";
const char* warning_start    = "\e[1;33m";
const char* status_start     = "\e[1;36m";
const char* fail_start       = "\e[1;31m";
const char* skipped_start    = "\e[1;33m";
const char* pass_start       = "\e[1;32m";
const char* highlight1_start = "\e[1;35m";
const char* highlight2_start = "\e[1;36m";
const char* reset            = "\e[0m";
} // namespace color

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

    strncpy(data.data() + data_length, str, length);

    data_length += length;
}

void expression::append_str(const char* str) {
    append_str(str, strlen(str));
}

void expression::append_impl(const void* ptr) {
    std::size_t length = snprintf(data.data() + data_length, 0, "%p", ptr);
    if (length > available()) {
        failed = true;
        return;
    }

    snprintf(data.data() + data_length, available(), "%p", ptr);

    data_length += length;
}

void expression::append_impl(std::nullptr_t) {
    append_str("nullptr");
}

void expression::append_impl(std::size_t i) {
    std::size_t length = snprintf(data.data() + data_length, 0, "%zu", i);
    if (length > available()) {
        failed = true;
        return;
    }

    snprintf(data.data() + data_length, available(), "%zu", i);

    data_length += length;
}

void expression::append_impl(std::ptrdiff_t i) {
    std::size_t length = snprintf(data.data() + data_length, 0, "%td", i);
    if (length > available()) {
        failed = true;
        return;
    }

    snprintf(data.data() + data_length, available(), "%td", i);

    data_length += length;
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

void registry::register_test(
    std::string_view name, std::string_view tags, std::string_view type, test_ptr func) {

    if (test_count == max_tests) {
        printf(
            "%serror:%s max number of tests reached; "
            "please increase 'max_tests' (currently %ld)\n.",
            color::error_start, color::reset, max_tests);
        std::terminate();
    }

    test_list[test_count] = test_case{name, tags, type, func};
    ++test_count;
}

void registry::print_location(
    const test_case& current_case, const char* filename, int line_number) const {

    printf(
        "running test case \"%s%.*s%s\"\n"
        "          at %s:%d\n"
        "          for type %s%.*s%s\n",
        color::highlight1_start, static_cast<int>(current_case.name.length()),
        current_case.name.data(), color::reset, filename, line_number, color::highlight1_start,
        static_cast<int>(current_case.type.length()), current_case.type.data(), color::reset);
}

void registry::print_failure() const {
    printf("%sfailed:%s ", color::fail_start, color::reset);
}
void registry::print_skip() const {
    printf("%sskipped:%s ", color::skipped_start, color::reset);
}

void registry::print_details(const char* message) const {
    printf("          %s%s%s\n", color::highlight2_start, message, color::reset);
}

void registry::print_details_expr(
    const char* check, const char* exp_str, const expression& exp) const {
    printf("          %s%s(%s)%s", color::highlight2_start, check, exp_str, color::reset);
    if (!exp.failed) {
        auto str = exp.str();
        printf(
            ", got %s%.*s%s\n", color::highlight2_start, static_cast<int>(str.length()), str.data(),
            color::reset);
    } else {
        printf("\n");
    }
}

void registry::run(test_case& t) {
    if (verbose) {
        printf(
            "%sstarting:%s %s%.*s%s.\n", color::status_start, color::reset, color::highlight1_start,
            static_cast<int>(t.name.length()), t.name.data(), color::reset);
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
        printf(
            "%sfinished:%s %s%.*s%s.\n", color::status_start, color::reset, color::highlight1_start,
            static_cast<int>(t.name.length()), t.name.data(), color::reset);
    }
}

void registry::set_state(test_case& t, test_state s) {
    if (static_cast<std::underlying_type_t<test_state>>(t.state) <
        static_cast<std::underlying_type_t<test_state>>(s)) {
        t.state = s;
    }
}

bool registry::run_all() {
    bool        success         = true;
    std::size_t run_count       = 0;
    std::size_t fail_count      = 0;
    std::size_t assertion_count = 0;
    for (std::size_t i = 0; i < test_count; ++i) {
        run(test_list[i]);
        ++run_count;
        assertion_count += test_list[i].tests;
        if (test_list[i].state == test_state::failed) {
            ++fail_count;
            success = false;
        }
    }

    printf("==========================================\n");
    if (success) {
        printf(
            "%ssuccess:%s all tests passed (%ld cases, %ld assertions)\n", color::pass_start,
            color::reset, run_count, assertion_count);
    } else {
        printf(
            "%serror:%s some tests failed (%ld out of %ld cases, %ld assertions)\n",
            color::fail_start, color::reset, fail_count, run_count, assertion_count);
    }

    return success;
}

namespace {
template<typename F>
void list_tests(const registry& r, F&& predicate) {
    for (std::size_t i = 0; i < r.test_count; ++i) {
        const auto& t = r.test_list[i];
        if (!predicate(t)) {
            continue;
        }

        if (!t.type.empty()) {
            printf(
                "%.*s [%.*s]\n", static_cast<int>(t.name.length()), t.name.data(),
                static_cast<int>(t.type.length()), t.type.data());
        } else {
            printf("%.*s\n", static_cast<int>(t.name.length()), t.name.data());
        }
    }
}

std::vector<std::string_view> split_tags(std::string_view s) {
    std::vector<std::string_view> tags;

    std::string_view delim    = ",";
    std::size_t      pos      = s.find(delim);
    std::size_t      last_pos = 0u;
    std::size_t      cur_size = 0u;

    while (pos != std::string_view::npos) {
        cur_size = pos - last_pos;
        if (cur_size != 0) {
            tags.push_back(s.substr(last_pos, cur_size));
        }
        last_pos = pos + delim.size();
        pos      = s.find(delim, last_pos);
    }

    tags.push_back(s.substr(last_pos));

    return tags;
}

} // namespace

void registry::list_all_tags() const {
    std::set<std::string_view> tags;
    for (std::size_t i = 0; i < test_count; ++i) {
        const auto& t = test_list[i];

        for (auto sv : split_tags(t.tags)) {
            tags.insert(sv);
        }
    }

    for (auto c : tags) {
        printf("%.*s\n", static_cast<int>(c.length()), c.data());
    }
}

void registry::list_all_tests() const {
    list_tests(*this, [](const test_case&) { return true; });
}

void registry::list_tests_with_tag(std::string_view tag) const {
    list_tests(*this, [&](const test_case& t) {
        for (auto sv : split_tags(t.tags)) {
            if (sv == tag) {
                return true;
            }
        }
        return false;
    });
}

registry tests;
} // namespace testing

int main(int argc, char* argv[]) {
    cxxopts::Options options(argv[0], "Snatch test runner");

    // clang-format off
    options.add_options()
        ("l,list-tests", "List tests by name")
        ("list-tests-with-tag", "List tests by name with a given tag", cxxopts::value<std::string>())
        ("list-tags", "List tags by name")
        ("h,help", "Print help");
    // clang-format on

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

    return testing::tests.run_all();

    // } else {
    //     return testing::tests.run_selected(argc - 1);
    // }
}
