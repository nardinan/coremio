![alt text](coremio.png "libcoremio")

## Introduction

coremio is a personal C11 library.

Before explaining what coremio *is*, it is useful to clarify what it is *not*. It is not a framework, not a platform, and not an attempt to modernize C by layering abstractions on top of it. coremio does not try to hide the language, soften its edges, or pretend that C can be something else if wrapped carefully enough.

coremio grew organically while working on other projects, when rewriting the same low-level building blocks stopped being instructive and started becoming a distraction. Over time, those blocks stabilized, their interfaces stopped changing, and they became boring enough to deserve a permanent place. That place is this repository.

The library has been developed as the technical foundation behind **The Barfing Fox**, my personal “virtual company” and umbrella for a collection of experimental, creative, and sometimes deliberately odd software projects. All The Barfing Fox projects share the same low-level needs: predictable memory handling, explicit data structures, simple parsers, and utilities that do not try to be smarter than the programmer. coremio exists to satisfy those needs and to provide a stable base on which everything else can be built.

This README has been heavily reviewed, rewritten, and reformatted with the help of ChatGPT. This is intentional. The goal is to keep a clear “2026 scent” in the documentation and to explicitly acknowledge the use of AI as a technical writing tool, even for a library written in plain C. coremio embraces pragmatic tooling, and that includes how it is documented.

coremio is permanently under development. Only the `master` branch is considered stable; everything else may change, break, or disappear without notice. If you decide to depend on it, depend on `master`.

---

## How to read this documentation

This document is not meant to be a reference manual. It is written as a guided tour, closer to a technical essay than to an API listing. The goal is not to enumerate every function exhaustively, but to explain the reasoning behind the library, the patterns it uses, and the way its pieces are meant to fit together.

Each chapter introduces a module by first explaining *why it exists* and *which problem it tries to solve*. Only after that does it move to concrete usage and code examples. Functions are described narratively and in context, not as isolated entries. Examples are intentionally simple and sometimes repetitive: repetition here is a tool to reinforce mental models, not an oversight.

The text is deliberately compact and essay-like. Line breaks are kept to a minimum so the document reads like a technical chapter rather than a collection of snippets. If you are looking for a quick overview, this may feel verbose. If you are comfortable reading technical explanations as you would read a book, this is the intended pace.

---

## Integrating coremio into a project

coremio is meant to be vendored. The recommended approach is to include it as a git submodule inside your project. This keeps the dependency explicit, versioned, and fully under your control, and avoids the ambiguity that often comes with system-wide installations or prebuilt binaries. coremio does not try to hide the fact that it is a dependency; instead, it embraces being part of your source tree.

Starting from now, I make a very honest effort to **not** change the signature or the behavior of already published public APIs. This is not a legally binding contract, and there is no blood oath involved, but the intent is real: if a function has been published and documented, I will try very hard to leave it alone. When a breaking change becomes technically unavoidable, it will be documented clearly and treated as an exception, not as the norm.

When using CMake, integration is deliberately straightforward. You add the library as a subdirectory, link against it, and expose its include path:

```cmake
add_subdirectory(external/coremio)
target_link_libraries(your_target PRIVATE coremio)
target_include_directories(your_target PRIVATE external/coremio/include)
```

Public headers live under `include/coremio/`, while implementation files live under `src/`. There is no umbrella header. You are expected to include only what you actually use:

```c
#include "coremio/result.h"
#include "coremio/list.h"
#include "coremio/json.h"
```

This makes dependencies explicit at the source level and avoids the accidental coupling that often comes from including large, catch-all headers.

---

## Conventions and general structure

Before looking at individual modules, it is worth spending a moment on the conventions used throughout coremio. They are simple, consistent, and deliberately visible, because the library prefers readability and predictability over cleverness.

Public functions start with `f_`. Private or internal helpers start with `p_`. Macros start with `d_`. This is not meant to be decorative; it allows you to understand, at a glance, whether a symbol is part of the public surface of the library or an internal detail that you are not expected to rely on.

Types follow a similarly explicit naming scheme. Structures start with `s_`, typedefs with `t_`, and enums with `e_`. The goal is not to enforce a specific coding religion, but to make types immediately recognizable while reading code, especially in contexts where C’s type system is intentionally minimal.

