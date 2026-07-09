#include "utinc.h"

#include "internal.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ==================================================
// Global Variables
// ==================================================

static struct utinc utinc;
struct utinc_stats *utinc_stats;

// ==================================================
// Logic Functions
// ==================================================

#if defined(CLOCK_MONOTONIC_RAW)
#define CLOCK_GETTIME(ts) clock_gettime(CLOCK_MONOTONIC_RAW, ts)
#else
#define CLOCK_GETTIME(ts) clock_gettime(CLOCK_MONOTONIC, ts)
#endif

int64_t utinc_timestamp(void) {
    struct timespec ts;
    CLOCK_GETTIME(&ts);
    int64_t timestamp = (int64_t)ts.tv_sec * 1000000000L + (int64_t)ts.tv_nsec;
    return timestamp;
}

void utinc_before_all(void (*callback)(void)) {
    utinc.before_all_callback = callback;
}

void utinc_after_all(void (*callback)(void)) {
    utinc.after_all_callback = callback;
}

void utinc_before_each(void (*callback)(void)) {
    utinc.before_each_callback = callback;
}

void utinc_after_each(void (*callback)(void)) {
    utinc.after_each_callback = callback;
}

void utinc_test_init(void) {
    memset(&utinc, 0, sizeof(utinc));
    utinc_stats = &utinc.stats;
    utinc.start_times.file_time = utinc_timestamp();
}

int utinc_test_teardown(void) {
    utinc.stats.executions.file_time = utinc_timestamp() - utinc.start_times.file_time;
    int exit_error = utinc.stats.global_test_suites.failed > 0;
    if (exit_error) {
        return 1;
    }
    return 0;
}

void utinc_test_suite_start(void) {
    memset(&utinc.stats.current_test_cases, 0, sizeof(utinc.stats.current_test_cases));
    memset(&utinc.stats.current_asserts, 0, sizeof(utinc.stats.current_asserts));
    if (utinc.before_all_callback != NULL) {
        utinc.before_all_callback();
    }
    utinc.start_times.suite_time = utinc_timestamp();
}

void utinc_test_suite_end(void) {
    utinc.stats.executions.suite_time = utinc_timestamp() - utinc.start_times.suite_time;
    if (utinc.stats.current_test_cases.failed > 0) {
        utinc.stats.global_test_suites.failed++;
    } else {
        utinc.stats.global_test_suites.passed++;
    }
    if (utinc.after_all_callback != NULL) {
        utinc.after_all_callback();
    }
}

void utinc_test_case_start(void) {
    memset(&utinc.stats.current_asserts, 0, sizeof(utinc.stats.current_asserts));
    if (utinc.before_each_callback != NULL) {
        utinc.before_each_callback();
    }
    utinc.start_times.case_time = utinc_timestamp();
}

void utinc_test_case_end(void) {
    utinc.stats.executions.case_time = utinc_timestamp() - utinc.start_times.case_time;
    if (utinc.stats.current_asserts.failed > 0) {
        utinc.stats.current_test_cases.failed++;
        utinc.stats.global_test_cases.failed++;
    } else {
        utinc.stats.current_test_cases.passed++;
        utinc.stats.global_test_cases.passed++;
    }
    if (utinc.after_each_callback != NULL) {
        utinc.after_each_callback();
    }
}

bool utinc_test_assert(bool condition) {
    if (condition) {
        utinc.stats.current_asserts.passed++;
    } else {
        utinc.stats.current_asserts.failed++;
    }
    return condition;
}

bool utinc_test_assert_block(void) {
    if (UTINC_BLOCK_ON_ASSERTS == 1) {
        return utinc.stats.current_asserts.failed > 0;
    } else {
        return false;
    }
}

// ==================================================
// Printing Functions
// ==================================================

static void utinc_print_chars(const char character, int length, int newline) {
    for (int i = 0; i < length; i++) {
        printf("%c", character);
    }
    if (newline == 1) {
        printf("\n");
    }
}

void utinc_print_intro(const char *title, struct utinc_stats *stats) {
    (void)stats;
    printf("\n%s", UTINC_COLOR_BOLD);
    utinc_print_chars('#', UTINC_LINE_WIDTH, true);
    printf("🗃️  %s%s\n", title, UTINC_COLOR_RESET);
    utinc_print_chars('=', UTINC_LINE_WIDTH, true);
    printf("\n");
}

