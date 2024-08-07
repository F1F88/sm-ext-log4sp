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


/**
 * Log level enum
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
 * Pattern time - specific time getting to use for pattern_formatter.
 * local time by default
 */
enum PatternTimeType
{
    PatternTimeType_Local,              // log localtime
    PatternTimeType_Utc                 // log utc
};

/**
 * Async overflow policy - block by default.
 */
enum AsyncOverflowPolicy
{
    AsyncOverflowPolicy_Block,          // Block until message can be enqueued.
    AsyncOverflowPolicy_OverrunOldest,  // Discard oldest message in the queue if full when trying to add new item.
    AsyncOverflowPolicy_DiscardNew      // Discard new message if the queue is full when trying to add new item.
}

/**
 * Convert a logging level enum to name.
 *
 * @note The native will log a message and:
 *       warn - If lvl out of bounds - Fix lvl to nearest bounds.
 *
 * @param lvl       Logging level enum.
 * @param buffer    Buffer to store level name.
 * @param maxlen    Maximum size of string buffer.
 */
native void LogLevelToName(LogLevel lvl, char[] buffer, int maxlen);

/**
 * Convert a logging level enum to short name.
 *
 * @note The native will log a message and:
 *       warn - If lvl out of bounds - Fix lvl to nearest bounds.
 *
 * @param lvl       Logging level enum.
 * @param buffer    Buffer to store level name.
 * @param maxlen    Maximum size of string buffer.
 */
native void LogLevelToShortName(LogLevel lvl, char[] buffer, int maxlen);

/**
 * Convert a logging level name to level enum.
 *
 * @note The native will log a message and:
 *       warn - If there is no level with this name - Return LogLevel_Off.
 *
 * @param name      Log level name.
 * @return          Logging level enum.
 */
native LogLevel NameToLogLevel(const char[] name);