Memory allocation is centralized. Error handling is explicit. Most containers are intrusive. These are not independent design decisions: they reinforce each other. Centralized allocation makes ownership visible, intrusive containers make lifetime management explicit, and explicit error handling keeps failure paths readable and local. Together, they form a style that favors clarity and mechanical sympathy over abstraction layers.

If these conventions feel rigid, that is by design. coremio optimizes for consistency across modules, not for local freedom in each file.

---

## `result`  -  making failures explicit and uninteresting

The `result` module is the backbone of coremio. Almost every other module depends on it, either directly or indirectly. The idea is simple: instead of returning magic numbers, sentinel values, or relying on `errno`, functions return a `coremio_result`. A `coremio_result` is a pointer to a shared, static structure describing an error. When everything goes well, the function returns `NOICE`.

```c
coremio_result rc = f_json_explode_buffer(text, &json);
if (rc != NOICE) {
  fprintf(stderr, "%s (%u): %s
",
          rc->name,
          rc->code,
          rc->description);
  return 1;
}
```

Each result object contains a numeric code, a short symbolic name, and a human-readable description. Because results are static and shared, they can be compared by pointer, logged directly, and propagated without allocation. This keeps error handling cheap and predictable, and avoids the temptation to dynamically allocate error messages in failure paths.

One important aspect of the `result` module is that it is not closed. Through the use of the macros `d_result_declare` and `d_result_define`, users can define their own error sets, scoped within their software, without modifying coremio itself. This allows each project to extend the error vocabulary in a controlled way, while still using the same mechanism and the same handling patterns.

A typical extension looks like this:

```c
/* header */
d_result_declare(MY_SHIT_WENT_SIDEWAYS);

/* source */
d_result_define(
  MY_SHIT_WENT_SIDEWAYS,
  1001,
  "MY_SHIT_WENT_SIDEWAYS",
  "Something went wrong, and it was not coremio's fault"
);
```

From that point on, `MY_SHIT_WENT_SIDEWAYS` behaves exactly like any built-in result: it can be returned, compared, logged, and propagated. The scope of the error remains local to your project, but the semantics remain uniform.

The goal of this module is not to be elegant or expressive. It is to make failure paths explicit, short, and boring. When something fails, you should be able to see it immediately, understand why it failed, and move on.

---

## `memory`  -  one place to think about allocation

Memory management is one of the few areas where small inconsistencies tend to grow into long debugging sessions. The `memory` module exists to centralize allocation and to make memory behavior observable when needed. Instead of calling `malloc`, `realloc`, and `free` directly, coremio uses `d_malloc`, `d_realloc`, and `d_free`, which funnel all allocations through a single layer.

```c
char *buffer = d_malloc(256);
buffer = d_realloc(buffer, 512);
d_free(buffer);
```

At first glance this may look like a thin wrapper, and in many cases it is. The important difference is that coremio can optionally attach metadata to each allocation, allowing the library to track live memory blocks. In debug-oriented tools or test binaries, this makes it possible to print a flat list of allocations that were never released:

```c
f_memory_print_plain();
```

This information is intentionally simple and unstructured. The goal is not to provide a full profiler, but to answer a very basic question: *what is still alive, and where did it come from?*

It is important to understand that `d_malloc` and `d_free` are not interchangeable with the standard allocator. Calling `free()` on a pointer returned by `d_malloc()` results in undefined behavior. Conversely, calling `d_free()` on a pointer returned by `malloc()` is very likely to end up in a segmentation fault, because `d_free()` assumes the presence of metadata stored a few bytes before the returned pointer. Mixing allocators is therefore forbidden. Choose one allocation model and stick to it consistently across your codebase.

If you do not want any tracking at all, the module can be disabled entirely by defining `d_coremio_use_standard_malloc` before including the header. In that configuration, `d_malloc`, `d_realloc`, and `d_free` map directly to the standard allocator, and the rest of the library remains unaffected.

The purpose of this module is not to reinvent memory allocation. It is to make allocation behavior explicit, consistent, and easy to inspect when something goes wrong.

---

## `list`  -  intrusive lists and explicit ownership

The `list` module implements a doubly-linked intrusive list. Intrusive means that the list does not allocate nodes on its own. Instead, each object that participates in a list embeds a `s_list_node`. This is a deliberate design choice, and it has important consequences for ownership and lifetime management.

One strict rule follows from this design: the `s_list_node` **must be the first field** of the user-defined structure. This is not a stylistic preference. Placing the node first allows safe and direct casting between the list node pointer and the container pointer, without relying on offsets or additional indirection.

