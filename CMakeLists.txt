cmake_minimum_required(VERSION 3.24)

project(coremio VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(coremio_codebase
  src/result.c
  src/memory.c
  src/list.c
  src/array.c
  src/red_black_tree.c
  src/dictionary.c
  src/boxed_nan.c
  src/local.string.c
  src/tokens.c
  src/json.c
  src/runner.c
  src/server.c
  src/http_payload.c
  src/neural_networks/feedforward_neural_network.c
  src/neural_networks/recurrent_neural_network.c
)
add_library(coremio ${coremio_codebase})
target_include_directories(coremio PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(coremio PUBLIC Threads::Threads)
