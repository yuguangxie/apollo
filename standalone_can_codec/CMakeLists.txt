cmake_minimum_required(VERSION 3.16)
project(standalone_can_codec CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(demo_can_codec
  examples/generated/demo_can_codec.cc
)

target_include_directories(demo_can_codec PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/generated
)

add_executable(test_generated_codec tests/test_generated_codec.cc)
target_link_libraries(test_generated_codec PRIVATE demo_can_codec)

enable_testing()
add_test(NAME test_generated_codec COMMAND test_generated_codec)