void utinc_print_outro(struct utinc_stats *stats) {
    utinc_print_chars('=', UTINC_LINE_WIDTH, true);
    printf("%s", UTINC_COLOR_BOLD);
    printf("📊 %s\n", "TEST FILE SUMMARY");
    printf("%s", UTINC_COLOR_RESET);
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);

    printf("Total Test Suites: %s%d%s\n", UTINC_COLOR_CYAN,
           stats->global_test_suites.failed + stats->global_test_suites.passed, UTINC_COLOR_RESET);
    if (stats->global_test_suites.passed > 0) {
        printf("    Passed: %s%d%s ✅\n", UTINC_COLOR_GREEN, stats->global_test_suites.passed, UTINC_COLOR_RESET);
    }
    if (stats->global_test_suites.failed > 0) {
        printf("    Failed: %s%d%s ❌\n", UTINC_COLOR_RED, stats->global_test_suites.failed, UTINC_COLOR_RESET);
    }
    printf(" Total Test Cases: %s%d%s\n", UTINC_COLOR_CYAN,
           stats->global_test_cases.failed + stats->global_test_cases.passed, UTINC_COLOR_RESET);
    if (stats->global_test_cases.passed > 0) {
        printf("    Passed: %s%d%s ✅\n", UTINC_COLOR_GREEN, stats->global_test_cases.passed, UTINC_COLOR_RESET);
    }
    if (stats->global_test_cases.failed > 0) {
        printf("    Failed: %s%d%s ❌\n", UTINC_COLOR_RED, stats->global_test_cases.failed, UTINC_COLOR_RESET);
    }

    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
    printf("%sCompleted in %.3f ms%s\n", UTINC_COLOR_GRAY, stats->executions.file_time / 1e6, UTINC_COLOR_RESET);

    utinc_print_chars('=', UTINC_LINE_WIDTH, true);
    if (stats->global_test_cases.failed == 0) {
        printf("🎉 %sRESULT: ALL TESTS PASSED%s\n", UTINC_COLOR_BOLD UTINC_COLOR_GREEN, UTINC_COLOR_RESET);
    } else {
        printf("💥 %sRESULT: %d TEST%s IN %d SUITE%s FAILED%s\n", UTINC_COLOR_BOLD UTINC_COLOR_RED,
               stats->global_test_cases.failed, stats->global_test_cases.failed == 1 ? "" : "S",
               stats->global_test_suites.failed, stats->global_test_suites.failed == 1 ? "" : "S", UTINC_COLOR_RESET);
    }
    printf("%s", UTINC_COLOR_BOLD);
    utinc_print_chars('#', UTINC_LINE_WIDTH, true);
    printf("%s\n", UTINC_COLOR_RESET);
}

void utinc_print_suite_intro(const char *description, struct utinc_stats *stats) {
    (void)stats;
    printf("🗂️  %s%s%s\n", UTINC_COLOR_BOLD, description, UTINC_COLOR_RESET);
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
}

void utinc_print_suite_outro(struct utinc_stats *stats) {
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
    printf("%s(%.3f ms)%s\n\n", UTINC_COLOR_GRAY, stats->executions.suite_time / 1e6, UTINC_COLOR_RESET);
}

void utinc_print_case_intro(const char *description, struct utinc_stats *stats) {
    (void)stats;
    printf("🧪 %s\n", description);
    printf("%s", UTINC_COLOR_DIM UTINC_COLOR_GRAY);
}

void utinc_print_case_outro(struct utinc_stats *stats) {
    if (stats->current_asserts.failed > 0) {
        printf("%s↳  ❌ %s[FAILED]%s\n", UTINC_COLOR_RESET, UTINC_COLOR_RED, UTINC_COLOR_RESET);
    } else {
        printf("%s↳  ✅ %s[PASSED]%s\n", UTINC_COLOR_RESET, UTINC_COLOR_GREEN, UTINC_COLOR_RESET);
    }
}

void utinc_print_assert(const char *filename, int line, const char *message) {
    printf("%s", UTINC_COLOR_RESET);
    printf("    ┌ %sAssertion failed:%s\n", UTINC_COLOR_RED, UTINC_COLOR_RESET);
    printf("    │ %sLocation:%s %s:%d\n", UTINC_COLOR_GRAY, UTINC_COLOR_RESET, filename, line);
    printf("    └ %sMessage:%s %s\n", UTINC_COLOR_GRAY, UTINC_COLOR_RESET, message);
    printf("%s", UTINC_COLOR_DIM UTINC_COLOR_GRAY);
}

void utinc_build_message(char *buffer, size_t length, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, length, format, args);
    va_end(args);
}
