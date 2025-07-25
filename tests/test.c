#include "test.h"

#include <stdlib.h>
#include <string.h>

#define ARENA_DEBUG_MODE
#define ARENA_IMPLEMENTATION
#include "../arena.h"

TEST_SUITE(arena_alloc) {
    
    TEST_CASE("Allocating 0 bytes") {

        arena_t arena = create_arena(10);

        void* ptr = arena_alloc(&arena, 0);
        TEST_ASSERT(ptr == NULL, "Expected a NULL pointer.");

        destroy_arena(&arena);
    }

    TEST_CASE("Allocating variables") {
        
        arena_t arena = create_arena(1024);

        int* integer = (int*)arena_alloc(&arena, sizeof(int));
        double* decimal = (double*)arena_alloc(&arena, sizeof(double));

        *integer = 10;
        *decimal = 124.1;

        const size_t expected_used_space = sizeof(*integer) + sizeof(*decimal);
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(arena_get_current_available_space(&arena) == 1024 - used_space, 
                    "Expected same value.");

        destroy_arena(&arena);
    }

    TEST_CASE("Allocating arrays") {
        
        arena_t arena = create_arena(1024);

        const int array_size = 120;

        int* integers = (int*)arena_alloc(&arena, sizeof(int) * array_size);
        double* decimals = (double*)arena_alloc(&arena, sizeof(double) * array_size);

        for(int i = 0; i < array_size; i++) {
            integers[i] = rand() % RAND_MAX;
            decimals[i] = rand() % RAND_MAX + 0.1;
        }

        const size_t integer_array_size = sizeof(*integers) * array_size;
        const size_t double_array_size = sizeof(*decimals) * array_size;

        const size_t expected_used_space = integer_array_size + double_array_size;
        const size_t used_space = 
            arena_get_current_used_space(&arena) + // second chunk (current)
            arena_get_used_space_of(&arena, 0); // first chunk

        TEST_ASSERT(arena_get_chunks_count(&arena) == 2, "Expected 2 chunks.");

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(arena_get_current_available_space(&arena) == 1024 - double_array_size,
                    "Expected same value.");

        TEST_ASSERT(arena_get_available_space_of(&arena, 0) == 1024 - integer_array_size,
                    "Expected same value.");

        destroy_arena(&arena);
    }

    TEST_CASE("Allocating strings") {

        arena_t arena = create_arena(1024);

        const int param_count = 10;
        const int max_url_param_length = 13;

        const char* base_url = "http://fake-api.org";
        
        const int full_url_max_length = strlen(base_url) + param_count * max_url_param_length + 1 + param_count;
        char* const full_url = (char*)arena_alloc(&arena, full_url_max_length);

        strcpy(full_url, base_url);
        strcat(full_url, "?");

        char* const param_buffer = (char*)arena_alloc(&arena, max_url_param_length);

        for(int i = 0; i < param_count; i++) {
            const int length = sprintf(param_buffer, "param%d=value%d", i, i);
            param_buffer[length] = '\0';

            strcat(full_url, param_buffer);
            
            if(i != param_count - 1) {
                strcat(full_url, "&");
            }
        }

        const char* expected_url = "http://fake-api.org?param0=value0&"
            "param1=value1&param2=value2&param3=value3&param4=value4&"
            "param5=value5&param6=value6&param7=value7&param8=value8&"
            "param9=value9";

        const size_t expected_used_space = max_url_param_length + full_url_max_length;
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(arena_get_chunks_count(&arena) == 1, "Expected one chunks.");

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(arena_get_current_available_space(&arena) == 1024 - used_space,
                    "Expected same value");

        TEST_ASSERT(strcmp(full_url, expected_url) == 0, 
                    "Expected '%s' but got '%s'", expected_url, full_url);

        TEST_ASSERT(strlen(full_url) == strlen(expected_url), "Expected same string lengths.");

        destroy_arena(&arena);
    }
}

