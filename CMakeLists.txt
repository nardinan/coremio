cmake_minimum_required(VERSION 3.24)
project(coremio C)
find_package(Threads)
set(CMAKE_C_STANDARD 11)
set(coremio_codebase src/array.c
        include/coremio/array.h
        include/coremio/assert.h
        src/dictionary.c
        include/coremio/dictionary.h
        src/json.c
        include/coremio/json.h
        src/list.c
        include/coremio/list.h
        src/local.string.c
        include/coremio/local.string.h
        src/memory.c
        include/coremio/memory.h
        src/red_black_tree.c
        include/coremio/red_black_tree.h
        src/result.c
        include/coremio/result.h
        src/tokens.c
        include/coremio/tokens.h
        include/coremio/coremio.h
        include/coremio/boxed_nan.h
        src/boxed_nan.c
        include/coremio/runner.h
        src/runner.c
        include/coremio/server.h
        src/server.c
        include/coremio/http_payload.h
        src/http_payload.c
        src/neural_networks/feedforward_neural_network.c
        include/coremio/neural_networks/feedforward_neural_network.h
        include/coremio/neural_networks/recurrent_neural_network.h
        src/neural_networks/recurrent_neural_network.c)
add_library(coremio ${coremio_codebase})
target_link_libraries(coremio PUBLIC Threads::Threads)


