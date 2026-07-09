#include "internal/shared.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ==================================================
// Global State
// ==================================================

struct linc linc;
struct linc_module *linc_default_module;
struct linc_sink *linc_default_sink;

// ==================================================
// State Management
// ==================================================

#if defined(__GNUC__)
#define LINC_AT_START __attribute__((constructor))
#define LINC_BOOTSTRAP(pthread_once, routine)
#else
#define LINC_AT_START
static pthread_once_t linc_once_init = PTHREAD_ONCE_INIT;
#define LINC_BOOTSTRAP(pthread_once, routine) pthread_once((pthread_once), (routine))
#endif

static void linc_shutdown(void) {
    pthread_mutex_lock(&linc.ring_buffer.mutex);
    linc.ring_buffer.shutdown = true;
    pthread_mutex_unlock(&linc.ring_buffer.mutex);

    pthread_cond_broadcast(&linc.ring_buffer.produce);
    pthread_cond_broadcast(&linc.ring_buffer.consume);

    pthread_join(linc.ring_buffer.worker, NULL);
}

static void linc_ring_buffer_init(void) {
    linc.ring_buffer.size = LINC_DEFAULT_RING_BUFFER_SIZE + 1;
    linc.ring_buffer.head = 0;
    linc.ring_buffer.tail = 0;
    linc.ring_buffer.shutdown = false;

    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_condattr_init(&cond_attr);

    pthread_mutex_init(&linc.ring_buffer.mutex, &mutex_attr);
    pthread_cond_init(&linc.ring_buffer.produce, &cond_attr);
    pthread_cond_init(&linc.ring_buffer.consume, &cond_attr);

    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);

    pthread_attr_t worker_attr;
    pthread_attr_init(&worker_attr);
    pthread_attr_setdetachstate(&worker_attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&linc.ring_buffer.worker, &worker_attr, linc_worker, NULL);
    pthread_attr_destroy(&worker_attr);
}

static void linc_task_sync_init(void) {
    linc.task_sync.worker_ready = 0;
    linc.task_sync.sinks_ready = 0;
    linc.task_sync.worker_ended = 0;
    linc.task_sync.sinks_ended = 0;
    linc.task_sync.metadata = NULL;

    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_condattr_init(&cond_attr);

    pthread_mutex_init(&linc.task_sync.mutex, &mutex_attr);
    pthread_cond_init(&linc.task_sync.wait_worker, &cond_attr);
    pthread_cond_init(&linc.task_sync.wait_sinks, &cond_attr);

    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);
}

LINC_AT_START
static void linc_bootstrap(void) {
    memset(&linc, 0, sizeof(linc));

    linc_timestamp_offset();
    linc_ring_buffer_init();
    linc_task_sync_init();
    linc_default_module = linc_register_default_module(&linc.modules);
    linc_default_sink = linc_register_default_sink(&linc.sinks);

    atexit(linc_shutdown);
}

void linc_init(void) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
}

// ==================================================
// Ring Buffer
// ==================================================

int linc_ring_buffer_enqueue(struct linc_metadata *metadata) {
    pthread_mutex_lock(&linc.ring_buffer.mutex);
    if (linc.ring_buffer.shutdown == true) {
        pthread_mutex_unlock(&linc.ring_buffer.mutex);
        return -1;
    }

    size_t entries = (linc.ring_buffer.head + linc.ring_buffer.size - linc.ring_buffer.tail) % linc.ring_buffer.size;
    while (entries == LINC_DEFAULT_RING_BUFFER_SIZE) {
        pthread_cond_wait(&linc.ring_buffer.produce, &linc.ring_buffer.mutex);
        entries = (linc.ring_buffer.head + linc.ring_buffer.size - linc.ring_buffer.tail) % linc.ring_buffer.size;

        if (linc.ring_buffer.shutdown == true) {
            pthread_mutex_unlock(&linc.ring_buffer.mutex);
            return -1;
        }
    }

    linc.ring_buffer.buffer[linc.ring_buffer.head] = *metadata;
    linc.ring_buffer.head = (linc.ring_buffer.head + 1) % linc.ring_buffer.size;

    pthread_cond_broadcast(&linc.ring_buffer.consume);
    pthread_mutex_unlock(&linc.ring_buffer.mutex);
    return 0;
}

