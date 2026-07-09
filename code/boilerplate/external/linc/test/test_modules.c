#include "linc.h"
#include "utinc.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char *title = "LINC modules functions test\n";

struct in_memory {
    char logs[256][816];
    int count;
};
struct in_memory in_memory;

int sink_in_memory_open(void *data) {
    (void)data;
    return 0;
}

int sink_in_memory_close(void *data) {
    (void)data;
    return 0;
}

int sink_in_memory_write(void *data, struct linc_metadata *metadata) {
    struct in_memory *memory = (struct in_memory *)data;
    metadata->timestamp = 0;
    metadata->thread_id = 0;
    metadata->line = 0;
    char *log = memory->logs[memory->count % 256];
    if (linc_stringify_metadata(metadata, log, 816, false) < 0) {
        return -1;
    }
    memory->logs[memory->count % 256][815] = '\0';
    memory->count++;
    return 0;
}

int sink_in_memory_flush(void *data) {
    (void)data;
    return 0;
}

linc_module modules[LINC_DEFAULT_MAX_MODULES];

DEFINE_CALLBACK(in_memory_sink_init, {
    modules[0] = linc_default_module;
    linc_set_sink_enabled(linc_default_sink, false);
    struct linc_sink_funcs in_memory_funcs;
    in_memory_funcs.data = &in_memory;
    in_memory_funcs.open = sink_in_memory_open;
    in_memory_funcs.close = sink_in_memory_close;
    in_memory_funcs.write = sink_in_memory_write;
    in_memory_funcs.flush = sink_in_memory_flush;
    linc_register_sink("in_memory", LINC_LEVEL_TRACE, true, in_memory_funcs);
})

DEFINE_CALLBACK(in_memory_sink_clean, {
    linc_set_module_level(linc_default_module, LINC_LEVEL_INFO);
    memset(&in_memory, 0, sizeof(in_memory));
})

TEST_RUNNER(title, {
    BEFORE_ALL(in_memory_sink_init);
    BEFORE_EACH(in_memory_sink_clean);

    TEST_SUITE("Default module tests", {
        TEST_CASE("Should enable and disable default module", {
            int result = -1;

            INFO("Default module");
            INFO_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(2, in_memory.count, "Error count");
            ASSERT_STRING_EQUAL(
                "[ 1970-01-01 00:00:00.000 ] [ INFO  ] [ 0000000000000000 ] [ main             ] "
                "test/test_modules.c:0 utinc_test_runner: "
                "Default module\n",
                in_memory.logs[0], "Log 1");
            ASSERT_STRING_EQUAL(
                "[ 1970-01-01 00:00:00.000 ] [ INFO  ] [ 0000000000000000 ] [ main             ] "
                "test/test_modules.c:0 utinc_test_runner: "
                "Default module\n",
                in_memory.logs[1], "Log 2");

            result = linc_set_module_enabled(modules[0], false);
            INFO("Default module");
            INFO_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(0, result, "Error result");
            ASSERT_EQUAL(2, in_memory.count, "Error count");

            result = linc_set_module_enabled(modules[0], true);
            INFO("Default module");
            INFO_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(0, result, "Error result");
            ASSERT_EQUAL(4, in_memory.count, "Error count");
            ASSERT_STRING_EQUAL(
                "[ 1970-01-01 00:00:00.000 ] [ INFO  ] [ 0000000000000000 ] [ main             ] "
                "test/test_modules.c:0 utinc_test_runner: "
                "Default module\n",
                in_memory.logs[2], "Log 1");
            ASSERT_STRING_EQUAL(
                "[ 1970-01-01 00:00:00.000 ] [ INFO  ] [ 0000000000000000 ] [ main             ] "
                "test/test_modules.c:0 utinc_test_runner: "
                "Default module\n",
                in_memory.logs[3], "Log 2");
        });

        TEST_CASE("Should change level of default module", {
            int result = -1;

            DEBUG("Default module");
            DEBUG_M(modules[0], "Default module");
            ERROR("Default module");
            ERROR_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(2, in_memory.count, "Error count");

            result = linc_set_module_level(modules[0], LINC_LEVEL_DEBUG);
            DEBUG("Default module");
            DEBUG_M(modules[0], "Default module");
            ERROR("Default module");
            ERROR_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(0, result, "Error result");
            ASSERT_EQUAL(6, in_memory.count, "Error count");

            result = linc_set_module_level(modules[0], LINC_LEVEL_FATAL);
            DEBUG("Default module");
            DEBUG_M(modules[0], "Default module");
            ERROR("Default module");
            ERROR_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(0, result, "Error result");
            ASSERT_EQUAL(6, in_memory.count, "Error count");

            result = linc_set_module_level(modules[0], -1);
            DEBUG("Default module");
            DEBUG_M(modules[0], "Default module");
            ERROR("Default module");
            ERROR_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(-1, result, "Error result");
            ASSERT_EQUAL(6, in_memory.count, "Error count");

            result = linc_set_module_level(modules[0], 6);
            DEBUG("Default module");
            DEBUG_M(modules[0], "Default module");
            ERROR("Default module");
            ERROR_M(modules[0], "Default module");
            sleep(1);
            ASSERT_EQUAL(-1, result, "Error result");
            ASSERT_EQUAL(6, in_memory.count, "Error count");
        });
    });

    TEST_SUITE("New module tests", {
        TEST_CASE("Should register a new module", {
            modules[1] = linc_register_module("new_module", LINC_LEVEL_DEBUG, true);
            ASSERT_NOT_NULL(modules[1], "Error new module");

            TRACE_M(modules[1], "New module log");
            DEBUG_M(modules[1], "New module log");
            INFO_M(modules[1], "New module log");
            sleep(1);
            ASSERT_EQUAL(2, in_memory.count, "Error count");
        });

        TEST_CASE("Should not register a new module", {
            linc_module wrong_module = linc_register_module("new_module", LINC_LEVEL_INFO, true);
            ASSERT_NULL(wrong_module, "Error wrong module");

            linc_module name_module = linc_register_module("01234567890123456789", LINC_LEVEL_INFO, true);
            ASSERT_NULL(name_module, "Error wrong module");

            for (int i = 2; i < LINC_DEFAULT_MAX_MODULES; i++) {
                char name[LINC_DEFAULT_MODULE_NAME_LENGTH];
                snprintf(name, sizeof(name), "module_%d", i);
                modules[i] = linc_register_module(name, LINC_LEVEL_INFO, true);
                ASSERT_NOT_NULL(modules[i], "Error new module");
            }

            linc_module overflow_module = linc_register_module("overflow_module", LINC_LEVEL_INFO, true);
            ASSERT_NULL(overflow_module, "Error overflow module");
        });
    });
})
