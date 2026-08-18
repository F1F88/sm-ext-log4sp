#if defined _log4sp_included
 #endinput
#endif
#define _log4sp_included

#pragma newdecls required
#pragma semicolon 1


#include <log4sp/common>
#include <log4sp/logger>
#include <log4sp/sinks/sink>
#include <log4sp/sinks/basic_file_sink>
#include <log4sp/sinks/callback_sink>
#include <log4sp/sinks/client_chat_all_sink>
#include <log4sp/sinks/client_console_all_sink>
#include <log4sp/sinks/daily_file_sink>
#include <log4sp/sinks/ringbuffer_sink>
#include <log4sp/sinks/rotating_file_sink>
#include <log4sp/sinks/server_console_sink>








/**
 * Do not edit below this line!
 */
public Extension __ext_log4sp =
{
    name = "Log4sp",
    file = "log4sp.ext",
#if defined AUTOLOAD_EXTENSIONS
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};


#if !defined REQUIRE_EXTENSIONS
public void __ext_log4sp_SetNTVOptional()
{
    MarkNativeAsOptional("LogLevelToName");
    MarkNativeAsOptional("LogLevelToShortName");
    MarkNativeAsOptional("NameToLogLevel");
    MarkNativeAsOptional("GetLog4spVersion");

    MarkNativeAsOptional("Logger.Logger");
    MarkNativeAsOptional("Logger.GetName");
    MarkNativeAsOptional("Logger.GetNameLength");
    MarkNativeAsOptional("Logger.GetLevel");
    MarkNativeAsOptional("Logger.SetLevel");
    MarkNativeAsOptional("Logger.SetPattern");
    MarkNativeAsOptional("Logger.ShouldLog");
    MarkNativeAsOptional("Logger.Log");
    MarkNativeAsOptional("Logger.LogF");
    MarkNativeAsOptional("Logger.LogSrc");
    MarkNativeAsOptional("Logger.LogSrcF");
    MarkNativeAsOptional("Logger.LogLoc");
    MarkNativeAsOptional("Logger.LogLocF");
    MarkNativeAsOptional("Logger.LogStackTrace");
    MarkNativeAsOptional("Logger.LogStackTraceF");
    MarkNativeAsOptional("Logger.Trace");
    MarkNativeAsOptional("Logger.TraceF");
    MarkNativeAsOptional("Logger.Debug");
    MarkNativeAsOptional("Logger.DebugF");
    MarkNativeAsOptional("Logger.Info");
    MarkNativeAsOptional("Logger.InfoF");
    MarkNativeAsOptional("Logger.Warn");
    MarkNativeAsOptional("Logger.WarnF");
    MarkNativeAsOptional("Logger.Error");
    MarkNativeAsOptional("Logger.ErrorF");
    MarkNativeAsOptional("Logger.Fatal");
    MarkNativeAsOptional("Logger.FatalF");
    MarkNativeAsOptional("Logger.Flush");
    MarkNativeAsOptional("Logger.GetFlushLevel");
    MarkNativeAsOptional("Logger.SetFlushLevel");
    MarkNativeAsOptional("Logger.GetSinks");
    MarkNativeAsOptional("Logger.GetSinksLength");
    MarkNativeAsOptional("Logger.AddSink");
    MarkNativeAsOptional("Logger.DropSink");
    MarkNativeAsOptional("Logger.SetErrorHandler");

    MarkNativeAsOptional("BasicFileSink.BasicFileSink");
    MarkNativeAsOptional("BasicFileSink.GetFilename");
    MarkNativeAsOptional("BasicFileSink.Truncate");

    MarkNativeAsOptional("CallbackSink.CallbackSink");
    MarkNativeAsOptional("CallbackSink.SetData");
    MarkNativeAsOptional("CallbackSink.GetData");

    MarkNativeAsOptional("DailyFileSink.DailyFileSink");
    MarkNativeAsOptional("DailyFileSink.GetFilename");

    MarkNativeAsOptional("RingBufferSink.RingBufferSink");
    MarkNativeAsOptional("RingBufferSink.DrainLatest");
    MarkNativeAsOptional("RingBufferSink.DrainOldest");
    MarkNativeAsOptional("RingBufferSink.GetMaxSize");
    MarkNativeAsOptional("RingBufferSink.GetSize");

    MarkNativeAsOptional("RotatingFileSink.RotatingFileSink");
    MarkNativeAsOptional("RotatingFileSink.GetFilename");
    MarkNativeAsOptional("RotatingFileSink.RotateNow");
    MarkNativeAsOptional("RotatingFileSink.GetMaxSize");
    MarkNativeAsOptional("RotatingFileSink.SetMaxSize");
    MarkNativeAsOptional("RotatingFileSink.GetMaxFiles");
    MarkNativeAsOptional("RotatingFileSink.SetMaxFiles");
    MarkNativeAsOptional("RotatingFileSink.CalcFilename");

    MarkNativeAsOptional("ServerConsoleSink.ServerConsoleSink");

    MarkNativeAsOptional("Sink.Log");
    MarkNativeAsOptional("Sink.Flush");
    MarkNativeAsOptional("Sink.GetLevel");
    MarkNativeAsOptional("Sink.SetLevel");
    MarkNativeAsOptional("Sink.SetPattern");
    MarkNativeAsOptional("Sink.ShouldLog");
}
#endif
