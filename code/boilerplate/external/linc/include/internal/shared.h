#ifndef LINC_INCLUDE_INTERNAL_SHARED_H
#define LINC_INCLUDE_INTERNAL_SHARED_H

#include "internal/modules.h"
#include "internal/sinks.h"
#include "linc.h"

// ==================================================
// Macros
// ==================================================

#define LINC_PARAM_STR(param) #param                 // Expands the parameter and converts it to a string
#define LINC_STRINGIFY(param) LINC_PARAM_STR(param)  // Stringifies the parameter

// [ 0000-00-00 00:00:00.000 ] [ LEVEL ] [ 012abc ] [ module ] filename.c:12345 function: Log message
//
// 0000-00-00 00:00:00.000                      -> LINC_LOG_TIMESTAMP_LENGTH
// LEVEL                                        -> LINC_LOG_LEVEL_LENGTH
// 012abc                                       -> LINC_LOG_THREAD_ID_LENGTH
// module                                       -> LINC_DEFAULT_MODULE_NAME_LENGTH
// filename.c                                   -> LINC_LOG_FILE_LENGTH
// 12345                                        -> LINC_LOG_LINE_LENGTH
// function                                     -> LINC_LOG_FUNC_LENGTH
// Log message                                  -> LINC_DEFAULT_MAX_MESSAGE_LENGTH
//
// [ -- ] [ -- ] [ -- ] [ -- ] --:-- --: --     -> LINC_LOG_EXTRA_FMT_LENGTH
// 5 [ 10 ] [ 15 ] [ 10 ] [ 10 ] 10:10 10: --   -> LINC_LOG_COLORS_FMT_LENGTH

#define LINC_LOG_TIMESTAMP_LENGTH 23  // Length of timestamp string "YYYY-MM-DD HH:MM:SS.mmm"
#define LINC_LOG_LEVEL_LENGTH 5       // Length of log level string
#define LINC_LOG_THREAD_ID_LENGTH 16  // Length of thread ID string
#define LINC_LOG_FILE_LENGTH 64       // Length of file name string
#define LINC_LOG_LINE_LENGTH 10       // Length of line number string
#define LINC_LOG_FUNC_LENGTH 64       // Length of function name string

#define LINC_LOG_EXTRA_FMT_LENGTH 24   // Extra characters for formatting, e.g., [ ], spaces, etc.
#define LINC_LOG_COLORS_FMT_LENGTH 80  // Extra characters for ANSI color codes

#define LINC_LOG_MAX_LENGTH                                                                                            \
    (LINC_LOG_TIMESTAMP_LENGTH + LINC_LOG_LEVEL_LENGTH + LINC_LOG_THREAD_ID_LENGTH + LINC_DEFAULT_MODULE_NAME_LENGTH + \
     LINC_LOG_FILE_LENGTH + LINC_LOG_LINE_LENGTH + LINC_LOG_FUNC_LENGTH + LINC_DEFAULT_MAX_MESSAGE_LENGTH +            \
     LINC_LOG_EXTRA_FMT_LENGTH + LINC_LOG_COLORS_FMT_LENGTH)

#define LINC_COLOR_RESET "\x1b[0m"
#define LINC_COLOR_BOLD "\x1b[1m"
#define LINC_COLOR_DIM "\x1b[2m"
#define LINC_COLOR_NORMAL "\x1b[22m"
#define LINC_COLOR_RED "\x1b[91m"
#define LINC_COLOR_GREEN "\x1b[92m"
#define LINC_COLOR_YELLOW "\x1b[93m"
#define LINC_COLOR_BLUE "\x1b[94m"
#define LINC_COLOR_MAGENTA "\x1b[95m"
#define LINC_COLOR_CYAN "\x1b[96m"
#define LINC_COLOR_WHITE "\x1b[97m"

// ==================================================
// Structures and Enums
// ==================================================

struct linc_ring_buffer {
        struct linc_metadata
            buffer[LINC_DEFAULT_RING_BUFFER_SIZE + 1];  // Ring buffer (+1 to distinguish full vs empty)
        size_t head;                                    // Points to the next position to write
        size_t tail;                                    // Points to the next position to read
        size_t size;                                    // Maximum number of elements in the buffer
        bool shutdown;                                  // Indicates if the worker thread should shut down
        pthread_t worker;                               // Worker thread handle
        pthread_mutex_t mutex;                          // Mutex for synchronizing access
        pthread_cond_t produce, consume;                // Condition variables for signaling
};

struct linc_task_sync {
        pthread_mutex_t mutex;                   // Mutex for synchronizing access
        pthread_cond_t wait_worker, wait_sinks;  // Condition variables for signaling
        size_t worker_ready, sinks_ready;        // Number of worker/sinks ready
        size_t worker_ended, sinks_ended;        // Number of worker/sinks ended
        struct linc_metadata *metadata;          // Pointer to the current metadata being processed
};

struct linc {
        struct linc_module_list modules;      // List of registered modules
        struct linc_sink_list sinks;          // List of registered sinks
        struct linc_ring_buffer ring_buffer;  // Ring buffer for log messages
        struct linc_task_sync task_sync;      // Synchronization for sinking tasks
};

extern struct linc linc;

// ==================================================
// Internal Functions
// ==================================================

void linc_init(void);
void linc_timestamp_offset(void);

int linc_ring_buffer_enqueue(struct linc_metadata *metadata);
int linc_ring_buffer_dequeue(struct linc_metadata *metadata);

void linc_task_sync_wait_tasks(void);
void linc_task_sync_wait_worker(void);
void linc_task_sync_signal_tasks(void);
void linc_task_sync_signal_worker(void);

void *linc_worker(void *arg);
void *linc_task(void *arg);

// ==================================================
// Public Functions (linc.h)
// ==================================================

// int64_t linc_timestamp(void);
// int linc_timestamp_string(int64_t timestamp, char *buffer, size_t size);
// const char *linc_level_string(enum linc_level level);
// int linc_stringify_metadata(struct linc_metadata *metadata, char *buffer, size_t length, bool use_colors);

#endif  // LINC_INCLUDE_INTERNAL_SHARED_H
