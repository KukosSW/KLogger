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
    KLOG_FATAL("First msg");
    KLOG_CRITICAL("Msg %u", msg_idx++);
    KLOG_ERROR("Msg %u", msg_idx++);
    KLOG_WARNING("Msg %u", msg_idx++);
    KLOG_INFO("Msg %u", msg_idx++);
    KLOG_DEBUG("Msg %u", msg_idx++);
    KLOG_DEBUG2("Msg %u", msg_idx++);
    KLOG_DEBUG3("Msg %u", msg_idx++);

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
    KLOG_FATAL("First msg");
    KLOG_CRITICAL("Msg %u", msg_idx++);
    KLOG_ERROR("Msg %u", msg_idx++);
    KLOG_WARNING("Msg %u", msg_idx++);
    KLOG_INFO("Msg %u", msg_idx++);
    KLOG_DEBUG("Msg %u", msg_idx++);
    KLOG_DEBUG2("Msg %u", msg_idx++);
    KLOG_DEBUG3("Msg %u", msg_idx++);

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
    KLOG_FATAL("First msg");
    KLOG_CRITICAL("Msg %u", msg_idx++);
    KLOG_ERROR("Msg %u", msg_idx++);
    KLOG_WARNING("Msg %u", msg_idx++);
    KLOG_INFO("Msg %u", msg_idx++);
    KLOG_DEBUG("Msg %u", msg_idx++);
    KLOG_DEBUG2("Msg %u", msg_idx++);
    KLOG_DEBUG3("Msg %u", msg_idx++);

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
        KLOG_FATAL("First msg");
        KLOG_CRITICAL("Msg %u", msg_idx++);
        KLOG_ERROR("Msg %u", msg_idx++);
        KLOG_WARNING("Msg %u", msg_idx++);
        KLOG_INFO("Msg %u", msg_idx++);
        KLOG_DEBUG("Msg %u", msg_idx++);
        KLOG_DEBUG2("Msg %u", msg_idx++);
        KLOG_DEBUG3("Msg %u", msg_idx++);

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
