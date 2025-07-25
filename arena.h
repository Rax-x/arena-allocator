#ifndef _ARENA_H_
#define _ARENA_H_

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE (1 << 12)
#define HUGE_PAGE_SIZE (1 << 14)

typedef struct _arena_chunk {

    struct _arena_chunk* next;
    
    size_t size;
    size_t used;

    uint8_t data[];
} arena_chunk_t;

typedef struct {
    arena_chunk_t* begin;
    arena_chunk_t* end;
} arena_t;

arena_t create_arena(size_t size);

void* arena_alloc(arena_t* restrict arena, size_t size);
void* arena_realloc(arena_t* restrict arena, void* ptr, size_t size);

char* arena_strdup(arena_t* restrict arena, 
                   const char* restrict str);

char* arena_strndup(arena_t* restrict arena, 
                    const char* restrict str, 
                    size_t length);

void destroy_arena(arena_t* restrict arena);

#ifdef ARENA_DEBUG_MODE

int arena_get_chunks_count(const arena_t* restrict arena);

size_t arena_get_available_space_of(const arena_t* restrict arena, int chunk_index);
size_t arena_get_current_available_space(const arena_t* restrict arena);

size_t arena_get_used_space_of(const arena_t* restrict arena, int chunk_index);
size_t arena_get_current_used_space(const arena_t* restrict arena);


#endif

#endif

#ifdef ARENA_IMPLEMENTATION

#ifndef ARENA_MALLOC
#include <stdlib.h>
#define ARENA_MALLOC malloc
#endif

#ifndef ARENA_FREE
#include <stdlib.h>
#define ARENA_FREE free
#endif

#ifndef ARENA_ASSERT
#include <assert.h>
#define ARENA_ASSERT(condition, message) assert((condition) && (message))
#endif

#include <string.h>

static arena_chunk_t* new_arena_chunk(size_t size) {

    arena_chunk_t* const chunk = ARENA_MALLOC(sizeof(arena_chunk_t) + size);
    ARENA_ASSERT(chunk != NULL, "Unable to allocate memory!");

    chunk->used = 0;
    chunk->size = size;
    chunk->next = NULL;

    return chunk;
}

arena_t create_arena(size_t size) {
    arena_t arena;

    arena.begin = new_arena_chunk(size);
    arena.end = arena.begin;

    return arena;
}

void* arena_alloc(arena_t* restrict arena, size_t size) {

    if(size <= 0) return NULL;

#ifdef ARENA_REDUCE_FRAGMENTATION

    /*
      With this, we'll reduce internal fragmentation by searching 
      for "holes" not big enough for previous allocations but large 
      enough for the current one.
    */
    
    arena_chunk_t* current = arena->begin;

    while(current->next != NULL) {
        if(current->used + size <= current->size) {
            break;
        }

        current = current->next;
    }
#else

    arena_chunk_t* current = arena->end;

#endif

    if(current->used + size > current->size) {
        current->next = new_arena_chunk(current->size);
        arena->end = current->next;
        current = arena->end;
    }

    void* ptr = current->data + current->used;
    current->used += size;

    return ptr;
}

void* arena_realloc(arena_t* restrict arena, void* ptr, size_t size) {
    
    if(size <= 0) {
        return NULL;
    }

    void* new_ptr = arena_alloc(arena, size);

    if(ptr != NULL) {
        memcpy(new_ptr, ptr, size);
    }

    return new_ptr;
}

inline char* arena_strdup(arena_t* restrict arena, 
                          const char* restrict str) {
    return arena_strndup(arena, str, strlen(str));
}

char* arena_strndup(arena_t* restrict arena, 
                    const char* restrict str, 
                    size_t length) {

    char* const new_str = (char*)arena_alloc(arena, sizeof(char) * length + 1);

    memcpy(new_str, str, sizeof(char) * length);
    new_str[length] = '\0';

    return new_str;
}

void destroy_arena(arena_t* restrict arena) {

    arena_chunk_t* chunk;
    arena_chunk_t* it = arena->begin;

    while(it != NULL){
        chunk = it;
        it = it->next;

        ARENA_FREE(chunk);
    }
}

#ifdef ARENA_DEBUG_MODE

int arena_get_chunks_count(const arena_t* restrict arena) {
    int count = 0;

    for(const arena_chunk_t* it = arena->begin; 
        it != NULL; 
        it = it->next) {

        count++;
    }

    return count;
}

size_t arena_get_available_space_of(const arena_t* restrict arena, int chunk_index) {

    if(chunk_index > arena_get_chunks_count(arena) || chunk_index < 0) {
        return 0;
    }

    const arena_chunk_t* it = arena->begin;

    for(int i = 0; i < chunk_index; i++) {
        it = it->next;
    }

    return (it->size - it->used);
}

inline size_t arena_get_current_available_space(const arena_t* restrict arena) {
    return (arena->end->size - arena->end->used);
}

size_t arena_get_used_space_of(const arena_t* restrict arena, int chunk_index) {

    if(chunk_index > arena_get_chunks_count(arena) || chunk_index < 0) {
        return 0;
    }

    const arena_chunk_t* it = arena->begin;

    for(int i = 0; i < chunk_index; i++) {
        it = it->next;
    }

    return it->used;
}

inline size_t arena_get_current_used_space(const arena_t* restrict arena) {
    return arena->end->used;
}


#endif

#endif
