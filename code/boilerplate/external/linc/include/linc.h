#ifndef LINC_INCLUDE_LINC_H
#define LINC_INCLUDE_LINC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ==================================================
// Macros and Definitions
// ==================================================

#if defined(__GNUC__)
#define LINC_PRINT_FMT(fmt, args) \
    __attribute__((format(printf, fmt, args)))  // GCC format attribute for printf-style functions
#else
#define LINC_PRINT_FMT(fmt, args)
#endif

#if !defined(LINC_DEFAULT_LEVEL)
#define LINC_DEFAULT_LEVEL LINC_LEVEL_INFO  // Default log level
#elif (LINC_DEFAULT_LEVEL < LINC_LEVEL_TRACE) || (LINC_DEFAULT_LEVEL > LINC_LEVEL_FATAL)
#error "LINC_DEFAULT_LEVEL must be between LINC_LEVEL_TRACE and LINC_LEVEL_FATAL"
#endif

#if !defined(LINC_DEFAULT_MODULE_NAME_LENGTH)
#define LINC_DEFAULT_MODULE_NAME_LENGTH 16  // Maximum length for module names
#elif (LINC_DEFAULT_MODULE_NAME_LENGTH < 1)
#error "LINC_DEFAULT_MODULE_NAME_LENGTH must be at least 1"
#endif

#if !defined(LINC_DEFAULT_MODULE_NAME)
#define LINC_DEFAULT_MODULE_NAME "main"  // Default module name
#elif (sizeof(LINC_DEFAULT_MODULE_NAME) > LINC_DEFAULT_MODULE_NAME_LENGTH) || (sizeof(LINC_DEFAULT_MODULE_NAME) < 1)
#error "LINC_DEFAULT_MODULE_NAME must be between 1 and LINC_DEFAULT_MODULE_NAME_LENGTH characters"
#endif

#if !defined(LINC_DEFAULT_MAX_MODULES)
#define LINC_DEFAULT_MAX_MODULES 8  // Maximum number of modules
#elif (LINC_DEFAULT_MAX_MODULES < 1)
#error "LINC_DEFAULT_MAX_MODULES must be at least 1"
#endif

#if !defined(LINC_DEFAULT_SINK_NAME_LENGTH)
#define LINC_DEFAULT_SINK_NAME_LENGTH 16  // Maximum length for sink names
#elif (LINC_DEFAULT_SINK_NAME_LENGTH < 1)
#error "LINC_DEFAULT_SINK_NAME_LENGTH must be at least 1"
#endif

#if !defined(LINC_DEFAULT_SINK_NAME)
#define LINC_DEFAULT_SINK_NAME "stderr"  // Default sink name
#elif (sizeof(LINC_DEFAULT_SINK_NAME) > LINC_DEFAULT_SINK_NAME_LENGTH) || (sizeof(LINC_DEFAULT_SINK_NAME) < 1)
#error "LINC_DEFAULT_SINK_NAME must be between 1 and LINC_DEFAULT_SINK_NAME_LENGTH characters"
#endif

#if !defined(LINC_DEFAULT_MAX_SINKS)
#define LINC_DEFAULT_MAX_SINKS 8  // Maximum number of sinks
#elif (LINC_DEFAULT_MAX_SINKS < 1)
#error "LINC_DEFAULT_MAX_SINKS must be at least 1"
#endif

#if !defined(LINC_DEFAULT_MAX_MESSAGE_LENGTH)
#define LINC_DEFAULT_MAX_MESSAGE_LENGTH 512  // Default maximum length for log messages
#elif (LINC_DEFAULT_MAX_MESSAGE_LENGTH < 1)
#error "LINC_DEFAULT_MAX_MESSAGE_LENGTH must be at least 1"
#endif

#if !defined(LINC_DEFAULT_RING_BUFFER_SIZE)
#define LINC_DEFAULT_RING_BUFFER_SIZE 1024  // Default size for the ring buffer
#elif (LINC_DEFAULT_RING_BUFFER_SIZE < 1)
#error "LINC_DEFAULT_RING_BUFFER_SIZE must be at least 1"
#endif

#define LINC_ZERO_CHAR_LENGTH 1     // Zero character length
#define LINC_NEWLINE_CHAR_LENGTH 1  // Newline character length

// ==================================================
// Structures and Enums
// ==================================================

enum linc_level {
    LINC_LEVEL_TRACE = 0,  // Lowest level, for detailed debugging information
    LINC_LEVEL_DEBUG = 1,  // Debugging information, useful for developers
    LINC_LEVEL_INFO = 2,   // General information about application state
    LINC_LEVEL_WARN = 3,   // Warning messages, indicating potential issues
    LINC_LEVEL_ERROR = 4,  // Error messages, indicating something went wrong
    LINC_LEVEL_FATAL = 5,  // Critical errors that cause the application to terminate
};

struct linc_metadata {
        int64_t timestamp;        // Timestamp in nanoseconds since epoch
        enum linc_level level;    // Level of the log
        uintptr_t thread_id;      // Thread ID where the log was generated
        const char *module_name;  // Module name where the log was generated
        const char *filename;     // Source file where the log was generated
        uint32_t line;            // Line number in the source file
        const char *func;         // Function name where the log was generated
        char message[LINC_DEFAULT_MAX_MESSAGE_LENGTH + LINC_ZERO_CHAR_LENGTH];  // Log message content
};

struct linc_sink_funcs {
        void *data;                                                // User-defined data pointer
        int (*open)(void *data);                                   // Function to open/init the sink
        int (*close)(void *data);                                  // Function to close/deinit the sink
        int (*write)(void *data, struct linc_metadata *metadata);  // Function to write a log message
        int (*flush)(void *data);                                  // Function to flush the sink (if applicable)
};

typedef struct linc_module *linc_module;  // Opaque pointer to a module
typedef struct linc_sink *linc_sink;      // Opaque pointer to a sink

extern linc_module linc_default_module;  // Default module
extern linc_sink linc_default_sink;      // Default sink

// ==================================================
// Functions
// ==================================================

void linc_log(linc_module module, enum linc_level level, const char *filename, uint32_t line, const char *func,
              const char *format, ...) LINC_PRINT_FMT(6, 7);

#define TRACE(...) linc_log(linc_default_module, LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG(...) linc_log(linc_default_module, LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define INFO(...) linc_log(linc_default_module, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define WARN(...) linc_log(linc_default_module, LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ERROR(...) linc_log(linc_default_module, LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define FATAL(...) linc_log(linc_default_module, LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define TRACE_M(module, ...) linc_log(module, LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG_M(module, ...) linc_log(module, LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define INFO_M(module, ...) linc_log(module, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define WARN_M(module, ...) linc_log(module, LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ERROR_M(module, ...) linc_log(module, LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define FATAL_M(module, ...) linc_log(module, LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

linc_module linc_register_module(const char *name, enum linc_level level, bool enabled);
linc_sink linc_register_sink(const char *name, enum linc_level level, bool enabled, struct linc_sink_funcs funcs);

int linc_set_module_level(linc_module module, enum linc_level level);
int linc_set_module_enabled(linc_module module, bool enabled);

int linc_set_sink_level(linc_sink sink, enum linc_level level);
int linc_set_sink_enabled(linc_sink sink, bool enabled);

int64_t linc_timestamp(void);
int linc_timestamp_string(int64_t timestamp, char *buffer, size_t size);
const char *linc_level_string(enum linc_level level);
int linc_stringify_metadata(struct linc_metadata *metadata, char *buffer, size_t length, bool use_colors);

#endif  // LINC_INCLUDE_LINC_H
