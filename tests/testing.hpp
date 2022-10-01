// #include <catch2/catch_template_test_macros.hpp>

// // Replace Catch2's REQUIRE_THROWS_AS, which allocates on Windows;
// // this confuses our memory leak checks.
// #undef REQUIRE_THROWS_AS
// #define REQUIRE_THROWS_AS(EXPRESSION, EXCEPTION) \
//     do { \
//         try { \
//             EXPRESSION; \
//             FAIL("no exception thrown"); \
//         } catch (const EXCEPTION&) { \
//         } catch (const std::exception& e) { \
//             FAIL(e.what()); \
//         } catch (...) { \
//             FAIL("unexpected exception thrown"); \
//         } \
//     } while (0)

#include "snatch.hpp"
