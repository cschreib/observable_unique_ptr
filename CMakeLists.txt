cmake_minimum_required(VERSION 3.8)

project(oup LANGUAGES CXX VERSION 1.0)

add_library(oup INTERFACE)
add_library(oup::oup ALIAS oup)
set_target_properties(oup PROPERTIES EXPORT_NAME oup::oup)

target_sources(oup INTERFACE ${PROJECT_SOURCE_DIR}/include/oup/observable_unique_ptr.hpp)
target_include_directories(oup INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(oup INTERFACE cxx_std_17)

install(FILES ${PROJECT_SOURCE_DIR}/include/oup/observable_unique_ptr.hpp DESTINATION include/oup)
install(TARGETS oup EXPORT oup)

if (OUP_DO_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()
