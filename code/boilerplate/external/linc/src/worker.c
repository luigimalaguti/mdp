#include "internal/shared.h"

#include <pthread.h>
#include <string.h>

// ==================================================
// Internal Functions
// ==================================================

static int linc_check_sink(struct linc_sink *sink, enum linc_level level) {
    if (sink == NULL || level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }

    pthread_rwlock_rdlock(&sink->lock);
    bool is_sink_enabled = sink->enabled == true;
    bool is_level_valid = level >= sink->level;
    pthread_rwlock_unlock(&sink->lock);

    if (!is_sink_enabled || !is_level_valid) {
        return -1;
    }
    return 0;
}

void *linc_task(void *arg) {
    struct linc_sink *sink = (struct linc_sink *)arg;

    while (true) {
        linc_task_sync_wait_worker();
        struct linc_metadata *metadata = linc.task_sync.metadata;
        if (metadata == NULL) {
            break;
        }
        int sink_check = linc_check_sink(sink, metadata->level);
        if (sink_check == 0) {
            sink->funcs.write(sink->funcs.data, metadata);
        }
        linc_task_sync_signal_worker();
    }
    sink->funcs.flush(sink->funcs.data);
    sink->funcs.close(sink->funcs.data);
    linc_task_sync_signal_worker();

    pthread_exit(0);
}

void *linc_worker(void *arg) {
    (void)arg;

    while (true) {
        struct linc_metadata metadata;
        if (linc_ring_buffer_dequeue(&metadata) < 0) {
            pthread_rwlock_rdlock(&linc.sinks.lock);
            linc_task_sync_wait_tasks();
            linc.task_sync.metadata = NULL;
            linc_task_sync_signal_tasks();
            pthread_rwlock_unlock(&linc.sinks.lock);
            break;
        }

        pthread_rwlock_rdlock(&linc.sinks.lock);
        linc_task_sync_wait_tasks();
        linc.task_sync.metadata = &metadata;
        linc_task_sync_signal_tasks();
        pthread_rwlock_unlock(&linc.sinks.lock);
    }

    pthread_exit(0);
}
