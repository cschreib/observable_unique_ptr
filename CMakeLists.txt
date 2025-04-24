cmake_minimum_required(VERSION 3.8..3.31)

# Setup main project
project(oup LANGUAGES CXX VERSION 0.7.3)

# Create library (header-only)
add_library(oup INTERFACE)
add_library(oup::oup ALIAS oup)
set_target_properties(oup PROPERTIES EXPORT_NAME oup::oup)

target_sources(oup INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/oup/observable_unique_ptr.hpp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/oup/observable_unique_ptr.hpp>)
target_include_directories(oup INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
target_compile_features(oup INTERFACE cxx_std_17)

# Setup install target and exports
install(FILES ${PROJECT_SOURCE_DIR}/include/oup/observable_unique_ptr.hpp
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/oup)
install(TARGETS oup EXPORT oup-targets)

install(EXPORT oup-targets
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/oup COMPONENT Development)

export(EXPORT oup-targets)

# Setup CMake config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/oup-config.cmake.in"
  "${PROJECT_BINARY_DIR}/oup-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  NO_SET_AND_CHECK_MACRO)

install(FILES
  "${PROJECT_BINARY_DIR}/oup-config.cmake"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/oup COMPONENT Development)

# Setup tests
if (OUP_DO_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()
