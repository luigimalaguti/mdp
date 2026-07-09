#ifndef LINC_INCLUDE_INTERNAL_SINKS_H
#define LINC_INCLUDE_INTERNAL_SINKS_H

#include "linc.h"

#include <pthread.h>

// ==================================================
// Structures and Enums
// ==================================================

struct linc_sink {
        char name[LINC_DEFAULT_SINK_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH];  // Sink name
        enum linc_level level;                                             // Minimum log level for this sink
        bool enabled;                                                      // Is sink enabled
        struct linc_sink_funcs funcs;                                      // Function pointers for sink operations
        pthread_t thread_id;                                               // Thread ID for async operations
        pthread_rwlock_t lock;                                             // Lock for thread safety
};

struct linc_sink_list {
        struct linc_sink list[LINC_DEFAULT_MAX_SINKS];  // List of registered sinks
        size_t count;                                   // Number of registered sinks
        pthread_rwlock_t lock;                          // Lock for thread safety
};

// ==================================================
// Internal Functions
// ==================================================

struct linc_sink *linc_register_default_sink(struct linc_sink_list *sinks);

// ==================================================
// Public Functions (linc.h)
// ==================================================

// linc_sink linc_register_sink(const char *name, enum linc_level level, bool enabled, struct linc_sink_funcs funcs);
// int linc_set_sink_level(linc_sink sink, enum linc_level level);
// int linc_set_sink_enabled(linc_sink sink, bool enabled);

#endif  // LINC_INCLUDE_INTERNAL_SINKS_H
