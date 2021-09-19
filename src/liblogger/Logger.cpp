#include "Logger.hpp"

#ifdef __cplusplus
    #include <cstdlib>
    #include <cstdarg>
    #include <cstdio>
    #include <cstring>
#else
    #include <stdlib.h>
    #include <stdarg.h>
    #include <stdio.h>
    #include <string.h>
#endif

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef __cplusplus
namespace logger
{
#endif
    typedef enum
    {
        False_e,
        True_e,
    } bool_t;

    typedef struct logMessage
    {
        /* pointer to next message in  queue*/
        struct logMessage* next;
        /* message severity level */
        logSeverityLevel_t severity;
        /* name of file where message  originated */
        const char* file;
        /* name  of function  where message originated */
        const char* function;
        /* line of code where message originated */
        int line;
        /* system thread id where message originated */
        long threadId;
        /* system time of message */
        time_t time;
        /* message */
        char buffer[LOG_MESSAGE_BUFFER_SIZE];
    } logMessage_t;

    typedef struct
    {
        /* output stream */
        FILE* stream;
        /* logging level */
        logSeverityLevel_t level;
        /* flag if logger is initialized */
        bool_t isInitialized;
        /*flag that indicates that logger thread  should run */
        bool_t runLogThread;
        /* logger thread identifier */
        pthread_t logThread;
        /* message queue lock */
        pthread_mutex_t queueLock;
        /* log write  condition  variable */
        pthread_cond_t logCondition;
        /* actual queue length */
        int queueLength;
        /* first message in log queue */
        logMessage_t* first;
        /* last message in  log queue */
        logMessage_t* last;
    } logData_t;

    /* log data lock */
    static pthread_mutex_t loggerDataLock = PTHREAD_MUTEX_INITIALIZER;
    static pthread_mutex_t streamLock = PTHREAD_MUTEX_INITIALIZER;

    /* log data instance */
    static logData_t loggerData;


    /* set prototypes with attribues */
    inline int lockLoggerData(void) __attribute__((always_inline));
    inline int unlockLoggerData(void) __attribute__((always_inline));
    inline int lockLoggerQueue(void) __attribute__((always_inline));
    inline int unlockLoggerQueue(void) __attribute__((always_inline));
    inline int lockStream(void) __attribute__((always_inline));
    inline int unlockStream(void) __attribute__((always_inline));
    inline logMessage_t* popMessage() __attribute__((always_inline));

    /*=====================================================================*/
    inline int lockLoggerData(void)
    {
        int rc;

        if ((rc = pthread_mutex_lock(&loggerDataLock)) != 0)
        {
            perror("FTL unable to lock logger data mutex");
        }

        return rc;
    }

    /*=====================================================================*/
    inline int unlockLoggerData(void)
    {
        int rc;

        if ((rc = pthread_mutex_unlock(&loggerDataLock)) != 0)
        {
            perror("FTL unable to unlock logger data mutex");
        }

        return rc;
    }

    /*=====================================================================*/
    inline int lockLoggerQueue(void)
    {
        int rc;

        if ((rc = pthread_mutex_lock(&loggerData.queueLock)) != 0)
        {
            perror("FTL unable to lock logger queue mutex");
        }

        return rc;
    }

    /*=====================================================================*/
    inline int unlockLoggerQueue(void)
    {
        int rc;

        if ((rc = pthread_mutex_unlock(&loggerData.queueLock)) != 0)
        {
            perror("FTL unable to unlock logger queue mutex");
        }

        return rc;
    }

    /*=====================================================================*/
    inline int lockStream(void)
    {
        int rc;

        if ((rc = pthread_mutex_lock(&streamLock)) != 0)
        {
            perror("FTL unable to lock logger stream mutex");
        }

        return rc;
    }

    /*=====================================================================*/
    inline int unlockStream(void)
    {
        int rc;

        if ((rc = pthread_mutex_unlock(&streamLock)) != 0)
        {
            perror("FTL unable to unlock logger stream mutex");
        }

        return rc;
    }

    /*=====================================================================*/
    int logSetLevel_f(const logSeverityLevel_t level)
    {
        if (lockLoggerData() != 0)
        {
            return LOG_EXIT_FAILURE;
        }

        loggerData.level = level;

        if (unlockLoggerData() != 0)
        {
            return LOG_EXIT_FAILURE;
        }

        return LOG_EXIT_SUCCESS;
    }

