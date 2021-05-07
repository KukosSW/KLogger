# KLogger - Kukos's logger


### Getting started:
Before you use this code I suggest to see, compile and play with examples which can be found in this repo (example directory)

To compile examples type:

````
$make all
````

or

````
$make example
````

## Features
KLogger is a almost full user customizable logging C library. By passing main file descriptor and predefined options you can set logging system as you want.
* Auto file generation + logging into this file when option is set.
* Auto new line detection. When you have forgotten new line in print, framework will add it by itself. But if you pass new line, them no new additional line will be added.
* Logging on a few descriptors at the same time, at most 4 descriptors: main fd, stdout, stderr, file
* Setting any valid descriptor as a main fd. You can set socket as a main fd
* Library is full multithread safe, but it requires pthread library. Your code needs pthread also to compile it with this library
* Getting useful information on FATAL level like StackTrace. Please note that full stacktrace can be printed only if program is compiled with **-rdynamic** flag.
* Library can be disbaled to create release version with no additional operation. Just define NDEBUG (disabling all except fatal) and KLOGGER_FATAL_SILENT (disabling fatal when NDEBUG is defined)
* Main header contains short description about logger levels, you can follow this style or you can use levels as you want. A few levels help you to create a code with simpler debugging system. You can enable only important levels to see less prints during debugging.
* KLogger has state machine to tell user what did wrong


## Platforms
For now KLogger has been tested only on Linux.

## Requirements
* Compiler with GnuC dialect and at least C11 standard
* Pthread library linked to program which is using klogger
* Makefile

## How to build

You can type make help to get list of all targets:
````
KLogger Makefile

Targets:
    all               - build klogger and examples
    lib               - build only klogger library
    examples          - examples
    install[P = Path] - install klogger to path P or default Path

Makefile supports Verbose mode when V=1
To check default compiler (gcc) change CC variable (i.e export CC=clang)
````
## How to install
To install KLogger on your computer you can use

````
make install [P=path]
````
or you can execute script directly from

````
scripts/install_klogger.sh [P=path]
````

