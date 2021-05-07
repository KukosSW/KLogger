#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <execinfo.h>
#include <stdlib.h>

#include <klogger/klogger.h>

#define CALLSTACK_SIZE_MAX 256

typedef struct KLogger_useroptions
{
    bool stdout_dup:1;      /* Log on stdout or not */
    bool stderr_dup:1;      /* Log on stderr or not */
    bool file_dup:1;        /* Create auto named file and log into it or not */
    bool timestamp:1;       /* Print Time or not */
    bool multithreading:1;  /* Print TID or not */

    klogger_level_t level;  /* Log only levels <= this level, others are skipped */
} KLogger_useroptions;

#define KLOGGER_DATA_MAX_FD (4) /* main fd + stdout dup + stderr dup + file */
typedef struct KLogger_data
{
    bool is_init;                /* Our state machine is simple, INITED or NOT */
    int file_fd;                 /* file descriptor used only for close */
    int fd[KLOGGER_DATA_MAX_FD]; /* all possibled descriptors ([i] == -1 if ith descriptor is unused) */
    const char* directory;       /* directory name with path (not absolute) */
    mtx_t mutex;                 /* Main mutex to make this logger threadsafe */
    KLogger_useroptions options; /* User options parsed from  klogger_level_t and klogger_option_t */
} KLogger_data;
static KLogger_data klogger_priv_data;

/* Keep it in proper order with alignment to biggest one */
static const char* klogger_priv_level_string[] = {"FATAL   ",
                                                  "CRITICAL",
                                                  "ERROR   ",
                                                  "WARNING ",
                                                  "INFO    ",
                                                  "DEBUG   ",
                                                  "DEBUG2  ",
                                                  "DEBUG3  ",
                                                 };

static KLogger_useroptions __klogger_parse_useroptions(int fd, klogger_level_t lvl, klogger_option_t options);

/* Write something to buffer, return number of bytes written into buffer */
static size_t __klogger_write_timestamp(char *buffer, size_t buffer_size);
static size_t __klogger_write_tid(char *buffer, size_t buffer_size);
static size_t __klogger_write_stacktrace(char *buffer, size_t buffer_size);

static KLogger_useroptions __klogger_parse_useroptions(int fd, klogger_level_t lvl, klogger_option_t options)
{
    return (KLogger_useroptions)
        {
            .level           = lvl,
            /* Cannot duplicate stdout, if user have chosen stdout as a main fd */
            .stdout_dup      = (options & KLOGGER_OPTIONS_STDOUT_DUPLICATE) && fd != 1,
            /* Cannot duplicate stderr, if user have chosen stderr as a main fd */
            .stderr_dup      = (options & KLOGGER_OPTIONS_STDERR_DUPLICATE) && fd != 2,
            .file_dup        = options & KLOGGER_OPTIONS_FILE_DUPLICATE,
            .timestamp       = options & KLOGGER_OPTIONS_USE_TIMESTAMP,
            .multithreading  = options & KLOGGER_OPTIONS_USE_THREADID
        };
}

static size_t __klogger_write_timestamp(char *buffer, size_t buffer_size)
{
    size_t bytes_written = 0;

    /* h:min:sec can be obtanined from localtime, but we need a usec too, use gettimeofday */
    struct timeval timeval_now;
    gettimeofday(&timeval_now, NULL);

    /* Write h:min:sec */
    const struct tm tm_time = *localtime(&(time_t){timeval_now.tv_sec});
    bytes_written += strftime(&buffer[bytes_written], buffer_size - bytes_written, "[%H:%M:%S", &tm_time);

    /* add.usec manually */
    bytes_written += (size_t)snprintf(&buffer[bytes_written], buffer_size - bytes_written, ".%06ld] ", (long)timeval_now.tv_usec);

    return bytes_written;
}

static size_t __klogger_write_tid(char *buffer, size_t buffer_size)
{
    pid_t id = (pid_t)syscall(__NR_gettid);

    return (size_t)snprintf(buffer, buffer_size, "[TID: %ld] ", (long)id);
}

static size_t __klogger_write_stacktrace(char *buffer, size_t buffer_size)
{
    size_t bytes_written = 0;
    void* callstack[CALLSTACK_SIZE_MAX];
    int frames = backtrace(callstack, CALLSTACK_SIZE_MAX);
    if (frames < 0)
        return 0;

    char** strs = backtrace_symbols(callstack, frames);
    if (strs == NULL)
        return 0;

    bytes_written += (size_t)snprintf(&buffer[bytes_written], buffer_size - bytes_written, "Stacktrace:\n");
    for (int i = 0; i < frames; ++i)
        bytes_written += (size_t)snprintf(&buffer[bytes_written], buffer_size - bytes_written, "%s\n", strs[i]);

    free(strs);

    return bytes_written;
}


