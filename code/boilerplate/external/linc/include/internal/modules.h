#ifndef LINC_INCLUDE_INTERNAL_MODULES_H
#define LINC_INCLUDE_INTERNAL_MODULES_H

#include "linc.h"

#include <pthread.h>

// ==================================================
// Structures and Enums
// ==================================================

struct linc_module {
        char name[LINC_DEFAULT_MODULE_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH];  // Module name
        enum linc_level level;                                               // Minimum log level for this module
        bool enabled;                                                        // Is module enabled
        pthread_rwlock_t lock;                                               // Lock for thread safety
};

struct linc_module_list {
        struct linc_module list[LINC_DEFAULT_MAX_MODULES];  // List of registered modules
        size_t count;                                       // Number of registered modules
        pthread_mutex_t mutex;                              // Mutex for thread safety
};

// ==================================================
// Internal Functions
// ==================================================

struct linc_module *linc_register_default_module(struct linc_module_list *modules);

// ==================================================
// Public Functions (linc.h)
// ==================================================

// linc_module linc_register_module(const char *name, enum linc_level level, bool enabled);
// int linc_set_module_level(linc_module module, enum linc_level level);
// int linc_set_module_enabled(linc_module module, bool enabled);

#endif  // LINC_INCLUDE_INTERNAL_MODULES_H