A minimal listable object therefore looks like this:

```c
typedef struct s_item {
  s_list_node head;
  int value;
} s_item;
```

A list itself is a lightweight structure that can be zero-initialized:

```c
s_list items = {0};
```

To add an element to the list, you allocate the object yourself and pass its pointer to the list API. Because the node is the first field, the object pointer can be safely cast to `s_list *`:

```c
s_item *a = d_malloc(sizeof(s_item));
a->value = 10;

f_list_append(&items, (s_list *)a, e_list_insert_tail);
```

Iteration returns the container pointer directly, not the embedded node. This keeps iteration code simple and avoids repetitive casts:

```c
s_item *it;
d_list_foreach(&items, it, s_item) {
  printf("%d\n", it->value);
}
```

Removing an element detaches it from the list but does not free it. Lifetime management remains the responsibility of the caller:

```c
f_list_remove(&items, (s_list *)a);
d_free(a);
```

This separation of concerns is intentional. The list owns the links; you own the memory. By making ownership explicit, the module avoids hidden allocations and implicit lifetimes, which are often the source of subtle bugs in non-intrusive container designs.

The `list` module does not try to be generic or abstract. It exists to support very common systems-level patterns - queues, registries, ownership graphs - using a small and predictable set of operations.

---

## `array`  -  a growable buffer that still feels like C

The `array` module provides a dynamically growing, contiguous memory buffer that behaves like a plain C pointer. There is no struct to pass around, no explicit “array object” to initialize and carry with you. You get a pointer, and you use it as such. The price you pay for this simplicity is that some metadata is stored transparently just before the pointer returned to you.

An array is created by specifying two parameters: the bucket size and the size of each element.

```c
int *values = f_array_malloc(16, sizeof(int));
```

The bucket size controls how the array grows. Internally, the buffer grows in chunks of `bucket * node_size` bytes. This means that the choice of bucket is a trade-off. A small bucket minimizes wasted memory but increases the number of reallocations as the array grows. A large bucket reduces the number of reallocations but may leave unused space at the end of the buffer. There is no universally correct value: parsers and token streams tend to benefit from small buckets, while sparse or jump-heavy access patterns often benefit from larger ones.

Before accessing an index, you must explicitly validate it:

```c
values = f_array_validate_access(values, 42);
values[42] = 123;
```

If the requested index exceeds the current size, the buffer is reallocated and the newly added memory is zero-filled. This makes it safe to treat the array as if it were always large enough, as long as access is validated beforehand.

Although the array behaves like a plain pointer, metadata is available through macros:

```c
printf("size=%zu bucket=%zu\n",
       d_array_size(values),
       d_array_bucket(values));
```

This information can be useful for diagnostics, debugging, or to tune bucket sizes based on real usage patterns. When the array is no longer needed, it must be released explicitly:

```c
f_array_free(values);
```

The `array` module is intentionally minimal. It does not provide iterators, bounds-checked accessors, or higher-level operations. Its purpose is to make a very common pattern - “I need a growable buffer, but I still want it to look like C” - simple and explicit.

---

## `dictionary`  -  structured lookup without ceremony

The `dictionary` module implements a string-keyed associative container. Conceptually, it answers a very simple question: given a key, give me the associated object, and if it does not exist yet, create it. Internally, it is built on top of a red-black tree, but that detail is intentionally hidden from the user. What matters at the API level is predictability and explicit ownership.

A dictionary owns its keys and its internal nodes. What it does not own is the semantic meaning of the stored data: that part is defined by the user through a custom payload structure. Each dictionary entry embeds a `s_dictionary_node`, which plays the same role as `s_list_node` in intrusive lists.

A minimal dictionary payload looks like this:

```c
typedef struct s_entry {
  s_dictionary_node head;
  int count;
} s_entry;
```

The dictionary itself must be explicitly initialized, and the size of the payload structure must be provided upfront:

```c
s_dictionary dict;
f_dictionary_initialize(&dict, sizeof(s_entry));
```

From this point on, interaction with the dictionary is intentionally straightforward. The most common operation is “get or create”:

```c
s_entry *e = (s_entry *)f_dictionary_get_or_create(&dict, "hello");
e->count++;
```

Iteration over the dictionary is done through an explicit callback-based traversal:

```c
void visit(s_dictionary_node *n, void *payload) {
  s_entry *e = (s_entry *)n;
  printf("%s -> %d\n", n->key, e->count);
}

f_dictionary_foreach(&dict, visit, NULL);
```

The traversal order reflects the internal ordering of the tree. The API does not promise insertion order, but it does guarantee that every entry will be visited exactly once.

When the dictionary is no longer needed, it must be released explicitly:

```c
f_dictionary_free(&dict);
```

The `dictionary` module exists to support a very common pattern in configuration systems, parsers, and runtime registries. It provides a predictable, explicit associative container that integrates cleanly with the rest of coremio’s design philosophy.

---

## `boxed_nan`  -  multiple values, one double

The `boxed_nan` module implements a technique commonly known as *NaN boxing*. The basic idea is to use the unused bit patterns of IEEE-754 NaN values to encode additional information. In practice, this allows a single `double` to represent not only floating-point numbers, but also integers, booleans, small strings, and pointers, all while remaining a plain C value that can be copied, passed, and returned cheaply.

The motivation for this module is uniformity. In many parts of coremio - tokenizers, parsers, small interpreters - it is convenient to move values around without constantly switching between unions, tagged structs, or heap-allocated objects. By collapsing multiple logical types into a single machine type, code becomes simpler to write and easier to reason about.

Creating boxed values is explicit and type-specific:

```c
double a = f_boxed_nan_int(123);
double b = f_boxed_nan_boolean(true);
double c = f_boxed_nan_double(3.14);
double s = f_boxed_nan_string("hello");
```

Each boxed value carries an internal signature that identifies what it represents. Before extracting the underlying data, the signature can be inspected:

```c
if (d_boxed_nan_get_signature(a) == d_boxed_nan_int_signature) {
  int v = d_boxed_nan_get_int(a);
}
```

String values deserve special attention. The `boxed_nan` module transparently supports both embedded strings and pointer-based strings. Short strings may be stored directly inside the NaN payload, while longer strings are stored elsewhere and referenced by pointer. From the user’s point of view, this distinction is intentionally invisible.

Retrieving a string always uses the same API:

```c
const char *str = d_boxed_nan_get_string(s);
printf("%s\n", str);
```

Whether the string was embedded or stored externally does not matter (except when you need to free the memory). The returned pointer is always a valid, null-terminated C string. Ownership rules depend on how the string was created, but access is uniform.

Comparing boxed values is equally explicit. Because values are encoded, direct comparison with `==` is not meaningful in most cases. Instead, helper functions are provided. For example, comparing two boxed strings:

```c
if (f_boxed_nan_string_compare(a, b) == 0) {
  /* strings are equal */
}
```

Or comparing boxed integers:

```c
if (d_boxed_nan_get_signature(x) == d_boxed_nan_int_signature &&
    d_boxed_nan_get_signature(y) == d_boxed_nan_int_signature &&
    d_boxed_nan_get_int(x) == d_boxed_nan_get_int(y)) {
  /* integers are equal */
}
```

The design deliberately avoids implicit conversions. A boxed integer is not a boxed double, and a boxed string is not a boxed symbol. The caller is expected to inspect the signature and act accordingly. This keeps the rules simple and prevents subtle type confusion.

The `boxed_nan` module is not meant to be used everywhere. It exists to support parts of the library where heterogeneous values are common and performance matters more than strict type separation. Used sparingly and intentionally, it allows coremio to remain simple without giving up flexibility.

---

## Chapter 11  -  `tokens`: turning text into structured values

The `tokens` module exists to do the first step of parsing in a way that stays honest: take raw text, walk it left to right, and produce a linear stream of tokens. Not a full parser, not a grammar engine, not an AST builder. Just the honest first step: break input into pieces you can later reason about.

In coremio, a token is a `double` (`typedef double t_token`) using the `boxed_nan` representation. This allows a token stream to contain heterogeneous values - words, symbols, numbers, booleans, and special markers - while remaining compact and cheap to move around. The tokenizer does not assign meaning; it assigns boundaries and categories.

### Exploding a buffer into tokens

Tokenization starts by exploding a buffer:

```c
coremio_result f_tokens_explode_buffer(
  const char* buffer,
  const char* symbols_characters_table,
  const char* word_symbols_characters_table,
  const char* ignorable_characters_table,
  size_t* line_accumulator,
  size_t* line_breaks_accumulator,
  size_t* character_accumulator,
  size_t* token_index,
  bool* last_token_incomplete,
  t_token** tokens
);
```