int linc_ring_buffer_dequeue(struct linc_metadata *metadata) {
    pthread_mutex_lock(&linc.ring_buffer.mutex);
    size_t entries = (linc.ring_buffer.head + linc.ring_buffer.size - linc.ring_buffer.tail) % linc.ring_buffer.size;
    if (linc.ring_buffer.shutdown == true && entries == 0) {
        pthread_mutex_unlock(&linc.ring_buffer.mutex);
        return -1;
    }

    while (entries == 0) {
        pthread_cond_wait(&linc.ring_buffer.consume, &linc.ring_buffer.mutex);
        entries = (linc.ring_buffer.head + linc.ring_buffer.size - linc.ring_buffer.tail) % linc.ring_buffer.size;

        if (linc.ring_buffer.shutdown == true && entries == 0) {
            pthread_mutex_unlock(&linc.ring_buffer.mutex);
            return -1;
        }
    }

    *metadata = linc.ring_buffer.buffer[linc.ring_buffer.tail];
    memset(&linc.ring_buffer.buffer[linc.ring_buffer.tail], 0, sizeof(struct linc_metadata));
    linc.ring_buffer.tail = (linc.ring_buffer.tail + 1) % linc.ring_buffer.size;

    pthread_cond_broadcast(&linc.ring_buffer.produce);
    pthread_mutex_unlock(&linc.ring_buffer.mutex);
    return 0;
}

// ==================================================
// Task Synchronization
// ==================================================

void linc_task_sync_wait_tasks(void) {
    pthread_mutex_lock(&linc.task_sync.mutex);
    while (linc.task_sync.sinks_ready < linc.sinks.count) {
        linc.task_sync.worker_ready++;
        pthread_cond_wait(&linc.task_sync.wait_sinks, &linc.task_sync.mutex);
        linc.task_sync.worker_ready--;
    }
    linc.task_sync.sinks_ready = 0;
    linc.task_sync.worker_ready = 0;
    linc.task_sync.metadata = NULL;
    pthread_mutex_unlock(&linc.task_sync.mutex);
}

void linc_task_sync_wait_worker(void) {
    pthread_mutex_lock(&linc.task_sync.mutex);
    linc.task_sync.sinks_ready++;
    if (linc.task_sync.sinks_ready == linc.sinks.count && linc.task_sync.worker_ready == 1) {
        pthread_cond_signal(&linc.task_sync.wait_sinks);
    }
    pthread_cond_wait(&linc.task_sync.wait_worker, &linc.task_sync.mutex);
    pthread_mutex_unlock(&linc.task_sync.mutex);
}

void linc_task_sync_signal_tasks(void) {
    pthread_mutex_lock(&linc.task_sync.mutex);
    pthread_cond_broadcast(&linc.task_sync.wait_worker);
    while (linc.task_sync.sinks_ended < linc.sinks.count) {
        linc.task_sync.worker_ended++;
        pthread_cond_wait(&linc.task_sync.wait_sinks, &linc.task_sync.mutex);
        linc.task_sync.worker_ended--;
    }
    linc.task_sync.sinks_ended = 0;
    linc.task_sync.worker_ended = 0;
    linc.task_sync.metadata = NULL;
    pthread_cond_broadcast(&linc.task_sync.wait_worker);
    pthread_mutex_unlock(&linc.task_sync.mutex);
}

void linc_task_sync_signal_worker(void) {
    pthread_mutex_lock(&linc.task_sync.mutex);
    linc.task_sync.sinks_ended++;
    if (linc.task_sync.sinks_ended == linc.sinks.count && linc.task_sync.worker_ended == 1) {
        pthread_cond_signal(&linc.task_sync.wait_sinks);
    }

    pthread_cond_wait(&linc.task_sync.wait_worker, &linc.task_sync.mutex);
    pthread_mutex_unlock(&linc.task_sync.mutex);
}
