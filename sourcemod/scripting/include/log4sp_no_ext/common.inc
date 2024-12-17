#if defined _log_for_sourcepawn_common_included
 #endinput
#endif
#define _log_for_sourcepawn_common_included

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

#define LOG4SP_DEFAULT_LOGGER_NAME      "log4sp"


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

enum PatternTimeType
{
    PatternTimeType_Local,
    PatternTimeType_Utc
};

enum AsyncOverflowPolicy
{
    AsyncOverflowPolicy_Block,
    AsyncOverflowPolicy_OverrunOldest,
    AsyncOverflowPolicy_DiscardNew
}


stock void LogLevelToName(LogLevel lvl, char[] buffer, int maxlen) {
    switch (lvl) {
        case LogLevel_Trace:    strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_TRACE);
        case LogLevel_Debug:    strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_DEBUG);
        case LogLevel_Info:     strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_INFO);
        case LogLevel_Warn:     strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_WARN);
        case LogLevel_Error:    strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_ERROR);
        case LogLevel_Fatal:    strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_FATAL);
        case LogLevel_Off:      strcopy(buffer, maxlen, LOG4SP_LEVEL_NAME_OFF);
    }
}

stock void LogLevelToShortName(LogLevel lvl, char[] buffer, int maxlen) {
    switch (lvl) {
        case LogLevel_Trace:    strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_TRACE);
        case LogLevel_Debug:    strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_DEBUG);
        case LogLevel_Info:     strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_INFO);
        case LogLevel_Warn:     strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_WARN);
        case LogLevel_Error:    strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_ERROR);
        case LogLevel_Fatal:    strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_FATAL);
        case LogLevel_Off:      strcopy(buffer, maxlen, LOG4SP_LEVEL_SHORT_NAME_OFF);
    }
}

stock LogLevel NameToLogLevel(const char[] name) {
    if (StrEqual(name, LOG4SP_LEVEL_NAME_TRACE))    return LogLevel_Trace;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_DEBUG))    return LogLevel_Debug;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_INFO))     return LogLevel_Info;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_WARN)  || StrEqual(name, "warning"))   return LogLevel_Warn;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_ERROR) || StrEqual(name, "err"))       return LogLevel_Error;
    if (StrEqual(name, LOG4SP_LEVEL_NAME_FATAL) || StrEqual(name, "critical"))  return LogLevel_Fatal;
    return LogLevel_Off;
}
