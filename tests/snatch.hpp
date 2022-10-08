#ifndef SNATCH_HPP
#define SNATCH_HPP

#include <array>
#include <cstdio>
#include <string>
#include <string_view>
#include <tuple>

#if !defined(SNATCH_MAX_TEST_CASES)
#    define SNATCH_MAX_TEST_CASES 5'000
#endif
#if !defined(SNATCH_MAX_EXPR_LENGTH)
#    define SNATCH_MAX_EXPR_LENGTH 1'024
#endif
#if !defined(SNATCH_MAX_TEST_NAME_LENGTH)
#    define SNATCH_MAX_TEST_NAME_LENGTH 1'024
#endif

// Testing framework configuration.
// --------------------------------

namespace testing {
// Maximum number of test cases in the whole program.
// A "test case" is created for each uses of the `*_TEST_CASE` macros,
// and for each type for the `TEMPLATE_LIST_TEST_CASE` macro.
constexpr std::size_t max_test_cases = SNATCH_MAX_TEST_CASES;
// Maximum length of a `CHECK(...)` or `REQUIRE(...)` expression,
// beyond which automatic variable printing is disabled.
constexpr std::size_t max_expr_length = SNATCH_MAX_EXPR_LENGTH;
// Maximum length of a full test case name.
// The full test case name includes the base name, plus any type.
constexpr std::size_t max_test_name_length = SNATCH_MAX_TEST_NAME_LENGTH;
} // namespace testing

// Forward declarations.
// ---------------------

namespace testing {
struct registry;
}

// Implementation details.
// -----------------------

namespace testing::impl {
template<typename T>
constexpr std::string_view get_type_name() {
#if defined(__clang__)
    constexpr auto prefix   = std::string_view{"[T = "};
    constexpr auto suffix   = "]";
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
    constexpr auto prefix   = std::string_view{"with T = "};
    constexpr auto suffix   = "; ";
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
    constexpr auto prefix   = std::string_view{"get_type_name<"};
    constexpr auto suffix   = ">(void)";
    constexpr auto function = std::string_view{__FUNCSIG__};
#else
#    error Unsupported compiler
#endif

    const auto start = function.find(prefix) + prefix.size();
    const auto end   = function.find(suffix);
    const auto size  = end - start;

    return function.substr(start, size);
}

template<typename T>
struct proxy;

template<typename... Args>
struct proxy<std::tuple<Args...>> {
    registry*        tests = nullptr;
    std::string_view name;
    std::string_view tags;

    template<typename F>
    const char* operator=(const F& func);
};

struct test_case;

using test_ptr = void (*)(test_case&);

template<typename T, typename F>
constexpr test_ptr to_test_case_ptr(const F&) {
    return [](test_case& t) { F{}.template operator()<T>(t); };
}

enum class test_state { not_run, success, skipped, failed };

struct test_case {
    std::string_view name;
    std::string_view tags;
    std::string_view type;
    test_ptr         func  = nullptr;
    test_state       state = test_state::not_run;
    std::size_t      tests = 0;
};

namespace color {
extern const char* error_start;
extern const char* warning_start;
extern const char* status_start;
extern const char* fail_start;
extern const char* skipped_start;
extern const char* pass_start;
extern const char* highlight1_start;
extern const char* highlight2_start;
extern const char* reset;
} // namespace color

struct small_string {
    std::array<char, max_expr_length> data;
    std::size_t                       data_length = 0;

