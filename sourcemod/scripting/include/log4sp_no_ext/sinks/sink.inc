#if defined _log_for_sourcepawn_sinks_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/common>


methodmap Sink < Handle
{
    public LogLevel GetLevel() {
        return LogLevel_Trace;
    }

    public void SetLevel(LogLevel lvl) {}

    public void SetPattern(const char[] pattern) {}

    public bool ShouldLog(LogLevel lvl) {
        return lvl >= this.GetLevel();
    }

    public void Log(const char[] name, LogLevel lvl, const char[] msg, const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING, int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0}) {
        char loc[PLATFORM_MAX_PATH];
        if (file && line > 0 && func) {
            int sep = FindCharInString(file, '\\', true) + 1;
            sep = sep != 0 ? sep : FindCharInString(file, '/', true) + 1;
            FormatEx(loc, sizeof(loc), "[%s::%d]", file[sep], line);
        }

        switch (lvl) {
            case LogLevel_Trace: LogMessage("[%s] ["... LOG4SP_LEVEL_NAME_TRACE ..."] [%s] %s", name, loc, msg);
            case LogLevel_Debug: LogMessage("[%s] ["... LOG4SP_LEVEL_NAME_DEBUG ..."] [%s] %s", name, loc, msg);
            case LogLevel_Info: LogMessage("[%s] ["... LOG4SP_LEVEL_NAME_INFO ..."] [%s] %s", name, loc, msg);
            case LogLevel_Warn: LogMessage("[%s] ["... LOG4SP_LEVEL_NAME_WARN ..."] [%s] %s", name, loc, msg);
            case LogLevel_Error:LogError("[%s] ["... LOG4SP_LEVEL_NAME_ERROR ..."] [%s] %s", name, loc, msg);
            case LogLevel_Fatal:LogError("[%s] ["... LOG4SP_LEVEL_NAME_FATAL ..."] [%s] %s", name, loc, msg);
            case LogLevel_Off:  LogError("[%s] ["... LOG4SP_LEVEL_NAME_OFF ..."] [%s] %s", name, loc, msg);
        }
    }

    public void Flush() {}
}
