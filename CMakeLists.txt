cmake_minimum_required(VERSION 3.24)
project(coremio C)

set(CMAKE_C_STANDARD 11)

add_library(coremio
  src/array.c
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
  include/coremio/lisp.h
  src/lisp.c
  include/coremio/boxed_nan.h
  src/boxed_nan.c)
