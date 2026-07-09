#include "internal/shared.h"
#include "linc.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// ==================================================
// Default Sink Functions
// ==================================================

static int linc_sink_stderr_open(void *data) {
    (void)data;
    return 0;
}

static int linc_sink_stderr_close(void *data) {
    (void)data;
    return 0;
}

static int linc_sink_stderr_write(void *data, struct linc_metadata *metadata) {
    FILE *output_file = (FILE *)data;

    if (output_file == NULL) {
        return -1;
    }
    int fd = fileno(output_file);
    if (fd < 0) {
        return -1;
    }
    bool use_colors = isatty(fd) == 1;

    char formatted_log[LINC_LOG_MAX_LENGTH + LINC_NEWLINE_CHAR_LENGTH + LINC_ZERO_CHAR_LENGTH];
    int written = linc_stringify_metadata(metadata, formatted_log, sizeof(formatted_log), use_colors);

    if (written < 0) {
        strcpy(formatted_log, "[ LINC ERROR ] Internal logging error\n");
        written = strlen(formatted_log);
    } else if ((size_t)written >= sizeof(formatted_log)) {
        written = sizeof(formatted_log) - 1;
    }

    size_t write_size = fwrite(formatted_log, sizeof(char), written, output_file);
    return write_size == (size_t)written ? 0 : -1;
}

static int linc_sink_stderr_flush(void *data) {
    FILE *output_file = (FILE *)data;
    return fflush(output_file);
}

// ==================================================
// Internal Functions
// ==================================================

static int linc_check_name_sink(struct linc_sink_list *sinks, const char *name) {
    if (name == NULL) {
        return -1;
    }

    size_t name_length = strnlen(name, LINC_DEFAULT_SINK_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH);
    if (name_length == 0 || name_length > LINC_DEFAULT_SINK_NAME_LENGTH) {
        return -1;
    }

    for (size_t i = 0; i < sinks->count; i++) {
        if (strncmp(sinks->list[i].name, name, LINC_DEFAULT_SINK_NAME_LENGTH) == 0) {
            return -1;
        }
    }

    return 0;
}

static int linc_check_level_sink(enum linc_level level) {
    if (level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }
    return 0;
}

static int linc_check_funcs_sink(struct linc_sink_funcs funcs) {
    if (funcs.open == NULL || funcs.close == NULL || funcs.write == NULL || funcs.flush == NULL) {
        return -1;
    }
    return 0;
}

static struct linc_sink *linc_add_sink(struct linc_sink_list *sinks, const char *name, enum linc_level level,
                                       bool enabled, struct linc_sink_funcs funcs) {
    bool is_failed = false;
    is_failed |= sinks == NULL;
    is_failed |= sinks->count >= LINC_DEFAULT_MAX_SINKS;
    is_failed |= linc_check_name_sink(sinks, name) < 0;
    is_failed |= linc_check_level_sink(level) < 0;
    is_failed |= linc_check_funcs_sink(funcs) < 0;

    if (is_failed == true) {
        return NULL;
    }

    struct linc_sink *sink = &sinks->list[sinks->count];
    strncpy(sink->name, name, LINC_DEFAULT_SINK_NAME_LENGTH);
    sink->name[LINC_DEFAULT_SINK_NAME_LENGTH] = '\0';
    sink->level = level;
    sink->funcs = funcs;
    sink->enabled = enabled;
    sinks->count += 1;

    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&sink->lock, &attr);
    pthread_rwlockattr_destroy(&attr);

    sink->funcs.open(sink->funcs.data);

    pthread_attr_t task_attr;
    pthread_attr_init(&task_attr);
    pthread_attr_setdetachstate(&task_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&sink->thread_id, &task_attr, linc_task, sink);
    pthread_attr_destroy(&task_attr);

    return sink;
}

struct linc_sink *linc_register_default_sink(struct linc_sink_list *sinks) {
    sinks->count = 0;
    struct linc_sink_funcs funcs = {
        .data = stderr,
        .open = linc_sink_stderr_open,
        .close = linc_sink_stderr_close,
        .write = linc_sink_stderr_write,
        .flush = linc_sink_stderr_flush,
    };
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlock_init(&sinks->lock, &attr);
    pthread_rwlockattr_destroy(&attr);
    struct linc_sink *sink = linc_add_sink(sinks, LINC_DEFAULT_SINK_NAME, LINC_LEVEL_TRACE, true, funcs);
    return sink;
}

// ==================================================
// Public Functions
// ==================================================

struct linc_sink *linc_register_sink(const char *name, enum linc_level level, bool enabled,
                                     struct linc_sink_funcs funcs) {
    linc_init();
    struct linc_sink_list *sinks = &linc.sinks;
    pthread_rwlock_wrlock(&sinks->lock);
    struct linc_sink *sink = linc_add_sink(sinks, name, level, enabled, funcs);
    pthread_rwlock_unlock(&sinks->lock);
    if (sink == NULL) {
        return NULL;
    }
    return sink;
}

int linc_set_sink_level(linc_sink sink, enum linc_level level) {
    linc_init();
    if (sink == NULL || linc_check_level_sink(level) < 0) {
        return -1;
    }
    pthread_rwlock_wrlock(&sink->lock);
    sink->level = level;
    pthread_rwlock_unlock(&sink->lock);
    return 0;
}

int linc_set_sink_enabled(linc_sink sink, bool enabled) {
    linc_init();
    if (sink == NULL) {
        return -1;
    }
    pthread_rwlock_wrlock(&sink->lock);
    sink->enabled = enabled;
    pthread_rwlock_unlock(&sink->lock);
    return 0;
}
