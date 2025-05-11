#if defined _log_for_sourcepawn_included
 #endinput
#endif
#define _log_for_sourcepawn_included

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
#include <log4sp/sinks/tcp_sink>
#include <log4sp/sinks/udp_sink>








/**
 * Do not edit below this line!
 */
public Extension __ext_log4sp =
{
    name = "Log for SourcePawn",
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

    MarkNativeAsOptional("Logger.Logger");
    MarkNativeAsOptional("Logger.CreateLoggerWith");
    MarkNativeAsOptional("Logger.CreateLoggerWithEx");
    MarkNativeAsOptional("Logger.Get");
    MarkNativeAsOptional("Logger.ApplyAll");
    MarkNativeAsOptional("Logger.GetName");
    MarkNativeAsOptional("Logger.GetNameLength");
    MarkNativeAsOptional("Logger.GetLevel");
    MarkNativeAsOptional("Logger.SetLevel");
    MarkNativeAsOptional("Logger.SetPattern");
    MarkNativeAsOptional("Logger.ShouldLog");
    MarkNativeAsOptional("Logger.Log");
    MarkNativeAsOptional("Logger.LogEx");
    MarkNativeAsOptional("Logger.LogAmxTpl");
    MarkNativeAsOptional("Logger.LogSrc");
    MarkNativeAsOptional("Logger.LogSrcEx");
    MarkNativeAsOptional("Logger.LogSrcAmxTpl");
    MarkNativeAsOptional("Logger.LogLoc");
    MarkNativeAsOptional("Logger.LogLocEx");
    MarkNativeAsOptional("Logger.LogLocAmxTpl");
    MarkNativeAsOptional("Logger.LogStackTrace");
    MarkNativeAsOptional("Logger.LogStackTraceEx");
    MarkNativeAsOptional("Logger.LogStackTraceAmxTpl");
    MarkNativeAsOptional("Logger.ThrowError");
    MarkNativeAsOptional("Logger.ThrowErrorEx");
    MarkNativeAsOptional("Logger.ThrowErrorAmxTpl");
    MarkNativeAsOptional("Logger.Trace");
    MarkNativeAsOptional("Logger.TraceEx");
    MarkNativeAsOptional("Logger.TraceAmxTpl");
    MarkNativeAsOptional("Logger.Debug");
    MarkNativeAsOptional("Logger.DebugEx");
    MarkNativeAsOptional("Logger.DebugAmxTpl");
    MarkNativeAsOptional("Logger.Info");
    MarkNativeAsOptional("Logger.InfoEx");
    MarkNativeAsOptional("Logger.InfoAmxTpl");
    MarkNativeAsOptional("Logger.Warn");
    MarkNativeAsOptional("Logger.WarnEx");
    MarkNativeAsOptional("Logger.WarnAmxTpl");
    MarkNativeAsOptional("Logger.Error");
    MarkNativeAsOptional("Logger.ErrorEx");
    MarkNativeAsOptional("Logger.ErrorAmxTpl");
    MarkNativeAsOptional("Logger.Fatal");
    MarkNativeAsOptional("Logger.FatalEx");
    MarkNativeAsOptional("Logger.FatalAmxTpl");
    MarkNativeAsOptional("Logger.Flush");
    MarkNativeAsOptional("Logger.GetFlushLevel");
    MarkNativeAsOptional("Logger.FlushOn");
    MarkNativeAsOptional("Logger.AddSink");
    MarkNativeAsOptional("Logger.AddSinkEx");
    MarkNativeAsOptional("Logger.DropSink");
    MarkNativeAsOptional("Logger.SetErrorHandler");

    MarkNativeAsOptional("BasicFileSink.BasicFileSink");
    MarkNativeAsOptional("BasicFileSink.GetFilename");
    MarkNativeAsOptional("BasicFileSink.Truncate");
    MarkNativeAsOptional("BasicFileSink.CreateLogger");

    MarkNativeAsOptional("CallbackSink.CallbackSink");
    MarkNativeAsOptional("CallbackSink.SetLogCallback");
    MarkNativeAsOptional("CallbackSink.SetLogPostCallback");
    MarkNativeAsOptional("CallbackSink.SetFlushCallback");
    MarkNativeAsOptional("CallbackSink.CreateLogger");

    MarkNativeAsOptional("DailyFileSink.DailyFileSink");
    MarkNativeAsOptional("DailyFileSink.GetFilename");
    MarkNativeAsOptional("DailyFileSink.CreateLogger");

    MarkNativeAsOptional("RingBufferSink.RingBufferSink");
    MarkNativeAsOptional("RingBufferSink.Drain");
    MarkNativeAsOptional("RingBufferSink.DrainFormatted");
    MarkNativeAsOptional("RingBufferSink.CreateLogger");

    MarkNativeAsOptional("RotatingFileSink.RotatingFileSink");
    MarkNativeAsOptional("RotatingFileSink.GetFilename");
    MarkNativeAsOptional("RotatingFileSink.RotateNow");
    MarkNativeAsOptional("RotatingFileSink.CalcFilename");
    MarkNativeAsOptional("RotatingFileSink.CreateLogger");

    MarkNativeAsOptional("ServerConsoleSink.ServerConsoleSink");
    MarkNativeAsOptional("ServerConsoleSink.CreateLogger");

    MarkNativeAsOptional("Sink.GetLevel");
    MarkNativeAsOptional("Sink.SetLevel");
    MarkNativeAsOptional("Sink.SetPattern");
    MarkNativeAsOptional("Sink.ShouldLog");
    MarkNativeAsOptional("Sink.Log");
    MarkNativeAsOptional("Sink.ToPattern");
    MarkNativeAsOptional("Sink.Flush");

    MarkNativeAsOptional("TCPSink.TCPSink");
    MarkNativeAsOptional("TCPSink.CreateLogger");
}
#endif
