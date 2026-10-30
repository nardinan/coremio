cmake_minimum_required(VERSION 3.24)

project(coremio VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
if(APPLE AND NOT DEFINED OPENSSL_ROOT_DIR)
  set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)
endif()
find_package(OpenSSL 3 REQUIRED)

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
  src/tls.c
  src/http_payload.c
  src/database.c
  src/neural_networks/feedforward_neural_network.c
  src/neural_networks/recurrent_neural_network.c
)
add_library(coremio ${coremio_codebase})
target_include_directories(coremio PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(coremio PRIVATE
  Threads::Threads
  SQLite3::SQLite3
  OpenSSL::SSL
  OpenSSL::Crypto
)
