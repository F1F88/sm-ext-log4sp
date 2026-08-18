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
 * Called before opening the file.
 */
typedef FileOpenFunction = function void (const char[] filename);

/**
 * Called after closing the file.
 */
typedef FileCloseFunction = function void (const char[] filename);

/**
 * Callback function for user defined error handler.
 *
 * @param origin    The origin name of the error.
 * @param loc       The source location of the error.
 * @param msg       The details message of the error.
 */
typedef ErrorHandler = function void (const char[] origin, const SourceLoc loc, const char[] msg);


/**
 * Convert a log level enum to name.
 *
 * @param buffer    Buffer to store log level name.
 * @param maxlen    Maximum length of the buffer.
 * @param lvl       Log level enum.
 * @return          Number of bytes written.
 */
stock int LogLevelToName(char[] buffer, int maxlen, LogLevel lvl)
{
    switch(lvl)
    {
        case LogLevel_Trace : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_TRACE);
        case LogLevel_Debug : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_DEBUG);
        case LogLevel_Info  : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_INFO);
        case LogLevel_Warn  : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_WARN);
        case LogLevel_Error : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_ERROR);
        case LogLevel_Fatal : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_FATAL);
        default             : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_NAME_OFF);
    }
}

/**
 * Convert a log level enum to short name.
 *
 * @param buffer    Buffer to store log level short name.
 * @param maxlen    Maximum length of the buffer.
 * @param lvl       Log level enum.
 * @return          Number of bytes written.
 */
stock int LogLevelToShortName(char[] buffer, int maxlen, LogLevel lvl)
{
    switch(lvl)
    {
        case LogLevel_Trace : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_TRACE);
        case LogLevel_Debug : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_DEBUG);
        case LogLevel_Info  : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_INFO);
        case LogLevel_Warn  : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_WARN);
        case LogLevel_Error : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_ERROR);
        case LogLevel_Fatal : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_FATAL);
        default             : return FormatEx(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_OFF);
    }
}

/**
 * Convert a log level name to log level enum.
 *
 * @param name      Log level name.
 * @return          Log level enum or LogLevel_Off when the log level name does not exist.
 */
stock LogLevel NameToLogLevel(const char[] name)
{
    if (StrEqual(name, LOG4SP_LEVEL_NAME_TRACE)) return LogLevel_Trace;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_DEBUG)) return LogLevel_Debug;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_INFO))  return LogLevel_Info;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_WARN))  return LogLevel_Warn;
    if (StrEqual(name, "warning"))               return LogLevel_Warn;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_ERROR)) return LogLevel_Error;
    if (StrEqual(name, "err"))                   return LogLevel_Error;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_FATAL)) return LogLevel_Fatal;
    if (StrEqual(name, "critical"))              return LogLevel_Fatal;
    return LogLevel_Off;
}

/**
 * Convert a log level name to log level enum.
 *
 * @param buildTime Build time.
 * @param maxlen1   Maximum length of the build time.
 * @param buildTags Build tags.
 * @param maxlen2   Maximum length of the tags.
 * @return          numeric representation.
 */
native int GetLog4spVersion(char[] buildTime="", int maxlen1=0, char[] buildTags="", int maxlen2=0);
