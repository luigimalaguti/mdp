#include "internal/shared.h"
#include "linc.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// ==================================================
// Internal Functions
// ==================================================

static int linc_check_module(struct linc_module *module, enum linc_level level) {
    if (module == NULL || level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }

    pthread_rwlock_rdlock(&module->lock);
    bool is_module_enabled = module->enabled == true;
    bool is_level_valid = level >= module->level;
    pthread_rwlock_unlock(&module->lock);

    if (!is_module_enabled || !is_level_valid) {
        return -1;
    }
    return 0;
}

// ==================================================
// Public Functions
// ==================================================

void linc_log(struct linc_module *module, enum linc_level level, const char *filename, uint32_t line, const char *func,
              const char *format, ...) {
    linc_init();
    if (module == NULL) {
        return;
    }
    int module_check = linc_check_module(module, level);
    if (module_check < 0) {
        return;
    }

    struct linc_metadata metadata = {
        .timestamp = linc_timestamp(),
        .level = level,
        .thread_id = (uintptr_t)pthread_self(),
        .module_name = module->name,
        .filename = filename,
        .line = line,
        .func = func,
    };

    if (format != NULL) {
        va_list args;
        va_start(args, format);
        vsnprintf(metadata.message, sizeof(metadata.message), format, args);
        va_end(args);
    }

    linc_ring_buffer_enqueue(&metadata);
}
