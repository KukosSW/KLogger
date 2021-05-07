#ifndef KLOGGER_H
#define KLOGGER_H

/*
    This is the main header of the KAssert, you shall include only this header.

    Author: Michal Kukowski
    email: michalkukowski10@gmail.com
    LICENCE: GPL3


    KLogger is a almost full user customizable logging C library. By passing main file descriptor
    and predefined options you can set logging system as you want.

    Main features:
    - auto file generation + logging into file
    - logging on a few descriptors at the same time
    - supporiting any valid decriptor as a main fd (you can send logs via socket)
    - library is full multithread safe, but it requires pthread library
    - library can be disbaled to create release version with no additional operation
      just define NDEBUG and KLOGGER_FATAL_SILENT
    - this header contains short description about logger levels, you can follow this style
      or you can use levels as you want
    - logger has state machine to tell user what did wrong
    - you can use macros like any other printf function
*/

#include "klogger-priv.h"

/**
 * Level of debugging. When you are using level X, all levels <= X will be logged
 * others will be skipped, so please init logger properly.
 * To disable logging to get full release version of program you can define NDEBUG like in case of assert
 *
 * KLOGGER_LEVEL_FATAL       - fatal error, application is going to terminate immediately
 *                             this cannot be suppress by log level, you need to use
 *                             special define KLOGGER_FATAL_SILENT (work only if NDEBUG is defined)
 *
 * KLOGGER_LEVEL_CIRTICAL    - critical condition occured like hardware error, system error.
 *                             You can wait for another try,
 *                             so you do not need to terminate a program now
 *                             Example: open file ends with error so you need to try again
 *
 * KLOGGER_LEVEL_ERROR       - your app error but controled by current layer.
 *                             Function (layer) cannot decide to terminate app or not.
 *                             Use this level for functions error before exit from function.
 *                             Example: NULL pointer passed to function which needs proper pointer
 *
 * KLOGGER_LEVEL_WARNING     - something goes wrong, but you can handle it and continue program.
 *                             This level should be used to tell user that maybe he has a bug
 *                             on upper layer, or he should fix something for better performance
 *                             Example: unaligned memory,
 *                                      existing file which should be cleaned by app before
 *
 * KLOGGER_LEVEL_INFO        - You have important message for upper layer or user.
 *                             This level is not for bad news. If something goes wrong use another level
 *                             Example: Thread created, some libary loaded, some framework inited
 *
 * KLOGGER_LEVEL_DEBUG       - This level is for you (programmer) to debug your code.
 *                             Use this level as you want. You can split dbg messages for 3 levels
 *                             to get less dbg messages during debugging
 *                             Please note that if you want to create message for release version
 *                             use another level.
 *
 * KLOGGER_LEVEL_DEBUG2      - Like KLOGGER_LEVEL_DEBUG but for less important messages
 *
 * KLOGGER_LEVEL_DEBUG3      - Like KLOGGER_LEVEL_DEBUG but for less important messages
 */
#define KLOGGER_LEVEL_FATAL    KLOGGER_PRIV_LEVEL_FATAL
#define KLOGGER_LEVEL_CRITICAL KLOGGER_PRIV_LEVEL_CRITICAL
#define KLOGGER_LEVEL_ERROR    KLOGGER_PRIV_LEVEL_ERROR
#define KLOGGER_LEVEL_WARNING  KLOGGER_PRIV_LEVEL_WARNING
#define KLOGGER_LEVEL_INFO     KLOGGER_PRIV_LEVEL_INFO
#define KLOGGER_LEVEL_DEBUG    KLOGGER_PRIV_LEVEL_DEBUG
#define KLOGGER_LEVEL_DEBUG2   KLOGGER_PRIV_LEVEL_DEBUG2
#define KLOGGER_LEVEL_DEBUG3   KLOGGER_PRIV_LEVEL_DEBUG3
#define KLOGGER_LEVEL_MAX      KLOGGER_PRIV_LEVEL_MAX


/**
 * Options can be combined using bitwise OR (|) operator
 * KLogger follows posix file options to make it easier to understand
 *
 * Lets assume that you have socket on which you want to send logs
 * + you want see logs on stderr and you need a timestamp
 * Then as a options you need to pass:
 * KLOGGER_OPTIONS_STDERR_DUPLICATE | KLOGGER_OPTIONS_USE_TIMESTAMP
 *
 * IF you do not know what you need, use default option
 * or multithread default option if your program has more than 1 thread (or proc)
 *
 */
