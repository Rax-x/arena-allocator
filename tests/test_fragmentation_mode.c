#include "test.h"

#define ARENA_DEBUG_MODE
#define ARENA_IMPLEMENTATION
#define ARENA_REDUCE_FRAGMENTATION
#include "../arena.h"

#include <string.h>

TEST_SUITE(reduce_fragmentation_mode) {

    TEST_CASE("Reduce fragmentation: interleaving big and small allocations") {

        arena_t arena = create_arena(PAGE_SIZE);

        const int big_array_length = PAGE_SIZE / sizeof(int);
        const int small_array_length = big_array_length / 2;

        const int big_array_size = sizeof(int) * big_array_length;
        const int small_array_size = sizeof(int) * small_array_length;

        int* array = (int*)arena_alloc(&arena, big_array_size);
        int* array1 = (int*)arena_alloc(&arena, small_array_size);
        int* array2 = (int*)arena_alloc(&arena, big_array_size);
        int* array3 = (int*)arena_alloc(&arena, small_array_size);

        memset(array, 0, big_array_size);
        memset(array1, 0, small_array_size);
        memset(array2, 0, big_array_size);
        memset(array3, 0, small_array_size);


        TEST_ASSERT(&array1[small_array_length] == array3, "Expected same memory address.");

        array1[small_array_length - 1] = 12;
        TEST_ASSERT(array1[small_array_length - 1] != *array3, "Expected different content.");


        // Without the 'ARENA_REDUCE_FRAGMENTATION' mode enabled, 
        // the chunks would be 4 instead of 3.
        TEST_ASSERT(arena_get_chunks_count(&arena) == 3, "Expected 3 chunks.");

        destroy_arena(&arena);
    }
}

int main(int argc, char** argv, test_context_t* context) {

    (void) argc;
    (void) argv;

    RUN_SUITE(reduce_fragmentation_mode, context);

    PRINT_WRAP_UP(context);

    return 0;
}