The function walks the buffer character by character and appends tokens to a dynamic coremio array. The word “explode” is intentional: the entire token stream is materialized in memory. No lazy iteration, no hidden state.

A typical usage for a full buffer looks like this:

```c
t_token *tokens = NULL;

size_t line = 0, line_breaks = 0, character = 0, token_index = 0;
bool last_token_incomplete = false;

coremio_result rc = f_tokens_explode_buffer(
  buffer,
  "{}[]:,",   /* symbols */
  NULL,       /* word-symbols */
  " \r\n\t",  /* ignorable */
  &line, &line_breaks, &character,
  &token_index, &last_token_incomplete,
  &tokens
);

if (rc != NOICE)
  return 1;
```

The resulting `tokens` pointer is a growable array. The number of tokens is obtained with `d_array_size(tokens)`. When finished, the entire token stream is released with:

```c
f_tokens_free(tokens);
```

This frees both the array and any heap-backed strings owned by the tokens.

### Exploding a stream into tokens

When the input comes from a file descriptor, `f_tokens_explode_stream` performs the same operation while managing chunked reads internally:

```c
coremio_result f_tokens_explode_stream(
  int stream,
  const char* symbols_characters_table,
  const char* word_symbols_characters_table,
  const char* ignorable_characters_table,
  t_token** tokens
);
```

Example:

```c
t_token *tokens = NULL;

coremio_result rc = f_tokens_explode_stream(
  fd,
  "{}[]:,",
  NULL,
  " \r\n\t",
  &tokens
);

if (rc != NOICE)
  return 1;

for (size_t i = 0; i < d_array_size(tokens); ++i)
  f_tokens_print_plain(1, tokens[i]);

f_tokens_free(tokens);
```

### The rule tables: ignorable, symbols, word-symbols

Tokenization behavior is entirely driven by three character tables.

**Ignorable characters** are skipped completely. They never appear in tokens and always terminate any token currently being built. Typical examples are spaces, tabs, and newlines. Newlines are also tracked internally and can be emitted as dedicated newline tokens when needed.

**Symbols characters** are emitted as standalone symbol tokens. These are characters that carry structural meaning, such as `{`, `}`, `[`, `]`, `:`, or `,`. A symbol is not a one-character word; it is a distinct token category, which simplifies later parsing.

**Word-symbol characters** are characters that would normally act as symbols, but are allowed to appear inside words. This is useful for DSLs or identifiers that legitimately contain characters like `.` or `-`. During word construction, characters in this table do not terminate the word even if they also appear in the symbols table.

This combination gives fine-grained control without introducing a separate rule language or callback system.

### Quoted strings and signed values

Quoted strings are recognized when encountering `'` or `"` characters, **only if those characters are not listed as symbols**. If they are listed as symbols, they will be emitted as symbol tokens and no string bootstrapping will occur.

Similarly, signed numeric values (`+42`, `-7`) are recognized only if `+` and `-` are not listed as symbols. If they are treated as symbols, they will break numeric parsing. This behavior is intentional: the tokenizer follows the tables you give it and does not guess intent.

### Reading and consuming tokens

Once exploded, tokens are consumed linearly. Inspection starts by checking the token category:

```c
t_token t = tokens[i];

if (d_token_is_symbol(t)) {
  char c = d_boxed_nan_get_symbol(t);
}
else if (d_token_is_string(t)) {
  const char* s = d_token_get_string(t);
}
else if (d_token_is_value(t)) {
  /* integer, double, boolean */
}
```

Strings are retrieved uniformly, regardless of whether they are embedded or heap-backed:

```c
const char* s = d_token_get_string(t);
```

Token comparisons are explicit. Comparing a token to a literal string is common:

```c
if (f_tokens_compare_string(t, "version")) {
  /* matched keyword */
}
```

Symbol comparison is done by value:

```c
if (d_token_is_given_symbol(t, '{')) {
  /* object begins */
}
```

Two tokens are equal only if their category and content match. A word token is never equal to a symbol token, even if the underlying character sequence is the same.

### What this module deliberately does not do

The `tokens` module does not assign grammar, precedence, or semantic meaning. It does not know about keywords, operators, or nesting rules. It produces a clean, explicit stream of tokens and leaves interpretation entirely to the consumer.

This separation is intentional. The tokenizer is meant to be boring, predictable, and reusable. Everything interesting happens after it.

