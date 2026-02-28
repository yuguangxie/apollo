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

add_executable(test_known_payload tests/test_known_payload.cc)
target_link_libraries(test_known_payload PRIVATE demo_can_codec)

add_executable(encode_decode_demo examples/encode_decode_demo.cc)
target_link_libraries(encode_decode_demo PRIVATE demo_can_codec)

enable_testing()
add_test(NAME test_generated_codec COMMAND test_generated_codec)
add_test(NAME test_known_payload COMMAND test_known_payload)
