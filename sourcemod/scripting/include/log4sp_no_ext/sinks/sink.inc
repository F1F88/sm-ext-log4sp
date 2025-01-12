#if defined _log_for_sourcepawn_sinks_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/common>


methodmap Sink < Handle
{
    public LogLevel GetLevel() { return LogLevel_Off; }

    public void SetLevel(LogLevel lvl) {}

    public void SetPattern(const char[] pattern) {}

    public bool ShouldLog(LogLevel lvl) { return false; }

    public void Log(const char[] name, LogLevel lvl, const char[] msg) {
        switch (lvl) {
            case LogLevel_Info:     LogMessage("["... LOG4SP_LEVEL_NAME_INFO ..."] [%s] %s", name, msg);
            case LogLevel_Warn:     LogMessage("["... LOG4SP_LEVEL_NAME_WARN ..."] [%s] %s", name, msg);
            case LogLevel_Error:    LogError("["... LOG4SP_LEVEL_NAME_ERROR ..."] [%s] %s", name, msg);
            case LogLevel_Fatal:    LogError("["... LOG4SP_LEVEL_NAME_FATAL ..."] [%s] %s", name, msg);
        }
    }

    public void Flush() {}
}

typeset SinkClientFilter
{
    function Action (int client);
};
