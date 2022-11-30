#include "boost/ut.hpp"

using namespace boost::ut;

#define REQUIRE(...) ::boost::ut::expect(::boost::ut::that % __VA_ARGS__)
#define CHECK(...) ::boost::ut::expect(::boost::ut::that % __VA_ARGS__)
#define FAIL(...) ::boost::ut::expect(::boost::ut::that % false)
#define FAIL_CHECK(...) ::boost::ut::expect(::boost::ut::that % false)
#define SKIP(...)
#define TEST_CASE(name, tags) name##_test = [=]() mutable
#define TEMPLATE_LIST_TEST_CASE(name, tags, types) name##_test = [=]<typename TestType>() mutable

#define REQUIRE_THROWS_AS(EXPRESSION, EXCEPTION) REQUIRE(throws([&]() { EXPRESSION; }))
#define CHECK_THROWS_AS(EXPRESSION, EXCEPTION) REQUIRE(throws([&]() { EXPRESSION; }))
#define REQUIRE_THROWS_MATCHES(EXPRESSION, EXCEPTION, MATCHER)                                     \
    REQUIRE(throws([&]() { EXPRESSION; }))
#define CHECK_THROWS_MATCHES(EXPRESSION, EXCEPTION, MATCHER) REQUIRE(throws([&]() { EXPRESSION; }))
#define SUITE static ::boost::ut::suite _ = []

namespace snatch::impl {
template<typename T>
constexpr std::string_view get_type_name() noexcept {
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
} // namespace snatch::impl

namespace snatch {
template<typename T>
constexpr std::string_view type_name = impl::get_type_name<T>();
} // namespace snatch