    std::string_view str() const;
    std::size_t      available() const;
    std::size_t      size() const;
    std::size_t      length() const;
    bool             empty() const;
    void             clear();
};

[[nodiscard]] bool append(small_string& ss, std::string_view value);

[[nodiscard]] bool append(small_string& ss, const void* ptr);
[[nodiscard]] bool append(small_string& ss, std::nullptr_t);
[[nodiscard]] bool append(small_string& ss, std::size_t i);
[[nodiscard]] bool append(small_string& ss, std::ptrdiff_t i);
[[nodiscard]] bool append(small_string& ss, float f);
[[nodiscard]] bool append(small_string& ss, double f);
[[nodiscard]] bool append(small_string& ss, bool value);
[[nodiscard]] bool append(small_string& ss, const std::string& str);
[[nodiscard]] bool append(small_string& ss, const char* str);
template<std::size_t N>
[[nodiscard]] bool append(small_string& ss, const char (&str)[N]) {
    return append(ss, std::string_view(str));
}

template<typename T, typename U, typename... Args>
[[nodiscard]] bool append(small_string& ss, T&& t, U&& u, Args&&... args) {
    if (!append(ss, std::forward<T>(t))) {
        return false;
    }
    return append(ss, std::forward<U>(u), std::forward<Args>(args)...);
}

void truncate_end(small_string& ss);

struct expression {
    small_string data;
    bool         failed = false;

    template<typename T>
    void append(T&& value) {
        using TD = std::decay_t<T>;
        if constexpr (std::is_integral_v<TD>) {
            if constexpr (std::is_signed_v<TD>) {
                if (!impl::append(data, static_cast<std::ptrdiff_t>(value))) {
                    failed = true;
                }
            } else {
                if (!impl::append(data, static_cast<std::size_t>(value))) {
                    failed = true;
                }
            }
        } else if constexpr (
            std::is_pointer_v<TD> || std::is_floating_point_v<TD> || std::is_same_v<TD, bool> ||
            std::is_convertible_v<T, const char*> || std::is_convertible_v<T, std::string> ||
            std::is_convertible_v<T, std::string_view>) {
            if (!impl::append(data, value)) {
                failed = true;
            }
        } else {
            failed = true;
        }
    }

#define EXPR_OPERATOR(OP, INVERSE_OP)                                                              \
    template<typename T>                                                                           \
    expression& operator OP(const T& value) {                                                      \
        if (data.empty()) {                                                                        \
            append(" " #INVERSE_OP " ");                                                           \
        }                                                                                          \
        append(value);                                                                             \
        return *this;                                                                              \
    }

    EXPR_OPERATOR(<=, >)
    EXPR_OPERATOR(<, >=)
    EXPR_OPERATOR(>=, <)
    EXPR_OPERATOR(>, <=)
    EXPR_OPERATOR(==, !=)
    EXPR_OPERATOR(!=, ==)

#undef EXPR_OPERATOR
};
} // namespace testing::impl

// Test registry.
// --------------

namespace testing {
struct registry {
    std::array<impl::test_case, max_test_cases> test_list;
    std::size_t                                 test_count = 0;

    bool verbose = false;

    impl::proxy<std::tuple<>> add(std::string_view name, std::string_view tags) {
        return {this, name, tags};
    }

    template<typename T>
    impl::proxy<T> add_with_types(std::string_view name, std::string_view tags) {
        return {this, name, tags};
    }

    void register_test(
        std::string_view name, std::string_view tags, std::string_view type, impl::test_ptr func);

    template<typename... Args, typename F>
    void register_tests(std::string_view name, std::string_view tags, const F& func) {
        (register_test(name, tags, impl::get_type_name<Args>(), impl::to_test_case_ptr<Args>(func)),
         ...);
    }

    void print_location(
        const impl::test_case& current_case, const char* filename, int line_number) const;

    void print_failure() const;
    void print_skip() const;
    void print_details(std::string_view message) const;
    void print_details_expr(
        std::string_view check, std::string_view exp_str, const impl::expression& exp) const;

    void run(impl::test_case& t);
    void set_state(impl::test_case& t, impl::test_state s);

    bool run_all_tests();
    bool run_tests_matching_name(std::string_view name);
    bool run_tests_with_tag(std::string_view tag);