---

## Chapter 12  -  `json`: parsing, navigating, and rewriting structured data

The JSON module exists to do something very pragmatic: take JSON text, unfold it into an in-memory tree, let you walk and mutate that tree with simple rules, then print it back out when you’re done. It does not aim to be streaming, incremental, or “zero-copy clever”. It is intentionally upfront: once you explode a JSON document, the whole thing is in RAM, and you can treat it like a living data structure.

This design is not a limitation; it is a choice. Most JSON files in real projects are configuration blobs, protocol descriptions, small datasets. They are meant to be loaded, inspected, tweaked, and saved. So coremio chooses clarity: it pays the parsing cost once, then keeps access cheap, explicit, and predictable.

### From text to tree: `f_json_explode_buffer` and `f_json_explode_stream`

You can explode JSON from a memory buffer:

```c
s_json json_tree;

coremio_result rc = f_json_explode_buffer(json_buffer, &json_tree);
if (rc != NOICE) {
  fprintf(stderr, "%s: %s\n", rc->name, rc->description);
  return 1;
}
```

Or directly from a file descriptor:

```c
s_json json_tree;

coremio_result rc = f_json_explode_stream(json_fd, &json_tree);
if (rc != NOICE)
  return 1;
```

Internally, the module tokenizes the input using the `tokens` engine configured with JSON-specific rules, then builds a tree of `s_json_node`. Every node represents exactly one JSON construct:

- `e_json_type_object` for objects
- `e_json_type_array` for arrays
- `e_json_type_value` for strings, numbers, booleans, and null
- `e_json_type_undefined` for placeholders created during incremental construction

The ownership model is straightforward: the `s_json` structure owns the entire tree and the token storage used to build it. When you are done with the document, everything is released in one step:

```c
f_json_free(&json_tree);
```

### The mental model: objects have labels, arrays have indexes

Access to JSON data is done through *paths*. A path is described by a compact format string with **no spaces**, where each character tells the engine how to descend the tree.

- `s` means “object key” (expects a `const char *`)
- `d` means “array index” (expects an integer)

So `"sd"` means: take an object entry, then take an array element.

Given this JSON:

```json
{
  "values": [10, 20, 30],
  "flags": { "enabled": true }
}
```

Accessing the third element of `values` looks like this:

```c
double third_value =
  f_json_get_value(&json_tree, NULL, "sd", "values", 2);
```

Reading a boolean from a nested object:

```c
bool enabled_flag =
  f_json_get_bool(&json_tree, NULL, "ss", "flags", "enabled");
```

The `starting_node` parameter allows relative navigation. Passing `NULL` means “start from the root”. Passing a valid `s_json_node *` means “start from here”.

Paths are strict. The number and type of arguments must match the format string exactly. This strictness keeps the API readable and avoids ambiguous access patterns.

### Reading values

The JSON module exposes type-specific getters. You call the one that matches what you expect to retrieve:

```c
double threshold_value =
  f_json_get_value(&json_tree, NULL, "s", "threshold");

char *name_string =
  f_json_get_char(&json_tree, NULL, "s", "name");

bool enabled_flag =
  f_json_get_bool(&json_tree, NULL, "ss", "flags", "enabled");
```

`f_json_get_char` returns a pointer to the string stored inside the JSON tree. This pointer remains valid as long as the JSON tree lives. If you need ownership, you explicitly duplicate it.

If a path does not exist or resolves to a node of the wrong type, the function returns a default value and reports the issue through the `result` mechanism. JSON access failures are not silent by design.

### Navigating nodes explicitly

Sometimes you want the node itself rather than its value. For that, the module exposes node-level accessors:

```c
s_json_node *flags_node =
  f_json_get_node(&json_tree, NULL, "s", "flags");
```

If you want missing nodes to be created automatically along the path, you use:

```c
s_json_node *enabled_node =
  f_json_get_node_or_create(&json_tree, NULL, "ss", "flags", "enabled");
```

Automatically created nodes start as empty containers. When you later assign a value to them, they are promoted to value nodes. This behavior makes it easy to construct JSON programmatically without manually instantiating intermediate objects.

### Writing values

Writing uses the same path language:

```c
f_json_set_value(&json_tree, 3.14, NULL, "s", "pi");

f_json_set_char(&json_tree, "coremio", NULL, "s", "name");

f_json_set_bool(&json_tree, true, NULL, "ss", "flags", "enabled");
```

