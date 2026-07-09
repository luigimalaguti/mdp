#include "linc.h"
#include "utinc.h"

#include <stdint.h>
#include <string.h>

const char *title = "LINC core functions test\n";

char timestamp_buffer[24];
char metadata_buffer[816];
struct linc_metadata metadata;

DEFINE_CALLBACK(clean_timestamp, { memset(timestamp_buffer, 0, sizeof(timestamp_buffer)); })
DEFINE_CALLBACK(clean_metadata, {
    memset(&metadata, 0, sizeof(metadata));
    memset(metadata_buffer, 0, sizeof(metadata_buffer));
})

TEST_RUNNER(title, {
    TEST_SUITE("Level string tests", {
        TEST_CASE("Should print TRACE level string",
                  { ASSERT_STRING_EQUAL("TRACE", linc_level_string(LINC_LEVEL_TRACE), "Error to parse level"); });

        TEST_CASE("Should print DEBUG level string",
                  { ASSERT_STRING_EQUAL("DEBUG", linc_level_string(LINC_LEVEL_DEBUG), "Error to parse level"); });

        TEST_CASE("Should print INFO level string",
                  { ASSERT_STRING_EQUAL("INFO", linc_level_string(LINC_LEVEL_INFO), "Error to parse level"); });

        TEST_CASE("Should print WARN level string",
                  { ASSERT_STRING_EQUAL("WARN", linc_level_string(LINC_LEVEL_WARN), "Error to parse level"); });

        TEST_CASE("Should print ERROR level string",
                  { ASSERT_STRING_EQUAL("ERROR", linc_level_string(LINC_LEVEL_ERROR), "Error to parse level"); });

        TEST_CASE("Should print FATAL level string",
                  { ASSERT_STRING_EQUAL("FATAL", linc_level_string(LINC_LEVEL_FATAL), "Error to parse level"); });

        TEST_CASE("Should print UNKN level string", {
            ASSERT_STRING_EQUAL("UNKN", linc_level_string(-1), "Error to parse level");
            ASSERT_STRING_EQUAL("UNKN", linc_level_string(6), "Error to parse level");
        });
    });

    BEFORE_EACH(clean_timestamp);
    TEST_SUITE("Timestamp string tests", {
        TEST_CASE("Should prints unix epoch", {
            int result = -1;
            int64_t ns = 0;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.000", timestamp_buffer, "Error formatting epoch timestamp");
        });

        TEST_CASE("Should increments time from unix epoch", {
            int result = -1;
            int64_t ns = 0;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.000", timestamp_buffer, "Error formatting epoch timestamp");

            ns = 1000000;  // ms
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.001", timestamp_buffer, "Error formatting millisecond timestamp");

            ns = 10000000;  // cs
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.010", timestamp_buffer, "Error formatting centisecond timestamp");

            ns = 100000000;  // ds
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.100", timestamp_buffer, "Error formatting decisecond timestamp");

            ns = 999999999;  // 0.999 s
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.999", timestamp_buffer, "Error formatting second timestamp");

            ns = 1000000000;  // s
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:01.000", timestamp_buffer, "Error formatting second timestamp");

            ns = 1000000000LL * 60;  // m
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:01:00.000", timestamp_buffer, "Error formatting minute timestamp");

            ns = 1000000000LL * 60 * 60;  // h
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 01:00:00.000", timestamp_buffer, "Error formatting hour timestamp");
        });

        TEST_CASE("Should increments date from unix epoch", {
            int result = -1;

            int64_t ns = 0;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.000", timestamp_buffer, "Error formatting epoch timestamp");

            ns = 1000000000LL * 60 * 60 * 24;  // day
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-02 00:00:00.000", timestamp_buffer, "Error formatting day timestamp");

            ns = 1000000000LL * 60 * 60 * 24 * 31;  // mon
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-02-01 00:00:00.000", timestamp_buffer, "Error formatting month timestamp");

            ns = 1000000000LL * 60 * 60 * 24 * 365;  // year
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1971-01-01 00:00:00.000", timestamp_buffer, "Error formatting year timestamp");

            ns = 1000000000LL * 60 * 60 * 24 * (365 * 8 + 366 * 2);  // decade
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1980-01-01 00:00:00.000", timestamp_buffer, "Error formatting decade timestamp");

            ns = 1000000000LL * 60 * 60 * 24 * (365 * 75 + 366 * 25);  // century
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2070-01-01 00:00:00.000", timestamp_buffer, "Error formatting century timestamp");
        });

        TEST_CASE("Should works with current datetime", {
            int result = -1;
            int64_t ns = 1757500215000000000;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2025-09-10 10:30:15.000", timestamp_buffer, "Error formatting current timestamp");

            ns -= 1000000000LL /* s */ * 60 /* m */ * 60 /* h */ * 24;  // yesterday
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2025-09-09 10:30:15.000", timestamp_buffer, "Error formatting yesterday timestamp");

            ns -= 1000000000LL /* s */ * 60 /* m */ * 60 /* h */ * 24 /* day */ * 31;  // last month
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2025-08-09 10:30:15.000", timestamp_buffer, "Error formatting last month timestamp");

            ns -= 1000000000LL /* s */ * 60 /* m */ * 60 /* h */ * 24 /* day */ * 365;  // last year
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2024-08-09 10:30:15.000", timestamp_buffer, "Error formatting last year timestamp");

            ns = 1757500215000000000 + 1000000000LL /* s */ * 60 /* m */ * 60 /* h */ * 24;  // tomorrow
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2025-09-11 10:30:15.000", timestamp_buffer, "Error formatting tomorrow timestamp");

            ns += 1000000000LL /* s */ * 60 /* m */ * 60 /* h */ * 24 /* day */ * 31;  // next month
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2025-10-12 10:30:15.000", timestamp_buffer, "Error formatting next month timestamp");

            ns += 1000000000LL /* s */ * 60 /* m */ * 60 /* h */ * 24 /* day */ * 365;  // next year
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2026-10-12 10:30:15.000", timestamp_buffer, "Error formatting next year timestamp");
        });

        TEST_CASE("Should decrement datetime from unix epoch", {
            int result = -1;
            int64_t ns = 0;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.000", timestamp_buffer, "Error formatting epoch timestamp");

            ns = -1000000;  // ms
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 23:59:59.999", timestamp_buffer, "Error formatting millisecond timestamp");

            ns = -10000000;  // cs
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 23:59:59.990", timestamp_buffer, "Error formatting centisecond timestamp");

            ns = -100000000;  // ds
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 23:59:59.900", timestamp_buffer, "Error formatting decisecond timestamp");

            ns = -1000000000;  // s
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 23:59:59.000", timestamp_buffer, "Error formatting second timestamp");

            ns = -1000000000LL * 60;  // m
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 23:59:00.000", timestamp_buffer, "Error formatting minute timestamp");

            ns = -1000000000LL * 60 * 60;  // h
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 23:00:00.000", timestamp_buffer, "Error formatting hour timestamp");

            ns = -1000000000LL * 60 * 60 * 24;  // day
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-12-31 00:00:00.000", timestamp_buffer, "Error formatting day timestamp");

            ns = -1000000000LL * 60 * 60 * 24 * 32;  // mon
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1969-11-30 00:00:00.000", timestamp_buffer, "Error formatting month timestamp");

            ns = -1000000000LL * 60 * 60 * 24 * 366;  // year
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1968-12-31 00:00:00.000", timestamp_buffer, "Error formatting year timestamp");
        });

        TEST_CASE("Should works with wrong buffer and size", {
            int result = 0;
            int ns = 0;
            result = linc_timestamp_string(ns, NULL, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, -1, "Error in linc_timestamp_string result with NULL buffer");

            result = linc_timestamp_string(ns, timestamp_buffer, 0);
            ASSERT_EQUAL(result, -1, "Error in linc_timestamp_string result with zero size");
            ASSERT_STRING_EQUAL("", timestamp_buffer, "Error formatting with zero size");

            result = linc_timestamp_string(ns, timestamp_buffer, 1);
            ASSERT_EQUAL(result, -1, "Error in linc_timestamp_string result with size 1");
            ASSERT_STRING_EQUAL("", timestamp_buffer, "Error formatting with size 1");

            result = linc_timestamp_string(ns, timestamp_buffer, 10);
            ASSERT_EQUAL(result, -1, "Error in linc_timestamp_string result with size 10");
            ASSERT_STRING_EQUAL("1970-01-0", timestamp_buffer, "Error formatting with size 10");

            result = linc_timestamp_string(ns, timestamp_buffer, 30);
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result with size 30");
            ASSERT_STRING_EQUAL("1970-01-01 00:00:00.000", timestamp_buffer, "Error formatting with size 30");
        });

        TEST_CASE("Should works with max and min timestamps", {
            int result = -1;
            int64_t ns = INT64_MAX;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("2262-04-11 23:47:16.854", timestamp_buffer, "Error formatting max timestamp");

            ns = INT64_MIN;
            result = linc_timestamp_string(ns, timestamp_buffer, sizeof(timestamp_buffer));
            ASSERT_EQUAL(result, 0, "Error in linc_timestamp_string result");
            ASSERT_STRING_EQUAL("1677-09-21 00:12:43.145", timestamp_buffer, "Error formatting min timestamp");
        });
    });

    BEFORE_EACH(clean_metadata);
    TEST_SUITE("Metadata string tests", {
        TEST_CASE("Should works with empty metadata", {
            int result = linc_stringify_metadata(&metadata, metadata_buffer, sizeof(metadata_buffer), false);
            ASSERT_TRUE(result > 0, "Error in linc_stringify_metadata result without colors");
            ASSERT_STRING_EQUAL(
                "[ 1970-01-01 00:00:00.000 ] "
                "[ TRACE ] "
                "[ 0000000000000000 ] "
                "[ unknown          ] "
                "unknown:0 unknown: \n",
                metadata_buffer, "Error formatting empty metadata without colors");

            result = linc_stringify_metadata(&metadata, metadata_buffer, sizeof(metadata_buffer), true);
            ASSERT_TRUE(result > 0, "Error in linc_stringify_metadata result with colors");
            ASSERT_STRING_EQUAL(
                "[ \x1b[1m1970-01-01 00:00:00.000\x1b[0m ] "
                "[ \x1b[2m\x1b[94mTRACE\x1b[0m ] "
                "[ \x1b[1m0000000000000000\x1b[0m ] "
                "[ \x1b[1munknown         \x1b[0m ] "
                "\x1b[96munknown\x1b[0m:"
                "\x1b[93m0\x1b[0m "
                "\x1b[95munknown\x1b[0m: "
                "\n",
                metadata_buffer, "Error formatting empty metadata with colors");
        });

        TEST_CASE("Should works with full metadata", {
            metadata.timestamp = 1757500215000000000;
            metadata.level = LINC_LEVEL_WARN;
            metadata.thread_id = 0x0123456789;
            metadata.module_name = "module";
            metadata.filename = "test_file.c";
            metadata.line = 42;
            metadata.func = "test_function";
            strncpy(metadata.message, "This is a test message", 23);

            int result = linc_stringify_metadata(&metadata, metadata_buffer, sizeof(metadata_buffer), false);
            ASSERT_TRUE(result > 0, "Error in linc_stringify_metadata result without colors");
            ASSERT_STRING_EQUAL(
                "[ 2025-09-10 10:30:15.000 ] "
                "[ WARN  ] "
                "[ 0000000123456789 ] "
                "[ module           ] "
                "test_file.c:42 test_function: "
                "This is a test message\n",
                metadata_buffer, "Error formatting full metadata without colors");

            result = linc_stringify_metadata(&metadata, metadata_buffer, sizeof(metadata_buffer), true);
            ASSERT_TRUE(result > 0, "Error in linc_stringify_metadata result with colors");
            ASSERT_STRING_EQUAL(
                "[ \x1b[1m2025-09-10 10:30:15.000\x1b[0m ] "
                "[ \x1b[93mWARN \x1b[0m ] "
                "[ \x1b[1m0000000123456789\x1b[0m ] "
                "[ \x1b[1mmodule          \x1b[0m ] "
                "\x1b[96mtest_file.c\x1b[0m:"
                "\x1b[93m42\x1b[0m "
                "\x1b[95mtest_function\x1b[0m: "
                "This is a test message\n",
                metadata_buffer, "Error formatting full metadata with colors");
        });

        TEST_CASE("Should works with wrong buffer and size", {
            metadata.timestamp = 1757500215000000000;
            metadata.level = LINC_LEVEL_WARN;
            metadata.thread_id = 0x0123456789;
            metadata.module_name = "module";
            metadata.filename = "test_file.c";
            metadata.line = 42;
            metadata.func = "test_function";
            strncpy(metadata.message, "This is a test message", 23);

            int result = linc_stringify_metadata(NULL, metadata_buffer, sizeof(metadata_buffer), false);
            ASSERT_EQUAL(result, -1, "Error in linc_stringify_metadata result with NULL metadata");

            result = linc_stringify_metadata(&metadata, NULL, sizeof(metadata_buffer), false);
            ASSERT_EQUAL(result, -1, "Error in linc_stringify_metadata result with NULL buffer");

            result = linc_stringify_metadata(&metadata, metadata_buffer, 0, false);
            ASSERT_EQUAL(result, -1, "Error in linc_stringify_metadata result with zero size");
            ASSERT_STRING_EQUAL("", metadata_buffer, "Error formatting with zero size");

            result = linc_stringify_metadata(&metadata, metadata_buffer, 10, false);
            ASSERT_EQUAL(result, -1, "Error in linc_stringify_metadata result with size 10");
            ASSERT_STRING_EQUAL("[ 2025-09", metadata_buffer, "Error formatting with size 10");

            result = linc_stringify_metadata(&metadata, metadata_buffer, 1000, false);
            ASSERT_TRUE(result > 0, "Error in linc_stringify_metadata result with size 1000");
            ASSERT_STRING_EQUAL(
                "[ 2025-09-10 10:30:15.000 ] "
                "[ WARN  ] "
                "[ 0000000123456789 ] "
                "[ module           ] "
                "test_file.c:42 test_function: "
                "This is a test message\n",
                metadata_buffer, "Error formatting with size 50");
        });
    });
})
