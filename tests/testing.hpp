#include "doctest/doctest.h"

#include <string_view>

#undef TEST_CASE
#define TEST_CASE(name, tags) DOCTEST_TEST_CASE(tags " " name)
#define TEMPLATE_TEST_CASE(name, tags, ...)                                                        \
    DOCTEST_TEST_CASE_TEMPLATE(tags " " name, TestType, __VA_ARGS__)

#define TEMPLATE_LIST_TEST_CASE(name, tags, types)                                                 \
    TEST_CASE_TEMPLATE(tags " " name, TestType, types)

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