    /*=====================================================================*/
    inline logMessage_t* popMessage()
    {
        /* lock message queue; on failure return NULL */
        if (lockLoggerQueue() != 0)
        {
            return  NULL;
        }

        /* if message queue is empty wait for new message */
        if (loggerData.queueLength == 0)
        {
            if (pthread_cond_wait(&loggerData.logCondition, &loggerData.queueLock) != 0)
            {
                perror("FTL failed to wake up logger thread from condition wait");
            }
        }

        /* if the thread is woken up with no message in queue */
        if (loggerData.queueLength == 0)
        {
            unlockLoggerQueue();
            return NULL;
        }

        /* take next message from queue */
        logMessage_t* message = loggerData.first;

        /* if there is only one message in queue */
        if (loggerData.queueLength == 1)
        {
            loggerData.first = NULL;
            loggerData.last = NULL;
        }
        else /* more then one message is present */
        {
            loggerData.first = loggerData.first->next;
        }

        loggerData.queueLength--;

        /* unlock queue and return extracted message */
        unlockLoggerQueue();
        return message;
    }

    /*=====================================================================*/
    int logWriteMessage_f(const logSeverityLevel_t severity,
                            const int line,
                            const char* file,
                            const char* function,
                            const char* format,
                            ...)
    {

        /* lock logger data */
        if (lockLoggerData() != 0)
        {
            return LOG_EXIT_FAILURE;
        }

        /* check if logger is initialized */
        if (loggerData.isInitialized == False_e)
        {
            unlockLoggerData();
            return LOG_EXIT_FAILURE;
        }

        /* filter out messages under specified level */
        if (severity < loggerData.level)
        {
            unlockLoggerData();
            return LOG_EXIT_SUCCESS;
        }

        /* unlock logger data*/
        if (unlockLoggerData() != 0)
        {
            return LOG_EXIT_FAILURE;
        }

        /* allocate memory for new message */
        logMessage_t* newMessage = (logMessage_t*) calloc(1, sizeof(logMessage_t));

        if (newMessage == NULL)
        {
            perror("FTL unable to allocate log message");
            return LOG_EXIT_FAILURE;
        }

        /* set message data */
        newMessage->file = file;
        newMessage->function = function;
        newMessage->line = line;
        newMessage->severity = severity;
        newMessage->threadId = syscall(SYS_gettid);
        newMessage->time = time(NULL);
        va_list args;
        va_start(args,  format);
        vsnprintf(newMessage->buffer, LOG_MESSAGE_BUFFER_SIZE, format, args);
        va_end(args);

        /* lock message queue */
        if (lockLoggerQueue() != 0)
        {
            free(newMessage);
            return LOG_EXIT_FAILURE;
        }

        /* if queue is empty */
        if (loggerData.queueLength == 0)
        {
            loggerData.first = newMessage;
            loggerData.last = newMessage;
        }
        else /* queue not empty  */
        {
            loggerData.last->next = newMessage;
            loggerData.last = newMessage;
        }

        loggerData.queueLength++;

        int rc = LOG_EXIT_SUCCESS;

        /* send signal to thread that new message is  present */
        if (pthread_cond_signal(&loggerData.logCondition) != 0)
        {
            perror("FTL unable to send condition signal to logger thread");
            rc = LOG_EXIT_FAILURE;
        }

        /* unlock message queue */
        if (unlockLoggerQueue() != 0)
        {
            rc = LOG_EXIT_FAILURE;
        }

        return rc;
    }

