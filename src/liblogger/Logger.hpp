#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
    #include <cstdio>
#else
    #include <stdio.h>
#endif

#ifdef __cplusplus
namespace logger
{
#endif

#define LOG_EXIT_FAILURE -1
#define LOG_EXIT_SUCCESS 0
#define LOG_MESSAGE_BUFFER_SIZE 1024

    typedef enum
    {
        logDev_e = 0,
        logDbg_e,
        logInf_e,
        logWrn_e,
        logErr_e,
        logFtl_e,
    } logSeverityLevel_t;

    /**
     * @brief function de-initializes logger (writes all remaining message to stream a release resources)
     *
     * @return int returns LOG_EXIT_SUCCESS on success; LOG_EXIT_FAILURE on failure
     */
    int logDestroy_f(void);

    /**
     * @brief initialize logger for usage
     *
     * @param stream file stream to write messages; if NULL is passed, stdout is used
     * @param level log level; messages with lower level than specified will not be written
     * @return int returns LOG_EXIT_SUCCESS on success; LOG_EXIT_FAILURE value on failure
     */
    int logInitialize_f(FILE* stream, const logSeverityLevel_t level);

    /**
     * @brief write message to log stream
     *
     * @param severity message severity
     * @param line line where message originated
     * @param file file where message originated
     * @param function function where message originated
     * @param format message/format to write
     * @param ... optional arguments for format
     * @return int returns LOG_EXIT_SUCCESS on success; LOG_EXIT_FAILURE value on failure
     */
    int logWriteMessage_f(const logSeverityLevel_t severity,
                            const int line,
                            const char* file,
                            const char* function,
                            const char* format,
                            ...);

    /**
     * @brief set log level during runtime
     *
     * @param level new logging level
     * @return int return LOG_EXIT_SUCCESS on success; LOG_EXIT_FAILURE value on failure
     */
    int logSetLevel_f(const logSeverityLevel_t level);

    /**
     * @brief lock logger stream to perform other operations
     *
     * @return int returns LOG_EXIT_SUCCESS on success; LOG_EXIT_FAILURE value on failure
     */
    int logLockLoggerStream();


    /**
     * @brief unlock logger stream to resume normal loggin operations
     *
     * @return int returns LOG_EXIT_SUCCESS on success; LOG_EXIT_FAILURE value on failure
     */
    int logUnlockLoggerStream();


#ifdef __cplusplus
#define LOG_MSG_DEV(_MSG_) logger::logWriteMessage_f(logger::logDev_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_DBG(_MSG_) logger::logWriteMessage_f(logger::logDbg_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_INF(_MSG_) logger::logWriteMessage_f(logger::logInf_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_WRN(_MSG_) logger::logWriteMessage_f(logger::logWrn_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_ERR(_MSG_) logger::logWriteMessage_f(logger::logErr_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_FTL(_MSG_) logger::logWriteMessage_f(logger::logFtl_e, __LINE__, __FILE__, __func__, _MSG_)

#define LOG_FMT_DEV(_MSG_, ...) logger::logWriteMessage_f(logger::logDev_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_DBG(_MSG_, ...) logger::logWriteMessage_f(logger::logDbg_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_INF(_MSG_, ...) logger::logWriteMessage_f(logger::logInf_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_WRN(_MSG_, ...) logger::logWriteMessage_f(logger::logWrn_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_ERR(_MSG_, ...) logger::logWriteMessage_f(logger::logErr_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_FTL(_MSG_, ...) logger::logWriteMessage_f(logger::logFtl_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#else
#define LOG_MSG_DEV(_MSG_) logWriteMessage_f(logDev_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_DBG(_MSG_) logWriteMessage_f(logDbg_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_INF(_MSG_) logWriteMessage_f(logInf_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_WRN(_MSG_) logWriteMessage_f(logWrn_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_ERR(_MSG_) logWriteMessage_f(logErr_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_FTL(_MSG_) logWriteMessage_f(logFtl_e, __LINE__, __FILE__, __func__, _MSG_)

#define LOG_FMT_DEV(_MSG_, ...) logWriteMessage_f(logDev_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_DBG(_MSG_, ...) logWriteMessage_f(logDbg_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_INF(_MSG_, ...) logWriteMessage_f(logInf_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_WRN(_MSG_, ...) logWriteMessage_f(logWrn_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_ERR(_MSG_, ...) logWriteMessage_f(logErr_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#define LOG_FMT_FTL(_MSG_, ...) logWriteMessage_f(logFtl_e, __LINE__, __FILE__, __func__, _MSG_, ##__VA_ARGS__)
#endif
    #ifdef __cplusplus
} // namespace logger
        #endif
#endif

