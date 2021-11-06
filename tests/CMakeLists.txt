# set OS preprocessor defines
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(OUP_PLATFORM_WASM TRUE)
    set(OUP_COMPILER_EMSCRIPTEN TRUE)
elseif (APPLE)
    set(OUP_PLATFORM_OSX TRUE)
elseif (UNIX)
    set(OUP_PLATFORM_LINUX TRUE)
elseif (WIN32)
    set(OUP_PLATFORM_WINDOWS TRUE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(OUP_COMPILER_MSVC TRUE)
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        912df7df354fd6c814b5d7c45ac44f494a5fb5fe
)

FetchContent_MakeAvailable(Catch2)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Catch2_SOURCE_DIR}/extras")

add_executable(oup_tests ${PROJECT_SOURCE_DIR}/tests/runtime_tests.cpp)
target_link_libraries(oup_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(oup_tests PRIVATE oup::oup)
target_compile_definitions(oup_tests PRIVATE
  OUP_PLATFORM_OSX
  OUP_PLATFORM_WASM
  OUP_PLATFORM_LINUX
  OUP_PLATFORM_WINDOWS
  OUP_COMPILER_MSVC
  OUP_COMPILER_EMSCRIPTEN)

include(CTest)
include(Catch)

catch_discover_tests(oup_tests)

# Compile-time error tests
set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
message(STATUS "Running compile-time tests...")

function(run_compile_test TEST_NAME TEST_FILE EXPECTED)
  try_compile(COMPILE_TEST_RESULT
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/tests/${TEST_FILE}
    CMAKE_FLAGS
      "-DINCLUDE_DIRECTORIES=${PROJECT_SOURCE_DIR}/include"
      "-DCMAKE_CXX_STANDARD=17")

  if(COMPILE_TEST_RESULT STREQUAL EXPECTED)
    message(STATUS "Test ${TEST_NAME} passed.")
  else()
    message(WARNING "FAILED test: ${TEST_NAME}, expected ${EXPECTED} and got ${COMPILE_TEST_RESULT}")
  endif()
endfunction()

run_compile_test("does_compilation_works" compile_test_good.cpp TRUE)
run_compile_test("is_copy_constructor_allowed" compile_test_copy_const.cpp FALSE)
run_compile_test("is_copy_assignment_allowed" compile_test_copy_assign.cpp FALSE)
run_compile_test("is_implicit_constructor_base_to_derived_allowed_acquire" compile_test_implicit_const_base_to_derived1.cpp FALSE)
run_compile_test("is_implicit_constructor_base_to_derived_allowed_move" compile_test_implicit_const_base_to_derived2.cpp FALSE)
run_compile_test("is_implicit_constructor_base_to_derived_allowed_move_with_deleter" compile_test_implicit_const_base_to_derived3.cpp FALSE)

message(STATUS "Running compile-time tests ended.")

# Benchmarks

add_executable(oup_size_benchmark ${PROJECT_SOURCE_DIR}/tests/size_benchmark.cpp)
target_link_libraries(oup_size_benchmark PRIVATE oup::oup)