    /*===================================================================*/
    void writeMessage(FILE* stream, logMessage_t* message)
    {
        /* get formated time */
        char timeString[20] =
        {
            'Y', 'Y', 'Y', 'Y', '-', 'M', 'M', '-', 'D', 'D', '_',
            'H', 'H', ':', 'M', 'M', ':', 'S', 'S', '\0'
        };

        struct tm result;

        if (strftime(timeString, 20, "%Y-%m-%d_%H:%M:%S", localtime_r(&message->time, &result)) == 0)
        {
            /*  on error fill the string with invalid but readable data */
            timeString[0] = 'Y';
            timeString[1] = 'Y';
            timeString[2] = 'Y';
            timeString[3] = 'Y';
            timeString[4] = '-';
            timeString[5] = 'M';
            timeString[6] = 'M';
            timeString[7] = '-';
            timeString[8] = 'D';
            timeString[9] = 'D';
            timeString[10] = '_';
            timeString[11] = 'H';
            timeString[12] = 'H';
            timeString[13] = ':';
            timeString[14] = 'M';
            timeString[15] = 'M';
            timeString[16] = ':';
            timeString[17] = 'S';
            timeString[18] = 'S';
            timeString[19] = '\0';
        }

        /* get file name for message footer */
        const char* trimmedFile = message->file;

        if (trimmedFile != NULL)
        {
            trimmedFile = strrchr(trimmedFile, '/');

            if (trimmedFile == NULL)
            {
                trimmedFile = message->file;
            }
            else
            {
                if (strlen(trimmedFile) != 1)
                {
                    trimmedFile++;
                }
            }
        }

        /* write message header - severity tag + time */
        int rc(0);
        lockStream();

        switch (message->severity)
        {
            case logDev_e:
                rc = fprintf(stream, "    [DEV] : %s : %s : [%d/%d/%ld %s:%d:%s()]\n", timeString, message->buffer, getppid(), getpid(), message->threadId, trimmedFile, message->line, message->function);
                break;

            case logDbg_e:
                rc = fprintf(stream, "    [DBG] : %s : %s [%d/%d/%ld %s:%d:%s()]\n", timeString, message->buffer, getppid(), getpid(), message->threadId, trimmedFile, message->line, message->function);
                break;

            case logInf_e:
                rc = fprintf(stream, "--- [INF] : %s : %s [%d/%d/%ld %s:%d:%s()]\n", timeString, message->buffer, getppid(), getpid(), message->threadId, trimmedFile, message->line, message->function);
                break;

            case logWrn_e:
                rc = fprintf(stream, "+++ [WRN] : %s : %s [%d/%d/%ld %s:%d:%s()]\n", timeString, message->buffer, getppid(), getpid(), message->threadId, trimmedFile, message->line, message->function);
                break;

            case logErr_e:
                rc = fprintf(stream, "!!! [ERR] : %s : %s [%d/%d/%ld %s:%d:%s()]\n", timeString, message->buffer, getppid(), getpid(), message->threadId, trimmedFile, message->line, message->function);
                break;

            case logFtl_e:
                rc = fprintf(stream, "XXX [FTL] : %s : %s [%d/%d/%ld %s:%d:%s()]\n", timeString, message->buffer, getppid(), getpid(), message->threadId, trimmedFile, message->line, message->function);
                break;
        }

        unlockStream();

        if (rc < 0)
        {
            perror("FTL error during writing log message header");
        }

        /* write stream data */
        if (fflush(stream) != 0)
        {
            perror("FTL error during flushing file stream");
        }
    }

    /*===================================================================*/
    void* loggerThreadBody(void*)
    {
        for (;;)
        {
            /* get shared information on one lock/unlock */
            if (lockLoggerData() != 0)
            {
                continue;
            }

            bool_t isInitialized = loggerData.isInitialized;
            bool_t runThread = loggerData.runLogThread;
            FILE* stream = loggerData.stream;

            unlockLoggerData();

            /* determine if thread should run; if not break the loop */
            if (isInitialized == False_e)
            {
                break;
            }

            /* determine if thread should run; if not break the loop */
            if (runThread == False_e)
            {
                break;
            }

            /* wait for new message; if no message returned, run next loop */
            logMessage_t* message = popMessage();

            if (message == NULL)
            {
                continue;
            }

            /* write message */
            writeMessage(stream, message);

            /* destroy message  */
            free(message);
        }

        return NULL;
    }