int klogger_init(int fd, klogger_level_t lvl, klogger_option_t options)
{
    if (klogger_priv_data.is_init)
    {
        perror("Klogger: Please init klogger only once");
        return 1;
    }

    /* Init CONST data */
    klogger_priv_data.directory = "./klogger_logs";

    /* INIT independend framework data */
    if (mtx_init(&klogger_priv_data.mutex, mtx_plain) != thrd_success)
    {
        perror("Klogger: mtx_init error");
        return 1;
    }

    /* INIT logger as a customized logger for user */
    klogger_priv_data.options = __klogger_parse_useroptions(fd, lvl, options);

    /* By default, incorrect fd is have -1 value */
    memset(&klogger_priv_data.fd[0], -1, sizeof(klogger_priv_data.fd));
    klogger_priv_data.file_fd = -1;

    /* Write down all correct descriptors */
    size_t fd_idx = 0;
    if (fd > 0)
        klogger_priv_data.fd[fd_idx++] = fd;

    if (klogger_priv_data.options.stdout_dup)
        klogger_priv_data.fd[fd_idx++] = 1;

    if (klogger_priv_data.options.stderr_dup)
        klogger_priv_data.fd[fd_idx++] = 2;

    /* Nothing to do for klogger, no fd + no file = no work for klogger :) */
    if (fd_idx == 0 && !klogger_priv_data.options.file_dup)
    {
        perror("Klogger: Nothing to do for klogger, please add fd or file");
        return 1;
    }

    /* Create file for logging */
    if (klogger_priv_data.options.file_dup)
    {
        /* create directory first if does not exist */
        if (stat(klogger_priv_data.directory, &(struct stat){0}) == -1)
			if (mkdir(klogger_priv_data.directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
			{
				perror("Klogger: mkdir error");
				return 1;
			}

        /* File based on time in seconds, so in case of a few fast runs file creation will fail */
        unsigned try = 0;
        const unsigned max_tries = 10;
        do {
            /* get local time */
            const struct tm tm_time = *localtime(&(time_t){time(NULL)});

            /* Create a file name in format: directory/YearMonthDay-HourMinuteSecond.log */
            char file_name[256] = {0};
            const size_t directory_name_len = strlen(klogger_priv_data.directory);
            memcpy(&file_name[0], klogger_priv_data.directory, directory_name_len);

            if (strftime(&file_name[directory_name_len], sizeof(file_name), "/%Y%m%d-%H%M%S.log", &tm_time) == 0)
            {
                perror("Klogger: strftime error");
                return 1;
            }

            /* file exists, so someone runs his app faster than expected */
            if (stat(&file_name[0], &(struct stat){0}) != -1)
            {
                try++;
                fprintf(stderr,
                        "Klogger: File %s exists, waiting for another try (%u/%u)\n",
                        &file_name[0],
                        try,
                        max_tries);

                /* After half tries lets wait 1s */
                if (try > max_tries / 2)
                    sleep(1);

                continue;
            }

            /* file created, this will finish do while loop */
            try = max_tries;

            /* Create file */
            if (creat(&file_name[0], S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH) == 0)
            {
                perror("Klogger: creat error");
                return 1;
            }

            /* Finally file is created, we can open it */
            klogger_priv_data.file_fd = open(&file_name[0], O_RDWR | O_TRUNC);
            if (klogger_priv_data.file_fd == -1)
            {
                perror("Klogger: open error");
                return -1;
            }

            klogger_priv_data.fd[fd_idx++] = klogger_priv_data.file_fd;
        } while (max_tries < 10);
    }

    klogger_priv_data.is_init = true;

    return 0;
}

void klogger_deinit(void)
{
    /* close file */
    if (klogger_priv_data.file_fd != -1)
        close(klogger_priv_data.file_fd);

    /* destroy mutex */
    mtx_destroy(&klogger_priv_data.mutex);

    klogger_priv_data.is_init = false;
}


void __attribute__(( format(printf, 5, 6) )) __klogger_print(const char* file,
                                                             const char* func,
                                                             int line,
                                                             klogger_level_t level,
                                                             const char* fmt,
                                                             ...)
{
    if (!klogger_priv_data.is_init)
    {
        /* Show this message only once */
        static bool printed = false;
        if (!printed)
            fprintf(stderr, "Klogger: Please init klogger before use\n");
        printed = true;

        return;
    }

    /* Cannot change lvl during work, so reading by a few threads is ok! */
    if (klogger_priv_data.options.level < level)
        return;

    if (mtx_lock(&klogger_priv_data.mutex) != thrd_success)
    {
        perror("Klogger: mtx_lock error");
        return;
    }

    /* klogger is thread safe, so we can use static buffer here */
    static char buffer[1 << 20] = {0};
    size_t buffer_index = 0;

    buffer_index = (size_t)snprintf(&buffer[0], sizeof(buffer) - buffer_index, "[%s] ", klogger_priv_level_string[level]);

    /* Add timestamp if needed. Format: h:min:sec.usec */
    if (klogger_priv_data.options.timestamp)
        buffer_index += __klogger_write_timestamp(&buffer[buffer_index], sizeof(buffer) - buffer_index);

    /* Add threadID if needed. */
    if (klogger_priv_data.options.multithreading)
        buffer_index += __klogger_write_tid(&buffer[buffer_index], sizeof(buffer) - buffer_index);

    /* Add file line and func */
    buffer_index += (size_t)snprintf(&buffer[buffer_index], sizeof(buffer) - buffer_index, "%s:%d %s: ", file, line, func);

    /* Add user message */
    va_list args;
    va_start(args, fmt);

    buffer_index += (size_t)vsnprintf(&buffer[buffer_index], sizeof(buffer) - buffer_index, fmt, args);

    va_end(args);

    /* User has forgotten new line add for him */
    if (buffer[buffer_index - 1] != '\n' && buffer_index < sizeof(buffer) - 1)
    {
        buffer[buffer_index++] = '\n';
        buffer[buffer_index] = '\0';
    }

    /* FATAL, user should close app, log stacktrace */
    if (level == KLOGGER_LEVEL_FATAL)
        buffer_index += __klogger_write_stacktrace(&buffer[buffer_index], sizeof(buffer) - buffer_index);

    /* buffer created, write into all valid descriptors */
    for (size_t i = 0; i < KLOGGER_DATA_MAX_FD; ++i)
        if (klogger_priv_data.fd[i] > 0)
            dprintf(klogger_priv_data.fd[i], "%s", &buffer[0]);

    mtx_unlock(&klogger_priv_data.mutex);
}
