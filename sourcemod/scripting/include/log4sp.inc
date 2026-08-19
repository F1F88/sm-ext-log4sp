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
    name = "log4sp",
    file = "log4sp.ext",
#if defined AUTOLOAD_EXTENSIONS && !defined LOG4SP_HEADER_ONLY
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS && !defined LOG4SP_HEADER_ONLY
    required = 1,
#else
    required = 0,
#endif
};


#if !defined REQUIRE_EXTENSIONS || defined LOG4SP_HEADER_ONLY
public void __ext_log4sp_SetNTVOptional()
{
    MarkNativeAsOptional("GetLog4spVersion");

    MarkNativeAsOptional("Logger.Logger");
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
    MarkNativeAsOptional("Logger.GetName");
    MarkNativeAsOptional("Logger.GetNameLength");
    MarkNativeAsOptional("Logger.GetLevel");
    MarkNativeAsOptional("Logger.SetLevel");
    MarkNativeAsOptional("Logger.ShouldLog");
    MarkNativeAsOptional("Logger.SetPattern");
    MarkNativeAsOptional("Logger.Flush");
    MarkNativeAsOptional("Logger.GetFlushLevel");
    MarkNativeAsOptional("Logger.SetFlushLevel");
    MarkNativeAsOptional("Logger.ShouldFlush");
    MarkNativeAsOptional("Logger.GetSinks");
    MarkNativeAsOptional("Logger.GetSinksLength");
    MarkNativeAsOptional("Logger.AddSink");
    MarkNativeAsOptional("Logger.DropSink");
    MarkNativeAsOptional("Logger.SetErrorHandler");
    MarkNativeAsOptional("Logger.Equals");
    MarkNativeAsOptional("Logger.IsValid");

    MarkNativeAsOptional("BasicFileSink.BasicFileSink");
    MarkNativeAsOptional("BasicFileSink.GetFilename");
    MarkNativeAsOptional("BasicFileSink.Truncate");
    MarkNativeAsOptional("BasicFileSink.IsValid");

    MarkNativeAsOptional("CallbackSink.CallbackSink");
    MarkNativeAsOptional("CallbackSink.SetData");
    MarkNativeAsOptional("CallbackSink.GetData");
    MarkNativeAsOptional("CallbackSink.IsValid");

    MarkNativeAsOptional("DailyFileSink.DailyFileSink");
    MarkNativeAsOptional("DailyFileSink.GetFilename");
    MarkNativeAsOptional("DailyFileSink.IsValid");

    MarkNativeAsOptional("RingBufferSink.RingBufferSink");
    MarkNativeAsOptional("RingBufferSink.DrainLatest");
    MarkNativeAsOptional("RingBufferSink.DrainOldest");
    MarkNativeAsOptional("RingBufferSink.GetMaxSize");
    MarkNativeAsOptional("RingBufferSink.GetSize");
    MarkNativeAsOptional("RingBufferSink.IsValid");

    MarkNativeAsOptional("RotatingFileSink.RotatingFileSink");
    MarkNativeAsOptional("RotatingFileSink.GetFilename");
    MarkNativeAsOptional("RotatingFileSink.RotateNow");
    MarkNativeAsOptional("RotatingFileSink.GetMaxSize");
    MarkNativeAsOptional("RotatingFileSink.SetMaxSize");
    MarkNativeAsOptional("RotatingFileSink.GetMaxFiles");
    MarkNativeAsOptional("RotatingFileSink.SetMaxFiles");
    MarkNativeAsOptional("RotatingFileSink.IsValid");
    MarkNativeAsOptional("RotatingFileSink.CalcFilename");

    MarkNativeAsOptional("ServerConsoleSink.ServerConsoleSink");
    MarkNativeAsOptional("ServerConsoleSink.IsValid");

    MarkNativeAsOptional("Sink.Log");
    MarkNativeAsOptional("Sink.Flush");
    MarkNativeAsOptional("Sink.GetLevel");
    MarkNativeAsOptional("Sink.SetLevel");
    MarkNativeAsOptional("Sink.ShouldLog");
    MarkNativeAsOptional("Sink.SetPattern");
    MarkNativeAsOptional("Sink.Equals");
    MarkNativeAsOptional("Sink.IsValid");
}
#endif