    /*=====================================================================*/
    int logDestroy_f(void)
    {
        /* lock logger data */
        if (lockLoggerData() != 0)
        {
            return LOG_EXIT_FAILURE;
        }

        /* logger was never initialzed; unlock mutex and exit */
        if (loggerData.isInitialized == False_e)
        {
            unlockLoggerData();
            return LOG_EXIT_FAILURE;
        }

        /* set log processing flag to false */
        loggerData.runLogThread = False_e;

        /* lock logger data */
        unlockLoggerData();

        int rc = LOG_EXIT_SUCCESS;

        /* signal the thread in case it is waiting on send condition */
        if (pthread_cond_signal(&loggerData.logCondition) != 0)
        {
            perror("failed to send condition signal on exit");
            rc = LOG_EXIT_FAILURE;
        }

        /* wait for thread to finish */
        if (pthread_join(loggerData.logThread,  NULL) != 0)
        {
            perror("logger thread failed to join");
            rc = LOG_EXIT_FAILURE;
        }

        /* release condition variable */
        if (pthread_cond_destroy(&loggerData.logCondition) != 0)
        {
            perror("failed to destroy log condition");
            rc = LOG_EXIT_FAILURE;
        }

        /* release queue mutex */
        if (pthread_mutex_destroy(&loggerData.queueLock) != 0)
        {
            perror("failed to destroy queue mutex");
            rc = LOG_EXIT_FAILURE;
        }

        /* write all remaining messages in queue */
        while (loggerData.first != NULL)
        {
            logMessage_t* toDelete = loggerData.first;
            writeMessage(loggerData.stream, toDelete);
            loggerData.first = loggerData.first->next;
            free(toDelete);
        }

        loggerData.first = NULL;
        loggerData.last = NULL;
        loggerData.queueLength = 0;

        /* flush stream */
        if (fflush(loggerData.stream) != 0)
        {
            perror("failed  to flush log stream");
            rc = LOG_EXIT_FAILURE;
        }

        /* close stream */
        if (fclose(loggerData.stream) != 0)
        {
            perror("failed to close log stream");
            rc = LOG_EXIT_FAILURE;
        }

        loggerData.isInitialized = False_e;

        /* unlock logger data */
        if (unlockLoggerData() != 0)
        {
            rc = LOG_EXIT_FAILURE;
        }

        return rc;
    }

    /*=====================================================================*/
    int logInitialize_f(FILE* stream, const logSeverityLevel_t level)
    {
        /* lock logger data */
        if (lockLoggerData() != 0)
        {
            return LOG_EXIT_FAILURE;
        }

        /* do not initialize  twice or more */
        if (loggerData.isInitialized == True_e)
        {
            perror("WRN logger already initialized (log-init)");
            unlockLoggerData();
            return LOG_EXIT_FAILURE;
        }

        /* output stream */
        if (stream == NULL)
        {
            loggerData.stream = stdout;
        }
        else
        {
            loggerData.stream = stream;
        }

        /* logging level */
        loggerData.level = level;

        /* actual queue length */
        loggerData.queueLength = 0;

        /* first message in log queue */
        loggerData.first = NULL;

        /* last message in  log queue */
        loggerData.last = NULL;

        /* message queue lock */
        if (pthread_mutex_init(&loggerData.queueLock, NULL) != 0)
        {
            perror("FTL unable to create queue lock");
            fclose(loggerData.stream);
            unlockLoggerData();
            return LOG_EXIT_FAILURE;
        }

        /* log write condition variable */
        if (pthread_cond_init(&loggerData.logCondition, NULL))
        {
            perror("FTL unable to create queue condition");
            fclose(loggerData.stream);
            pthread_mutex_destroy(&loggerData.queueLock);
            unlockLoggerData();
            return LOG_EXIT_FAILURE;
        }

        /*flag that indicates that logger thread should run */
        loggerData.runLogThread = True_e;

        /* logger thread identifier */
        if (pthread_create(&loggerData.logThread, NULL, loggerThreadBody, NULL) != 0)
        {
            perror("FTL unable to create logger thread");
            fclose(loggerData.stream);
            pthread_mutex_destroy(&loggerData.queueLock);
            pthread_cond_destroy(&loggerData.logCondition);
            return LOG_EXIT_FAILURE;
        }

        /* flag if logger is initialized */
        loggerData.isInitialized = True_e;

        /* finally unlock logger data */
        if (unlockLoggerData() != 0)
        {
            /* this is very bad situation right here; destroy all we can and try again to unlock */
            loggerData.runLogThread = False_e;
            loggerData.isInitialized = False_e;
            fclose(loggerData.stream);
            pthread_mutex_destroy(&loggerData.queueLock);
            pthread_cond_destroy(&loggerData.logCondition);
            unlockLoggerData();
            return LOG_EXIT_FAILURE;
        }

        return LOG_EXIT_SUCCESS;
    }

    int logLockLoggerStream()
    {
        return lockStream();
    }

    int logUnlockLoggerStream()
    {
        return unlockStream();
    }

    #ifdef __cplusplus
} // namespace logger
    #endif