    void list_all_tests() const;
    void list_all_tags() const;
    void list_tests_with_tag(std::string_view tag) const;
};

extern registry tests;
} // namespace testing

// Implementation details.
// -----------------------

namespace testing::impl {
template<typename... Args>
template<typename F>
const char* proxy<std::tuple<Args...>>::operator=(const F& func) {
    if constexpr (sizeof...(Args) > 0) {
        tests->template register_tests<Args...>(name, tags, func);
    } else {
        tests->register_test(name, tags, {}, func);
    }
    return name.data();
}
} // namespace testing::impl

// Test macros.
// ------------

#define TESTING_CONCAT_IMPL(x, y) x##y
#define TESTING_MACRO_CONCAT(x, y) TESTING_CONCAT_IMPL(x, y)
#define TESTING_EXPR(x) testing::impl::expression{} <= x

#define TEST_CASE(NAME, TAGS)                                                                      \
    static const char* TESTING_MACRO_CONCAT(test_id_, __COUNTER__) =                               \
        testing::tests.add(NAME, TAGS) =                                                           \
            [](testing::impl::test_case & CURRENT_CASE [[maybe_unused]]) -> void

#define TEMPLATE_LIST_TEST_CASE(NAME, TAGS, TYPES)                                                 \
    static const char* TESTING_MACRO_CONCAT(test_id_, __COUNTER__) =                               \
        testing::tests.add_with_types<TYPES>(NAME, TAGS) = []<typename TestType>(                  \
            testing::impl::test_case & CURRENT_CASE [[maybe_unused]]) -> void

#define REQUIRE(EXP)                                                                               \
    do {                                                                                           \
        ++CURRENT_CASE.tests;                                                                      \
        if (!(EXP)) {                                                                              \
            const auto EXP2 = TESTING_EXPR(EXP);                                                   \
            testing::tests.print_failure();                                                        \
            testing::tests.print_location(CURRENT_CASE, __FILE__, __LINE__);                       \
            testing::tests.print_details_expr("REQUIRE", #EXP, EXP2);                              \
            throw testing::impl::test_state::failed;                                               \
        }                                                                                          \
    } while (0)

#define CHECK(EXP)                                                                                 \
    do {                                                                                           \
        ++CURRENT_CASE.tests;                                                                      \
        if (!(EXP)) {                                                                              \
            const auto EXP2 = TESTING_EXPR(EXP);                                                   \
            testing::tests.print_failure();                                                        \
            testing::tests.print_location(CURRENT_CASE, __FILE__, __LINE__);                       \
            testing::tests.print_details_expr("CHECK", #EXP, EXP2);                                \
            testing::tests.set_state(CURRENT_CASE, testing::impl::test_state::failed);             \
        }                                                                                          \
    } while (0)

#define FAIL(MESSAGE)                                                                              \
    do {                                                                                           \
        testing::tests.print_failure();                                                            \
        testing::tests.print_location(CURRENT_CASE, __FILE__, __LINE__);                           \
        testing::tests.print_details(MESSAGE);                                                     \
        throw testing::impl::test_state::failed;                                                   \
    } while (0)

#define SKIP(MESSAGE)                                                                              \
    do {                                                                                           \
        testing::tests.print_skip();                                                               \
        testing::tests.print_location(CURRENT_CASE, __FILE__, __LINE__);                           \
        testing::tests.print_details(MESSAGE);                                                     \
        throw testing::impl::test_state::skipped;                                                  \
    } while (0)

#define REQUIRE_THROWS_AS(EXPRESSION, EXCEPTION)                                                   \
    do {                                                                                           \
        try {                                                                                      \
            EXPRESSION;                                                                            \
            FAIL("no exception thrown");                                                           \
        } catch (const EXCEPTION&) {                                                               \
        } catch (const std::exception& e) {                                                        \
            FAIL(e.what());                                                                        \
        } catch (...) {                                                                            \
            FAIL("unexpected exception thrown");                                                   \
        }                                                                                          \
    } while (0)

#endif
