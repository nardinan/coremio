cmake_minimum_required(VERSION 3.24)
project(core C)

set(CMAKE_C_STANDARD 11)

add_library(coremio
  src/array.c
  include/core/array.h
  src/assert.c
  include/core/assert.h
  src/dictionary.c
  include/core/dictionary.h
  src/json.c
  include/core/json.h
  src/list.c
  include/core/list.h
  src/local.string.c
  include/core/local.string.h
  src/memory.c
  include/core/memory.h
  src/red_black_tree.c
  include/core/red_black_tree.h
  src/result.c
  include/core/result.h
  src/tokens.c
  include/core/tokens.h
  )
