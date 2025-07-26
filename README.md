# 🏟️ Arena allocator

When I develop toy compilers, I have to deal with data structures allocation and deallocation and it's really painful with C,
to make my life easier I choose arena allocators, because I don't have to deal with memory deallocation and I'm sure
that at some point in the program, all the allocated memory is released. This is the implementation that always I copy
and paste from a project to another.

>[!IMPORTANT]
>This library is written in **C99**.

# 🌱 How to Use It in Your Project
This library is designed for easy integration into your C projects. 
It follows the [stb-style](https://github.com/nothings/stb), which means you don't need 
to compile it separately or link against any `.lib` or `.so` files.

For more information about the stb-style, you can read this [guide](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).

## Basic Usage
To use the arena allocator in your project, simply include the `arena.h` header file. In one (and only one) of your `.c` source files, 
you must define `ARENA_IMPLEMENTATION` before including `arena.h`. 
This tells the preprocessor to include the function definitions in that specific compilation unit.

```c
// In one of your .c files (e.g., main.c)
#define ARENA_IMPLEMENTATION
#include "arena.h"

// Your application code...
```

## Advanced Usage: Separating Implementation
For larger projects, or to improve compilation times, you might prefer to put the implementation code 
in a dedicated `.c` file. This is a common practice with stb-style libraries.

**1. Create a separate implementation file (e.g., arena_impl.c):**
```c
// arena_impl.c
// This file will contain the actual function definitions for the arena allocator.
#define ARENA_IMPLEMENTATION
#include "arena.h"
```

**2. Include the header in other project files (e.g., my_project_file.c):**
```c
// my_project_file.c
// Other source files in your project can simply include the header.
#include <stdio.h> // Example standard library include
#include "arena.h" // Include the arena allocator header

// Your project's code that uses the arena allocator
// ...
```

# 🧪 Testing

At the moment I've implemented only the basic test cases. 
If other test cases come up in your mind, open an **issue**, and we will discuss it.

To execute the tests, open your terminal and run the following command:
```bash
make test
```

# 📔 API

## Constants

>[!NOTE]
> These constants aren't used anywhere in the implementation code,
> they're not the default value for nothing or anything like this, simply
> rappresents values that I usually use.

- `PAGE_SIZE`: defines the size of a 4KB page.
- `HUGE_PAGE_SIZE`: defines the size of a 16KB page.

## Data Structures

```c
typedef struct _arena_chunk {
    struct _arena_chunk* next;
    size_t size;
    size_t used;
    uint8_t data[]; // Flexible array member
} arena_chunk_t;
```

This structure represents a single block of memory within the arena.

- `next`: A pointer to the next `arena_chunk_t` in the linked list of chunks. This allows the arena to grow by adding more chunks as needed.
- `size`: The total size (in bytes) of the data buffer within this chunk.
- `used`: The amount of memory (in bytes) currently allocated from this chunk.
- `data[]`: A flexible array member ([FAM](https://en.wikipedia.org/wiki/Flexible_array_member)).
            This means that the actual memory for the chunk's data is allocated immediately after the `arena_chunk_t` structure itself.
            This allows for efficient memory usage without additional pointer indirection.

--- 
```c
typedef struct {
    arena_chunk_t* begin;
    arena_chunk_t* end;
} arena_t;
```

This structure represents the arena itself, managing the collection of arena_chunk_t blocks.

- `begin`: A pointer to the first `arena_chunk_t` in the linked list.
- `end`: A pointer to the last (current) `arena_chunk_t` in the linked list. New allocations are primarily attempted from this chunk.

## Functions

```c
  arena_t create_arena(size_t size);
```

Initializes a new memory arena. It allocates the first `arena_chunk_t` with the specified size.

**Parameters:**
 - `size`: The desired size for the memory chunks in the arena.

**Returns:** An `arena_t` structure representing the newly created and initialized arena.

---

```c
  void* arena_alloc(arena_t* restrict arena, size_t size);
```

Allocates a block of memory of `size` bytes from the specified arena.

It first attempts to allocate from the end chunk of the arena.
If the current chunk does not have enough contiguous free space, a new chunk is allocated (with the same size as the previous) 
and appended to the arena's linked list of chunks. The allocation then proceeds from this new chunk.

>[!NOTE]
> If the `ARENA_REDUCE_FRAGMENTATION` is defined, before allocating a new chunk,
> the arena linked list of chunks is scanned, trying to find a _"hole"_ big enough
> for current allocation. In this way we'll reduce the internal chunk fragmentation.

**Parameters:**
 - `arena`: A pointer to the `arena_t` structure from which to allocate memory.
 - `size`: The number of bytes to allocate.

**Returns:** A `void*` pointer to the newly allocated memory block, or `NULL` if `size` is less than or equal to zero. The returned memory is not initialized.

---

```c
  void* arena_realloc(arena_t* restrict arena, 
                      const void* restrict ptr, 
                      size_t old_size, 
                      size_t new_size);
```
Resizes a previously allocated memory block `ptr` within the arena to `new_size` bytes.

Unlike standard realloc, it will allocate a new block of `new_size` bytes using `arena_alloc`, copy the contents from the old `ptr` to the new block, 
and then effectively abandon the old block (as arena allocators typically don't support individual deallocations).

It's important to note that `arena_realloc` might be less efficient than realloc in a general-purpose allocator 
because it often involves copying data and cannot truly "free" the old space.

**Parameters:**
 - `arena`: A pointer to the `arena_t` structure.
 - `ptr`: A pointer to the memory block previously allocated by `arena_alloc`. If `ptr` is `NULL`, this function behaves like `arena_alloc`.
 - `old_size`: The original size of the memory block `ptr`. An incorrect `old_size` will result in undefined behavior.
 - `new_size`: The new desired size for the memory block.

**Returns:** A `void*` pointer to the new memory block, or `NULL` if the `size` is less or equal to zero.

---

```c
  char* arena_strdup(arena_t* restrict arena, const char* restrict str);
```

Duplicates a null-terminated string `str` into memory allocated from the arena. It allocates enough space for the string plus the null terminator.

**Parameters:**
 - `arena`: A pointer to the `arena_t` structure.
 - `str`: The null-terminated string to duplicate.

**Returns:** A `char*` pointer to the duplicated string within the arena.

---

```c
  char* arena_strndup(arena_t* restrict arena, const char* restrict str, size_t length);
```

Duplicates at most `length` characters from the string `str` into memory allocated from the arena. The duplicated string will be null-terminated.

**Parameters:**
- `arena`: A pointer to the `arena_t` structure.
- `str`: The source string.
- `length`: The maximum number of characters to duplicate from `str`.

**Returns:** A `char*` pointer to the duplicated string within the arena.

--- 

```c
  void destroy_arena(arena_t* restrict arena);
```

Frees all memory associated with the arena. This function iterates through all `arena_chunk_t` blocks in the linked list and deallocates them.
After this call, the `arena_t` structure and any pointers obtained from `arena_alloc` within this arena become invalid.

Parameters:
- `arena`: A pointer to the `arena_t` structure to destroy.

## Debugging Functions (ARENA_DEBUG_MODE)

These functions are available only when `ARENA_DEBUG_MODE` is defined, 
providing introspection into the arena's state for debugging and profiling purposes.

---

```c
  int arena_get_chunks_count(const arena_t* restrict arena);
```

Returns the total number of `arena_chunk_t` blocks currently allocated in the arena.

**Parameters:**
- `arena`: A pointer to the `arena_t` structure.

**Returns:** An `int` representing the count of chunks.

---
```c
  size_t arena_get_available_space_of(const arena_t* restrict arena, int chunk_index);
```
Returns the available (unused) space in a specific chunk identified by its `chunk_index`.

**Parameters:**
- `arena`: A pointer to the `arena_t` structure.
- `chunk_index`: The 0-based index of the chunk to query.

**Returns:** A `size_t` representing the available space in bytes for that chunk.

---
```c
  size_t arena_get_current_available_space(const arena_t* restrict arena);
```

Returns the available (unused) space in the current chunk of the arena.

**Parameters:**
- `arena`: A pointer to the `arena_t` structure.

**Returns:** A `size_t` representing the available space in bytes in the current chunk.

---
```c
  size_t arena_get_used_space_of(const arena_t* restrict arena, int chunk_index);
```

Returns the used space in a specific chunk identified by its `chunk_index`.

**Parameters:**
- `arena`: A pointer to the `arena_t` structure.
- `chunk_index`: The 0-based index of the chunk to query.

**Returns:** A `size_t` representing the used space in bytes for that chunk.

---
```c
  size_t arena_get_current_used_space(const arena_t* restrict arena);
```

Returns the used space in the current chunk of the arena.

**Parameters:**
- `arena`: A pointer to the `arena_t` structure.

**Returns:** A `size_t` representing the used space in bytes in the current chunk.

# ✨ Customization

You can integrate your custom `malloc()` and `free()` implementations with the library 
by defining the `ARENA_MALLOC` and `ARENA_FREE` macros. 
This allows the library to use your preferred memory management functions.

>[!NOTE]
> By default, `stdlib.h`'s malloc and free are used.

```c
#include "xmalloc.h" // Your custom memory allocation headers

#define ARENA_MALLOC xmalloc
#define ARENA_FREE xfree

#define ARENA_IMPLEMENTATION
#include "arena.h"

...
```

# 🧮 Usage Considerations

- **Lifetime Management:** Arena allocators are best suited for situations where many objects have the same lifetime and can be 
deallocated all at once by destroying the entire arena. They are not ideal for fine-grained, individual deallocations.

- **Fragmentation:** Internal fragmentation can occur if many small allocations are made, leaving small unused gaps within chunks that are too small for subsequent allocations.
To reduce internal fragmentation, the `ARENA_REDUCE_FRAGMENTATION` mode was introduced, which is responsible for finding free space to use within the arena chunks.
- **Performance:** Can offer significant performance benefits over malloc/free for frequent allocations, as it reduces system call overhead and improves cache locality.
- **Memory Overhead:** Each `arena_chunk_t` has a small overhead for its next, size, and used members.

# 🧩 Contributing
We welcome contributions! Please follow these steps:

1. Fork the repository.
2. Create a new branch (using this [convention](https://medium.com/@abhay.pixolo/naming-conventions-for-git-branches-a-cheatsheet-8549feca2534)).
3. Make your changes and commit them with descriptive messages.
4. Push your changes to your fork.
5. Create a pull request to the main repository.
