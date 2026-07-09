#ifndef UTINC_INCLUDE_INTERNAL_H
#define UTINC_INCLUDE_INTERNAL_H

#include "utinc.h"

#define UTINC_LINE_WIDTH 80

#define UTINC_COLOR_RESET "\x1b[0m"
#define UTINC_COLOR_BOLD "\x1b[1m"
#define UTINC_COLOR_DIM "\x1b[2m"
#define UTINC_COLOR_RED "\x1b[31m"
#define UTINC_COLOR_GREEN "\x1b[32m"
#define UTINC_COLOR_YELLOW "\x1b[33m"
#define UTINC_COLOR_BLUE "\x1b[34m"
#define UTINC_COLOR_CYAN "\x1b[36m"
#define UTINC_COLOR_GRAY "\x1b[90m"

struct utinc {
        struct utinc_stats stats;
        struct utinc_time start_times;
        void (*before_all_callback)(void);
        void (*after_all_callback)(void);
        void (*before_each_callback)(void);
        void (*after_each_callback)(void);
};

#endif  // UTINC_INCLUDE_INTERNAL_H
