#if defined _log4sp_common_included
 #endinput
#endif
#define _log4sp_common_included

#pragma newdecls required
#pragma semicolon 1

#define LOG4SP_LEVEL_TRACE              0
#define LOG4SP_LEVEL_DEBUG              1
#define LOG4SP_LEVEL_INFO               2
#define LOG4SP_LEVEL_WARN               3
#define LOG4SP_LEVEL_ERROR              4
#define LOG4SP_LEVEL_FATAL              5
#define LOG4SP_LEVEL_OFF                6

#define LOG4SP_LEVEL_NAME_TRACE         "trace"
#define LOG4SP_LEVEL_NAME_DEBUG         "debug"
#define LOG4SP_LEVEL_NAME_INFO          "info"
#define LOG4SP_LEVEL_NAME_WARN          "warn"
#define LOG4SP_LEVEL_NAME_ERROR         "error"
#define LOG4SP_LEVEL_NAME_FATAL         "fatal"
#define LOG4SP_LEVEL_NAME_OFF           "off"

#define LOG4SP_LEVEL_SHORT_NAME_TRACE   "T"
#define LOG4SP_LEVEL_SHORT_NAME_DEBUG   "D"
#define LOG4SP_LEVEL_SHORT_NAME_INFO    "I"
#define LOG4SP_LEVEL_SHORT_NAME_WARN    "W"
#define LOG4SP_LEVEL_SHORT_NAME_ERROR   "E"
#define LOG4SP_LEVEL_SHORT_NAME_FATAL   "F"
#define LOG4SP_LEVEL_SHORT_NAME_OFF     "O"

#define LOG4SP_GLOBAL_LOGGER_NAME       "log4sp"


/**
 * Log level enum.
 */
enum LogLevel
{
    LogLevel_Trace = LOG4SP_LEVEL_TRACE,
    LogLevel_Debug = LOG4SP_LEVEL_DEBUG,
    LogLevel_Info  = LOG4SP_LEVEL_INFO,
    LogLevel_Warn  = LOG4SP_LEVEL_WARN,
    LogLevel_Error = LOG4SP_LEVEL_ERROR,
    LogLevel_Fatal = LOG4SP_LEVEL_FATAL,
    LogLevel_Off   = LOG4SP_LEVEL_OFF,
    LogLevel_Total
}

/**
 * Specific time used for pattern formatting.
 */
enum PatternTimeType
{
    PatternTimeType_Local,              // log localtime
    PatternTimeType_Utc                 // log utc
};

/**
 * Source code location.
 */
enum struct SourceLoc
{
    char filename[PLATFORM_MAX_PATH];   // file name/path.
    int line;                           // line number.
    char funcname[PLATFORM_MAX_PATH];   // function name.

    bool IsEmpty()
    {
        return !this.filename[0] || this.line <= 0 || !this.funcname[0];
    }
}


/**
 * Convert a log level enum to name.
 *
 * @param buffer    Buffer to store log level name.
 * @param maxlen    Maximum length of the buffer.
 * @param lvl       Log level enum.
 * @return          Number of characters written to the buffer, not including the null terminator.
 */
native int LogLevelToName(char[] buffer, int maxlen, LogLevel lvl);

/**
 * Convert a log level enum to short name.
 *
 * @param buffer    Buffer to store log level short name.
 * @param maxlen    Maximum length of the buffer.
 * @param lvl       Log level enum.
 * @return          Number of characters written to the buffer, not including the null terminator.
 */
native int LogLevelToShortName(char[] buffer, int maxlen, LogLevel lvl);

/**
 * Convert a log level name to log level enum.
 *
 * @param name      Log level name.
 * @return          Log level enum or LogLevel_Off when the log level name does not exist.
 */
native LogLevel NameToLogLevel(const char[] name);
