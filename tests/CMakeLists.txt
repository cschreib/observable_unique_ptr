function(add_platform_definitions TARGET)
  target_compile_features(${TARGET} INTERFACE cxx_std_20)
  if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
      target_compile_definitions(${TARGET} PRIVATE OUP_PLATFORM_WASM)
      target_compile_definitions(${TARGET} PRIVATE OUP_COMPILER_EMSCRIPTEN)
  elseif (APPLE)
      target_compile_definitions(${TARGET} PRIVATE OUP_PLATFORM_OSX)
  elseif (UNIX)
      target_compile_definitions(${TARGET} PRIVATE OUP_PLATFORM_LINUX)
  elseif (WIN32)
      target_compile_definitions(${TARGET} PRIVATE OUP_PLATFORM_WINDOWS)
      if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
          target_compile_definitions(${TARGET} PRIVATE OUP_COMPILER_MSVC)
      endif()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(${TARGET} PRIVATE -Wall)
      target_compile_options(${TARGET} PRIVATE -Wextra)
      target_compile_options(${TARGET} PRIVATE -Werror)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
      target_compile_options(${TARGET} PRIVATE -Wall)
      target_compile_options(${TARGET} PRIVATE -Wextra)
      target_compile_options(${TARGET} PRIVATE -Werror)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET} PRIVATE /W4)
      target_compile_options(${TARGET} PRIVATE /WX)
      target_compile_options(${TARGET} PRIVATE /EHs)
    endif()
endfunction()

include(FetchContent)

FetchContent_Declare(doctest
                     GIT_REPOSITORY https://github.com/doctest/doctest.git
                     GIT_TAG        v2.4.9)
FetchContent_MakeAvailable(doctest)

set(RUNTIME_TEST_FILES
  ${PROJECT_SOURCE_DIR}/tests/testing.cpp
  ${PROJECT_SOURCE_DIR}/tests/tests_common.cpp
  ${PROJECT_SOURCE_DIR}/tests/memory_tracker.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_owner_misc.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_owner_construction.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_owner_assignment_move.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_owner_comparison.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_owner_cast_move.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_lifetime.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_make_observable.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_misc.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_construction.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_construction_from_owner.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_construction_copy.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_construction_move.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_assignment_from_owner.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_assignment_copy.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_assignment_move.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_comparison.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_cast_copy.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_cast_move.cpp
  ${PROJECT_SOURCE_DIR}/tests/runtime_tests_observer_from_this.cpp)

add_executable(oup_runtime_tests ${RUNTIME_TEST_FILES})
target_link_libraries(oup_runtime_tests PRIVATE oup::oup)
target_link_libraries(oup_runtime_tests PRIVATE doctest::doctest)
add_platform_definitions(oup_runtime_tests)

add_custom_target(oup_runtime_tests_run
  COMMAND oup_runtime_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES ${RUNTIME_TEST_FILES}
)
set_target_properties(oup_runtime_tests_run PROPERTIES EXCLUDE_FROM_ALL True)

# Compile-time error tests
set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
message(STATUS "Running compile-time tests...")

function(run_compile_test TEST_NAME TEST_FILE EXPECTED)
  try_compile(COMPILE_TEST_RESULT
    ${PROJECT_SOURCE_DIR}/tests
    SOURCES
    ${PROJECT_SOURCE_DIR}/tests/${TEST_FILE}
    ${PROJECT_SOURCE_DIR}/tests/tests_common.cpp
    CMAKE_FLAGS
      "-DINCLUDE_DIRECTORIES=${PROJECT_SOURCE_DIR}/include"
      "-DCMAKE_CXX_STANDARD=20")

  if(COMPILE_TEST_RESULT STREQUAL EXPECTED)
    message(STATUS "Test ${TEST_NAME} passed.")
  else()
    message(WARNING "FAILED test: ${TEST_NAME}, expected ${EXPECTED} and got ${COMPILE_TEST_RESULT}")
  endif()
endfunction()

run_compile_test("does_compilation_work" compile_test_good.cpp TRUE)
run_compile_test("is_copy_constructor_allowed" compile_test_copy_const.cpp FALSE)
run_compile_test("is_copy_assignment_allowed" compile_test_copy_assign.cpp FALSE)
run_compile_test("is_implicit_constructor_base_to_derived_allowed_acquire" compile_test_implicit_const_base_to_derived1.cpp FALSE)
run_compile_test("is_implicit_constructor_base_to_derived_allowed_move" compile_test_implicit_const_base_to_derived2.cpp FALSE)
run_compile_test("is_implicit_constructor_base_to_derived_allowed_move_with_deleter" compile_test_implicit_const_base_to_derived3.cpp FALSE)
run_compile_test("is_observer_construct_raw_allowed" compile_test_observer_construct_raw.cpp FALSE)
run_compile_test("is_observer_assign_raw_allowed" compile_test_observer_assign_raw.cpp FALSE)
run_compile_test("is_acquire_construct_raw_allowed" compile_test_sealed_construct_raw.cpp FALSE)
run_compile_test("is_acquire_assign_raw_allowed" compile_test_sealed_assign_raw.cpp FALSE)
run_compile_test("is_sealed_release_allowed" compile_test_sealed_release.cpp FALSE)
run_compile_test("is_sealed_reset_allowed" compile_test_sealed_reset.cpp FALSE)

message(STATUS "Running compile-time tests ended.")

# Benchmarks
add_executable(oup_size_benchmark
  ${PROJECT_SOURCE_DIR}/tests/memory_tracker.cpp
  ${PROJECT_SOURCE_DIR}/tests/size_benchmark.cpp)
target_link_libraries(oup_size_benchmark PRIVATE oup::oup)
add_platform_definitions(oup_size_benchmark)

add_executable(oup_speed_benchmark
  ${PROJECT_SOURCE_DIR}/tests/speed_benchmark.cpp
  ${PROJECT_SOURCE_DIR}/tests/speed_benchmark_utility.cpp
  ${PROJECT_SOURCE_DIR}/tests/speed_benchmark_utility2.cpp)
target_link_libraries(oup_speed_benchmark PRIVATE oup::oup)
add_platform_definitions(oup_speed_benchmark)