TEST_SUITE(arena_realloc) {

    TEST_CASE("Realloc array: new size greater") {

        arena_t arena = create_arena(1024);

        const int initial_array_size = 100;

        int* integers = (int*)arena_alloc(&arena, sizeof(int) * initial_array_size);

        for(int i = 0; i < initial_array_size; i++) {
            integers[i] = rand() % RAND_MAX;
        }

        const int new_array_size = initial_array_size + 12;
        int* integers2 = (int*)arena_realloc(&arena, integers, sizeof(int) * new_array_size);

        const size_t expected_used_space = sizeof(int) * (initial_array_size + new_array_size);
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(memcmp(integers, integers2, initial_array_size) == 0, "Expected same content.");
        
        destroy_arena(&arena);
    }

    TEST_CASE("Realloc array: new size smaller") {

        arena_t arena = create_arena(1024);

        const int initial_array_size = 100;

        int* integers = (int*)arena_alloc(&arena, sizeof(int) * initial_array_size);

        for(int i = 0; i < initial_array_size; i++) {
            integers[i] = rand() % RAND_MAX;
        }

        const int new_array_size = initial_array_size - 12;
        int* integers2 = (int*)arena_realloc(&arena, integers, sizeof(int) * new_array_size);

        const size_t expected_used_space = sizeof(int) * (initial_array_size + new_array_size);
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(memcmp(integers2, integers, new_array_size) == 0, "Expected same content.");
        
        destroy_arena(&arena);
    }

    TEST_CASE("Realloc array: new size zero") {

        arena_t arena = create_arena(1024);

        const int initial_array_size = 100;

        int* integers = (int*)arena_alloc(&arena, sizeof(int) * initial_array_size);

        for(int i = 0; i < initial_array_size; i++) {
            integers[i] = rand() % RAND_MAX;
        }

        const int new_array_size = 0;
        int* integers2 = (int*)arena_realloc(&arena, integers, sizeof(int) * new_array_size);

        const size_t expected_used_space = sizeof(int) * (initial_array_size + new_array_size);
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(integers2 == NULL, "Expected NULL pointer.");
        
        destroy_arena(&arena);
    }

    TEST_CASE("Realloc array: NULL origin") {

        arena_t arena = create_arena(1024);

        const int new_array_size = 10;
        int* integers = (int*)arena_realloc(&arena, NULL, sizeof(int) * new_array_size);

        const size_t expected_used_space = sizeof(int) * new_array_size;
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);

        TEST_ASSERT(integers != NULL, "Expected non-NULL pointer.");
        
        destroy_arena(&arena);
    }
}

TEST_SUITE(arena_strdup_and_strndup) {

    TEST_CASE("strdup: empty string") { 

        arena_t arena = create_arena(8);

        char* string = arena_strdup(&arena, "");

        TEST_ASSERT(string != NULL, "Expected a non-NULL string.");
        TEST_ASSERT(arena_get_current_used_space(&arena) == 1, 
                    "Expected 1 byte for null-terminator.");

        destroy_arena(&arena);
    }
    
    TEST_CASE("strdup: c-string") { 

        arena_t arena = create_arena(32);

        const char* original_string = "Hello World";
        char* string = arena_strdup(&arena, "Hello World");

        const size_t expected_used_space = strlen(original_string) + 1;
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);
        

        TEST_ASSERT(string != NULL, "Expected a non-NULL string.");
        TEST_ASSERT(strcmp(original_string, string) == 0, "Expected same content.");

        destroy_arena(&arena);
    }
    
    TEST_CASE("strndup: empty string") { 

        arena_t arena = create_arena(8);
        char* string = arena_strndup(&arena, "", 0);

        TEST_ASSERT(string != NULL, "Expected a non-NULL string.");
        TEST_ASSERT(arena_get_current_used_space(&arena) == 1, 
                    "Expected 1 byte for null-terminator.");

        destroy_arena(&arena);
    }
    
    TEST_CASE("strndup: substring") { 

        arena_t arena = create_arena(256);

        const char* url = "http://fake-url.com/route";

        const int domain_length = 12;
        const char* expected_domain = "fake-url.com";
        const char* domain = arena_strndup(&arena, url + 7, domain_length);

        const size_t expected_used_space = domain_length + 1;
        const size_t used_space = arena_get_current_used_space(&arena);

        TEST_ASSERT(used_space == expected_used_space, 
                    "Expected %lu bytes used, but got %lu bytes.", 
                    expected_used_space, used_space);
        

        TEST_ASSERT(domain != NULL, "Expected a non-NULL string.");
        TEST_ASSERT(strcmp(domain, expected_domain) == 0, "Expected same content.");

        destroy_arena(&arena);
    }
}

int main(int argc, char** argv, test_context_t* context) {

    (void) argc;
    (void) argv;

    RUN_SUITE(arena_alloc, context);
    RUN_SUITE(arena_realloc, context);
    RUN_SUITE(arena_strdup_and_strndup, context);

    PRINT_WRAP_UP(context);

    return 0;
}
