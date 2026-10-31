cmake_minimum_required(VERSION 3.16)
project(SLeeLaImaging VERSION 2.9.0 LANGUAGES CXX)

add_executable(sleela-imaging native/imaging.cpp)
target_compile_features(sleela-imaging PRIVATE cxx_std_17)
set_target_properties(sleela-imaging PROPERTIES CXX_EXTENSIONS OFF)
if(MSVC)
  target_compile_options(sleela-imaging PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(sleela-imaging PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME imaging-integration
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_imaging.py $<TARGET_FILE:sleela-imaging>)