#define KLOGGER_OPTIONS_STDOUT_DUPLICATE     KLOGGER_PRIV_OPTIONS_STDOUT_DUPLICATE
#define KLOGGER_OPTIONS_STDERR_DUPLICATE     KLOGGER_PRIV_OPTIONS_STDERR_DUPLICATE
#define KLOGGER_OPTIONS_FILE_DUPLICATE       KLOGGER_PRIV_OPTIONS_FILE_DUPLICATE
#define KLOGGER_OPTIONS_USE_TIMESTAMP        KLOGGER_PRIV_OPTIONS_USE_TIMESTAMP
#define KLOGGER_OPTIONS_USE_THREADID         KLOGGER_PRIV_OPTIONS_USE_THREADID

#define KLOGGER_OPTIONS_DEFAULT              (KLOGGER_OPTIONS_STDERR_DUPLICATE | KLOGGER_OPTIONS_FILE_DUPLICATE | KLOGGER_OPTIONS_USE_TIMESTAMP)
#define KLOGGER_OPTIONS_MULTITHREAD_DEFAULT  (KLOGGER_OPTIONS_DEFAULT | KLOGGER_OPTIONS_USE_THREADID)

/**
 * This function initializes klogger. Shall be call only once before any othe klogger functions.
 * If you want to log only to file, pass as fd -1 and add to options KLOGGER_OPTIONS_FILE_DUPLICATE
 * If you want to log to stdout and to file pass as fd -1 and
 * to options KLOGGER_OPTIONS_FILE_DUPLICATE | KLOGGER_OPTIONS_STDERR_DUPLICATE
 * If you want to log on socket, stderr and file pass as fd socket fd
 * and add to options  KLOGGER_OPTIONS_FILE_DUPLICATE | KLOGGER_OPTIONS_STDERR_DUPLICATE
 *
 * By default (using KLOGGER_OPTIONS_DEFAULT) logging on stderr, to file with timestamps are enabled
 * When your program will be multithread use KLOGGER_OPTIONS_MULTITHREAD_DEFAULT for default options
 *
 * @param[in] fd      - main file descriptor can be any correct descriptor,
 *                      but should be used for others than stdout and stderr,
 *                      to log on console use options instead
 * @param[in] level   - max level to print (see klogger_level_t for details)
 * @param[in] options - user options(see klogger_option_t for details)
 *
 * @return 0 on success, non-zero value on fail
 */
int klogger_init(int fd, klogger_level_t level, klogger_option_t options);

/**
 * This function will destroy all klogger private data. Call only once after init and use
 */
void klogger_deinit(void);

/**
 * NDEBUG like in case of assert can change code into full release version without any logging
 * Please note that to suppress KLOG_FATAL you need to define also KLOGGER_FATAL_SILENT
 */
#ifndef NDEBUG
/**
 * Use this macros to log your messages.
 * Those macros works like printf
 *
 * If level passed to klogger_init is greater of equal to macro level (see klogger_level_t)
 * then macro will log something to proper descriptor, otherwise will do nothing
 */
#define KLOG_FATAL(...)     KLOG_PRIV_FATAL(__VA_ARGS__)
#define KLOG_CRITICAL(...)  KLOG_PRIV_CRITICAL(__VA_ARGS__)
#define KLOG_ERROR(...)     KLOG_PRIV_ERROR(__VA_ARGS__)
#define KLOG_WARNING(...)   KLOG_PRIV_WARNING(__VA_ARGS__)
#define KLOG_INFO(...)      KLOG_PRIV_INFO(__VA_ARGS__)
#define KLOG_DEBUG(...)     KLOG_PRIV_DEBUG(__VA_ARGS__)
#define KLOG_DEBUG2(...)    KLOG_PRIV_DEBUG2(__VA_ARGS__)
#define KLOG_DEBUG3(...)    KLOG_PRIV_DEBUG3(__VA_ARGS__)

#else /* #ifndef NDEBUG */

/* KLOG_FATAL needs another define */
#ifndef KLOGGER_FATAL_SILENT

#define KLOG_FATAL(...) KLOG_PRIV_FATAL(__VA_ARGS__)

#else /* #ifndef KLOGGER_FATAL_SILENT */

#define KLOG_FATAL(...)

#endif /* #ifndef KLOGGER_FATAL_SILENT */

#define KLOG_CRITICAL(...)
#define KLOG_ERROR(...)
#define KLOG_WARNING(...)
#define KLOG_INFO(...)
#define KLOG_DEBUG(...)
#define KLOG_DEBUG2(...)
#define KLOG_DEBUG3(...)

#endif /* #ifndef NDEBUG */

#endif /* include guard */
