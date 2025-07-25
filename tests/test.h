#ifndef _TEST_H_
#define _TEST_H_

#include <stdio.h>

typedef struct {
    int test_cases_count;
    int test_cases_failed;
} test_context_t;

#define TEST_SUITE(suite_name) void TestSuite__ ## suite_name(test_context_t* __tcontext)

#define RUN_SUITE(suite_name, context)                                  \
    do {                                                                \
        printf("\033[1;32m===== %s =====\n\033[0m", #suite_name);       \
        TestSuite__ ## suite_name(context);                             \
        putchar('\n');                                                  \
    } while(0)

#define TEST_CASE(description)                                          \
    for(int __func__ ## flag = (printf("\033[0;32mRunning \'%s\' ... ", description), \
                                ++__tcontext->test_cases_count);        \
        __func__ ## flag;                                               \
        __func__ ## flag = 0, printf("\033[1;32mPASS\n\033[0m"))

#define TEST_ASSERT(expression, ...)                                    \
    if(!(expression)) {                                                 \
        fprintf(stdout, "\n\033[1;31m[%s, Ln: %d] Assertion failed: %s\n\t", \
                __func__, __LINE__, #expression);                       \
        fprintf(stdout, "Message: " __VA_ARGS__);                       \
        fprintf(stdout, "\n\033[0m");                                   \
        __tcontext->test_cases_failed++;                                \
        break;                                                          \
    }                                                                   \

#define PRINT_WRAP_UP(context)                                          \
    printf("\n[Tests: \033[1;34m%d\033[0m, "                            \
           "Passed: \033[1;32m%d\033[0m, "                              \
           "Failed: \033[1;31m%d\033[0m]\n\n",                          \
           (context)->test_cases_count,                                 \
           (context)->test_cases_count - (context)->test_cases_failed,  \
           (context)->test_cases_failed)


#define main(...)                                       \
    __test_main(__VA_ARGS__);                           \
    int main(int argc, char** argv) {                   \
                                                        \
        test_context_t context = (test_context_t){      \
            .test_cases_count = 0,                      \
            .test_cases_failed = 0,                     \
        };                                              \
                                                        \
        setvbuf(stdin, NULL, _IONBF, 0);                \
        return __test_main(argc, argv, &context);       \
    }                                                   \
    int __test_main(__VA_ARGS__) 

#endif
