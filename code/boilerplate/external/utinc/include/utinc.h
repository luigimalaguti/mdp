#ifndef UTINC_INCLUDE_UTINC_H
#define UTINC_INCLUDE_UTINC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// ==================================================
// Data Structures
// ==================================================

struct utinc_counts {
        int passed;
        int failed;
};

struct utinc_time {
        int64_t file_time;   // Total time for the entire test file, in microseconds
        int64_t suite_time;  // Total time for the current test suite, in microseconds
        int64_t case_time;   // Total time for the current test case, in microseconds
};

struct utinc_stats {
        struct utinc_counts
            global_test_suites;  // Counts of all test suites. Counts number of test suites passed or failed
        struct utinc_counts
            global_test_cases;  // Counts of all test cases. Counts number of test cases passed or failed

        struct utinc_counts current_test_cases;  // Counts of test cases in the current suite. Counts number of test
                                                 // cases passed or failed
        struct utinc_counts
            current_asserts;  // Counts of asserts in the current test case. Counts number of asserts passed or failed

        struct utinc_time executions;  // Time tracking for file, suite, and case
};

extern struct utinc_stats *utinc_stats;  // Pointer to the global stats structure

// ==================================================
// Macros
// ==================================================

#if defined(__GNUC__)
#define UTINC_PRINT_FMT(fmt, args) \
    __attribute__((format(printf, fmt, args)))  // GCC format attribute for printf-style functions
#else
#define UTINC_PRINT_FMT(fmt, args)
#endif

#if !defined(UTINC_BLOCK_ON_ASSERTS)
#define UTINC_BLOCK_ON_ASSERTS 0  // Default behavior is to not block on failed assertions
#elif (UTINC_BLOCK_ON_ASSERTS != 0 && UTINC_BLOCK_ON_ASSERTS != 1)
#error "UTINC_BLOCK_ON_ASSERTS must be defined as 0 or 1"
#endif

// ==================================================
// Functions
// ==================================================

int64_t utinc_timestamp(void);

void utinc_before_all(void (*callback)(void));
void utinc_after_all(void (*callback)(void));
void utinc_before_each(void (*callback)(void));
void utinc_after_each(void (*callback)(void));

void utinc_test_init(void);
void utinc_test_runner(void);
int utinc_test_teardown(void);

void utinc_test_suite_start(void);
void utinc_test_suite_end(void);

void utinc_test_case_start(void);
void utinc_test_case_end(void);

bool utinc_test_assert(bool condition);
bool utinc_test_assert_block(void);

void utinc_print_intro(const char *title, struct utinc_stats *stats);
void utinc_print_outro(struct utinc_stats *stats);

void utinc_print_suite_intro(const char *description, struct utinc_stats *stats);
void utinc_print_suite_outro(struct utinc_stats *stats);

void utinc_print_case_intro(const char *description, struct utinc_stats *stats);
void utinc_print_case_outro(struct utinc_stats *stats);

void utinc_print_assert(const char *filename, int line, const char *message);
void utinc_build_message(char *buffer, size_t length, const char *format, ...) UTINC_PRINT_FMT(3, 4);

// ==================================================
// Testing Macros
// ==================================================

#define DEFINE_CALLBACK(name, code) void name(void) code

#define BEFORE_ALL(callback) utinc_before_all(callback)
#define AFTER_ALL(callback) utinc_after_all(callback)

#define BEFORE_EACH(callback) utinc_before_each(callback)
#define AFTER_EACH(callback) utinc_after_each(callback)

#define TEST_RUNNER(title, code)               \
    int main(void) {                           \
        utinc_test_init();                     \
        utinc_print_intro(title, utinc_stats); \
        utinc_test_runner();                   \
        int status = utinc_test_teardown();    \
        utinc_print_outro(utinc_stats);        \
        return status;                         \
    }                                          \
    DEFINE_CALLBACK(utinc_test_runner, code)

#define TEST_SUITE(description, code)                      \
    do {                                                   \
        utinc_test_suite_start();                          \
        utinc_print_suite_intro(description, utinc_stats); \
        code utinc_test_suite_end();                       \
        utinc_print_suite_outro(utinc_stats);              \
    } while (0)

#define TEST_CASE(description, code)                      \
    do {                                                  \
        utinc_test_case_start();                          \
        utinc_print_case_intro(description, utinc_stats); \
        code utinc_test_case_end();                       \
        utinc_print_case_outro(utinc_stats);              \
    } while (0)

// ==================================================
// Assertions Macros
// ==================================================

#define ASSERT_TRUE(condition, message)                      \
    do {                                                     \
        if (utinc_test_assert_block()) break;                \
        if (!utinc_test_assert((condition) == true)) {       \
            utinc_print_assert(__FILE__, __LINE__, message); \
        }                                                    \
    } while (0)

#define ASSERT_FALSE(condition, message)                     \
    do {                                                     \
        if (utinc_test_assert_block()) break;                \
        if (!utinc_test_assert((condition) == false)) {      \
            utinc_print_assert(__FILE__, __LINE__, message); \
        }                                                    \
    } while (0)

#define ASSERT_EQUAL(expected, actual, message)                                                               \
    do {                                                                                                      \
        if (utinc_test_assert_block()) break;                                                                 \
        if (!utinc_test_assert((expected) == (actual))) {                                                     \
            char fail_message[256];                                                                           \
            utinc_build_message(fail_message, sizeof(fail_message), "%s (expected: %d, actual: %d)", message, \
                                (int)(expected), (int)(actual));                                              \
            utinc_print_assert(__FILE__, __LINE__, fail_message);                                             \
        }                                                                                                     \
    } while (0)

#define ASSERT_NOT_EQUAL(expected, actual, message)                                                           \
    do {                                                                                                      \
        if (utinc_test_assert_block()) break;                                                                 \
        if (!utinc_test_assert((expected) != (actual))) {                                                     \
            char fail_message[256];                                                                           \
            utinc_build_message(fail_message, sizeof(fail_message), "%s (expected: %d, actual: %d)", message, \
                                (int)(expected), (int)(actual));                                              \
            utinc_print_assert(__FILE__, __LINE__, fail_message);                                             \
        }                                                                                                     \
    } while (0)

#define ASSERT_NULL(pointer, message)                        \
    do {                                                     \
        if (utinc_test_assert_block()) break;                \
        if (!utinc_test_assert((pointer) == NULL)) {         \
            utinc_print_assert(__FILE__, __LINE__, message); \
        }                                                    \
    } while (0)

#define ASSERT_NOT_NULL(pointer, message)                    \
    do {                                                     \
        if (utinc_test_assert_block()) break;                \
        if (!utinc_test_assert((pointer) != NULL)) {         \
            utinc_print_assert(__FILE__, __LINE__, message); \
        }                                                    \
    } while (0)

#define ASSERT_STRING_EQUAL(expected, actual, message)                                                            \
    do {                                                                                                          \
        if (utinc_test_assert_block()) break;                                                                     \
        if (!utinc_test_assert(strcmp((expected), (actual)) == 0)) {                                              \
            char fail_message[256];                                                                               \
            utinc_build_message(fail_message, sizeof(fail_message), "%s (expected: '%s', actual: '%s')", message, \
                                (expected), (actual));                                                            \
            utinc_print_assert(__FILE__, __LINE__, fail_message);                                                 \
        }                                                                                                         \
    } while (0)

#endif  // UTINC_INCLUDE_UTINC_H