Let's assume that you have your project MyProject and directories as follows:
```
.
├── app
├── external
├── inc
├── src
└── tests
````

You want to install KLogger in external/Klogger directory.

````
1. Download this repo
2. $make install P=/home/$user/MyProject/external/Klogger
3. Now you need to link include files and libklogger.a file, you can add to your compile this options
-I/home/$user/MyProject/external/Klogger/inc -L/home/$user/MyProject/external/Klogger -lklogger -lpthread
4. You can pass to compiler -rdynamic to get better Stacktrace prints.
5. In your files you need include main header: #include <klogger/klogger.h>
6. Write your code, add loggs and enjoy! Please see examples for details.
````

## How to use
````c
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
````

## Example
````c
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <klogger/klogger.h>

void example1(void);
void example2(void);
void example3(void);
void example4(void);

/*
    Log on stderr + auto file
    Get timestamp
    Enable all levels

    Output:
    [FATAL   ] [12:11:10.361632] example/main.c:10 example1: First msg
    Stacktrace:
    ./example.out() [0x4019ee]
    ./example.out() [0x401af3]
    ./example.out() [0x401c16]
    /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf3) [0x7f700e0040b3]
    ./example.out() [0x4011ee]
    [CRITICAL] [12:11:10.362121] example/main.c:11 example1: Msg 1
    [ERROR   ] [12:11:10.362170] example/main.c:12 example1: Msg 2
    [WARNING ] [12:11:10.362199] example/main.c:13 example1: Msg 3
    [INFO    ] [12:11:10.362230] example/main.c:14 example1: Msg 4
    [DEBUG   ] [12:11:10.362259] example/main.c:15 example1: Msg 5
    [DEBUG2  ] [12:11:10.362285] example/main.c:16 example1: Msg 6
    [DEBUG3  ] [12:11:10.362311] example/main.c:17 example1: Msg 7
*/
void example1(void)
{
    klogger_init(-1, KLOGGER_LEVEL_MAX, KLOGGER_OPTIONS_DEFAULT);

    unsigned msg_idx = 1;
    KLOG_FATAL("First msg\n");
    KLOG_CRITICAL("Msg %u\n", msg_idx++);
    KLOG_ERROR("Msg %u\n", msg_idx++);
    KLOG_WARNING("Msg %u\n", msg_idx++);
    KLOG_INFO("Msg %u\n", msg_idx++);
    KLOG_DEBUG("Msg %u\n", msg_idx++);
    KLOG_DEBUG2("Msg %u\n", msg_idx++);
    KLOG_DEBUG3("Msg %u\n", msg_idx++);

    klogger_deinit();
}

/*
    Log on stderr + auto file
    Get timestamp + TID
    Enable level <= WARNING

    Output:
    [FATAL   ] [12:12:40.257240] [TID: 791898] example/main.c:50 example2: First msg
    Stacktrace:
    ./example.out() [0x4019ee]
    ./example.out() [0x401c43]
    ./example.out() [0x401d66]
    /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf3) [0x7f9d1cdf50b3]
    ./example.out() [0x4011ee]
    [CRITICAL] [12:12:40.257718] [TID: 791898] example/main.c:51 example2: Msg 1
    [ERROR   ] [12:12:40.257769] [TID: 791898] example/main.c:52 example2: Msg 2
    [WARNING ] [12:12:40.257801] [TID: 791898] example/main.c:53 example2: Msg 3
*/
void example2(void)
{
    klogger_init(-1, KLOGGER_LEVEL_WARNING, KLOGGER_OPTIONS_MULTITHREAD_DEFAULT);

    unsigned msg_idx = 1;
    KLOG_FATAL("First msg\n");
    KLOG_CRITICAL("Msg %u\n", msg_idx++);
    KLOG_ERROR("Msg %u\n", msg_idx++);
    KLOG_WARNING("Msg %u\n", msg_idx++);
    KLOG_INFO("Msg %u\n", msg_idx++);
    KLOG_DEBUG("Msg %u\n", msg_idx++);
    KLOG_DEBUG2("Msg %u\n", msg_idx++);
    KLOG_DEBUG3("Msg %u\n", msg_idx++);

    klogger_deinit();
}

/*
    Log on stderr only
    Get timestamp
    Enable only FATAL level

    Output:
    [FATAL   ] example/main.c:84 example3: First msg
    Stacktrace:
    ./example.out() [0x4019ee]
    ./example.out() [0x401d90]
    ./example.out() [0x401eb6]
    /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf3) [0x7f021124d0b3]
    ./example.out() [0x4011ee]
*/
void example3(void)
{
    klogger_init(-1, KLOGGER_LEVEL_FATAL, KLOGGER_OPTIONS_STDERR_DUPLICATE);

    unsigned msg_idx = 1;
    KLOG_FATAL("First msg\n");
    KLOG_CRITICAL("Msg %u\n", msg_idx++);
    KLOG_ERROR("Msg %u\n", msg_idx++);
    KLOG_WARNING("Msg %u\n", msg_idx++);
    KLOG_INFO("Msg %u\n", msg_idx++);
    KLOG_DEBUG("Msg %u\n", msg_idx++);
    KLOG_DEBUG2("Msg %u\n", msg_idx++);
    KLOG_DEBUG3("Msg %u\n", msg_idx++);

    klogger_deinit();
}

/*
    To demonstrate a powerful feature (logging to socket) this example creates another proc

    Log on socket
    Disable all fancy features
    Enable levels <= DEBUG

    Output:
    [FATAL   ] example/main.c:156 example4: First msg
    Stacktrace:
    ./example.out() [0x401a9e]
    ./example.out() [0x401fec]
    ./example.out() [0x402206]
    /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf3) [0x7fa5b16d60b3]
    ./example.out() [0x40129e]
    [CRITICAL] example/main.c:157 example4: Msg 1
    [ERROR   ] example/main.c:158 example4: Msg 2
    [WARNING ] example/main.c:159 example4: Msg 3
    [INFO    ] example/main.c:160 example4: Msg 4
    [DEBUG   ] example/main.c:161 example4: Msg 5
*/
void example4(void)
{
    /* Child, lets say that this is program which reads logs */
    if (fork() == 0)
    {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        struct sockaddr_in address = {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(6666)};
        bind(server_fd, (struct sockaddr *)&address, sizeof(address));
        listen(server_fd, 1);

        int logs_socket = accept(server_fd, (struct sockaddr *)&address, &(socklen_t){sizeof(address)});
        char buffer[1024] = {0};

        ssize_t bytes;
        while ((bytes = read(logs_socket, buffer, sizeof(buffer))) > 0)
        {
            buffer[bytes] = '\0';
            printf("%s", buffer);
        }

        close(logs_socket);
        close(server_fd);
    }
    else /* Parent, he have some program to execute and logger started */
    {
        int log_socket = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_addr.s_addr = inet_addr("127.0.0.1"), .sin_port = htons(6666)};
        connect(log_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

        klogger_init(log_socket, KLOGGER_LEVEL_DEBUG, 0);

        unsigned msg_idx = 1;
        KLOG_FATAL("First msg\n");
        KLOG_CRITICAL("Msg %u\n", msg_idx++);
        KLOG_ERROR("Msg %u\n", msg_idx++);
        KLOG_WARNING("Msg %u\n", msg_idx++);
        KLOG_INFO("Msg %u\n", msg_idx++);
        KLOG_DEBUG("Msg %u\n", msg_idx++);
        KLOG_DEBUG2("Msg %u\n", msg_idx++);
        KLOG_DEBUG3("Msg %u\n", msg_idx++);

        klogger_deinit();
        close(log_socket);
    }
}

int main(void)
{
    example1();
    example2();
    example3();
    example4();
    return 0;
}

````

#### Contact
email: michalkukowski10@gmail.com