If the target node exists and is an empty container, it is promoted to a value node. If the target node is a non-empty object or array, the operation fails with `SHIT_INVALID_PARAMETERS`. This prevents accidentally overwriting structured data with a scalar.

### Arrays: indexing, promotion, and appending

Array access uses the `d` specifier. Internally, array elements are stored as a list of child nodes.

```c
double first_value =
  f_json_get_value(&json_tree, NULL, "sd", "values", 0);
```

There is a small but important promotion rule: if a node exists as an empty object and you first access it using `d`, it will be promoted to an array. This allows you to build arrays incrementally without pre-declaring them.

Appending a new element follows a clear pattern: obtain the array node, compute its current length, then set the element at that index.

```c
s_json_node *array_node =
  f_json_get_node(&json_tree, NULL, "s", "values");

size_t array_length = 0;

if (array_node && array_node->type == e_json_type_array) {
  for (s_json_node *child_node =
         (s_json_node *)array_node->content.children.head;
       child_node;
       child_node = (s_json_node *)child_node->head.next) {
    ++array_length;
  }

  f_json_set_value(&json_tree, 999, array_node, "d", array_length);
}
```

The implementation does not auto-fill missing indexes. If you try to write index 10 on an empty array, indexes 0–9 are not implicitly created. This is intentional: coremio avoids inventing structure behind your back.

### Deleting entries

Nodes can be deleted using the same path mechanism:

```c
f_json_delete_node(&json_tree, NULL, "s", "temporary");

f_json_delete_node(&json_tree, NULL, "sd", "values", 2);
```

Deletion removes the node from its parent container and frees the entire subtree.

### Printing JSON back to text

Serialization is performed by printing the tree to a file descriptor:

```c
f_json_print_plain(STDOUT_FILENO, NULL, &json_tree);
```

You can also print a subtree by providing a starting node:

```c
s_json_node *flags_node =
  f_json_get_node(&json_tree, NULL, "s", "flags");

f_json_print_plain(STDOUT_FILENO, flags_node, &json_tree);
```

This approach keeps I/O explicit and avoids hidden allocations. If you want the JSON as a string, you redirect output to a pipe or memory-backed file descriptor.

---

## Chapter 13  -  `neural_networks`: small, single-threaded, educational

The neural network modules in coremio exist for one reason only: understanding. They are not meant to compete with machine learning frameworks, they do not try to be fast, scalable, or clever, and they make no attempt to hide the math behind abstractions. They are plain C implementations of a feedforward neural network and a recurrent neural network, written to explore how these models actually work and to support a handful of demonstrative projects developed under *The Barfing Fox* umbrella.

Both implementations are single-threaded, CPU-bound, and dependency-free. Memory ownership is explicit, state is visible, and nothing happens “in the background”. This makes them suitable for experimentation, learning, and small symbolic problems, but unsuitable for large datasets, high-dimensional numeric workloads, or production-grade training pipelines.

The feedforward neural network follows a discrete-input model. Instead of operating on dense vectors, it assumes inputs and outputs are *indexes* into a finite set of symbols. This makes it particularly well-suited for classification problems where inputs naturally map to identifiers: characters, tokens, labels, or small categorical domains. Training and inference operate on these indexes, and the network internally computes activations across layers to determine the most likely output symbol and its score.

The recurrent neural network builds on the same foundation but introduces temporal state. It processes sequences of symbol indexes and maintains internal memory across steps, allowing it to model order and context. Time is handled explicitly through a fixed number of frames, and state can be reset manually when starting a new sequence. The recurrent model also supports probabilistic generation, with temperature-based sampling, making it usable for small procedural or generative experiments.

Both models support dumping and loading their internal state to and from textual representations. This is not meant for interoperability with other tools, but for transparency: models can be inspected, versioned, and reloaded without opaque binary blobs.

These neural network modules are experimental by nature. They were written to learn the mechanics, not to optimize them away. They are intentionally simple, intentionally readable, and intentionally limited. For small projects, demos, and “let’s see what happens if…” experiments, they can be surprisingly fun. For anything else, they serve as a reminder of what modern frameworks are abstracting for you.

---

## Closing notes

coremio is a toolbox. It is opinionated, explicit, and intentionally low-level. If you find yourself copying pieces of it into other projects, that usually means it is doing its job.

---

## License

MIT.
