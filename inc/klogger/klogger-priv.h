#ifndef KLOGGER_PRIV_H
#define KLOGGER_PRIV_H

/*
    This is the private header for the KLogger.
    Do not include it directly

    Author: Michal Kukowski
    email: michalkukowski10@gmail.com
    LICENCE: GPL3
*/

#ifndef KLOGGER_H
#error "Never include <klogger/klogger-priv.h> directly, use <klogger/klogger.h> instead."
#endif

#ifndef __GNUC__
#error "Gnu extension is required to compile code with KLogger!"
#endif

/* C11 has value 201112L */
#if __STDC_VERSION__ < 201112L
#error "At least C11 is required to compile code with KLogger!"
#endif

#include <stdint.h>

/* Need bitwise operations, so instead of enum use uint32_t + defines like in POSIX */
typedef uint32_t klogger_option_t;
#define KLOGGER_PRIV_OPTIONS_STDOUT_DUPLICATE    (1 << 0)
#define KLOGGER_PRIV_OPTIONS_STDERR_DUPLICATE    (1 << 1)
#define KLOGGER_PRIV_OPTIONS_FILE_DUPLICATE      (1 << 2)
#define KLOGGER_PRIV_OPTIONS_USE_TIMESTAMP       (1 << 3)
#define KLOGGER_PRIV_OPTIONS_USE_THREADID        (1 << 4)

/* Integer values are critical for this framework functionality, so I decided to hardcode them */
typedef enum klogger_priv_level
{
    KLOGGER_PRIV_LEVEL_FATAL    = 0,
    KLOGGER_PRIV_LEVEL_CRITICAL = 1,
    KLOGGER_PRIV_LEVEL_ERROR    = 2,
    KLOGGER_PRIV_LEVEL_WARNING  = 3,
    KLOGGER_PRIV_LEVEL_INFO     = 4,
    KLOGGER_PRIV_LEVEL_DEBUG    = 5,
    KLOGGER_PRIV_LEVEL_DEBUG2   = 6,
    KLOGGER_PRIV_LEVEL_DEBUG3   = 7,
    KLOGGER_PRIV_LEVEL_MAX      = 7,
} klogger_level_t;

void __attribute__(( format(printf, 5, 6) )) __klogger_print(const char* file,
                                                             const char* func,
                                                             int line,
                                                             klogger_level_t level,
                                                             const char* fmt,
                                                             ...);

#define KLOG_PRIV_GENERAL(LVL, ...) __klogger_print(__FILE__, __func__, __LINE__, LVL, __VA_ARGS__)

#define KLOG_PRIV_FATAL(...)     KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_FATAL, __VA_ARGS__)
#define KLOG_PRIV_CRITICAL(...)  KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_CRITICAL, __VA_ARGS__)
#define KLOG_PRIV_ERROR(...)     KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_ERROR, __VA_ARGS__)
#define KLOG_PRIV_WARNING(...)   KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_WARNING, __VA_ARGS__)
#define KLOG_PRIV_INFO(...)      KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_INFO, __VA_ARGS__)
#define KLOG_PRIV_DEBUG(...)     KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_DEBUG, __VA_ARGS__)
#define KLOG_PRIV_DEBUG2(...)    KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_DEBUG2, __VA_ARGS__)
#define KLOG_PRIV_DEBUG3(...)    KLOG_PRIV_GENERAL(KLOGGER_PRIV_LEVEL_DEBUG3, __VA_ARGS__)

#endif
