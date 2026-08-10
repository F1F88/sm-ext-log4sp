#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <profiler>
#include <log4sp>


enum
{
    CmdSinks_BasicFile      = (1 << 0),
    CmdSinks_Callback       = (1 << 1),
    CmdSinks_DailyFile      = (1 << 2),
    CmdSinks_RingBuffer     = (1 << 3),
    CmdSinks_RotateFile     = (1 << 4),
    CmdSinks_ServerConsole  = (1 << 5),
    CmdSinks_All            = (~0)
};

enum
{
    CmdFuncs_Log            = (1 << 0),
    CmdFuncs_LogSrc         = (1 << 1),
    CmdFuncs_LogLoc         = (1 << 2),
    CmdFuncs_LogEx          = (1 << 3),
    CmdFuncs_LogAmxTpl      = (1 << 4),
    CmdFuncs_All            = (~0)
};


Profiler g_hProfiler = null;
int      g_iBenchRound = 0;


public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    RegConsoleCmd("sm_bench_log4sp", Command_Bench);

    g_hProfiler = new Profiler();

    BenchDB.Initialize();
}

public void OnPluginEnd()
{
    BenchDB.Destroy();
}


Action Command_Bench(int client, int args)
{
    // sm_bench_log4sp <calls> <sinks> <functions> <fmts>
    //    calls: Integer - Default 1_000_000
    //    sinks: Integer - Default All(~0)
    //       BasicFile      1
    //       Callback       2
    //       DailyFile      4
    //       RingBuffer     8
    //       RotateFile     16
    //       ServerConsole  32
    //    funcs: Integer - Default 8
    //       Log            1
    //       LogSrc         2
    //       LogLoc         4
    //       LogEx          8
    //       LogAmxTpl      16
    //    fmts: Boolean - Default false
    //       true           all
    //       false          mock
    int calls = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;
    int sinks = (args >= 2) ? GetCmdArgInt(2) : CmdSinks_All;
    int funcs = (args >= 3) ? GetCmdArgInt(3) : CmdFuncs_LogEx;
    bool fmts = (args >= 4) ? (!!GetCmdArgInt(4)) : false;

    BenchAll(calls, sinks, funcs, fmts);

    return Plugin_Handled;
}


void BenchAll(int calls, int sinks, int funcs, bool fmts)
{
    g_iBenchRound++;

    if (sinks & CmdSinks_BasicFile)
    {
        char filename[PLATFORM_MAX_PATH];
        BuildPath(Path_SM, filename, sizeof(filename), "logs/bench-log4sp/basic-file.log");

        BasicFileSink sink = new BasicFileSink(filename, .truncate=true);
        Logger logger = new Logger("basic-file");
        logger.AddSink(sink);
        sink.Close();

        BenchAllLogFunc(calls, funcs, fmts, logger);

        logger.Close();
    }

    if (sinks & CmdSinks_Callback)
    {
        CallbackSink sink = new CallbackSink(CallBackSink_CB);
        Logger logger = new Logger("callback");
        logger.AddSink(sink);
        sink.Close();

        BenchAllLogFunc(calls, funcs, fmts, logger);

        logger.Close();
    }

    if (sinks & CmdSinks_DailyFile)
    {
        char filename[PLATFORM_MAX_PATH];
        BuildPath(Path_SM, filename, sizeof(filename), "logs/bench-log4sp/daily-file.log");

        DailyFileSink sink = new DailyFileSink(filename, .truncate=true);
        Logger logger = new Logger("daily-file");
        logger.AddSink(sink);
        sink.Close();

        BenchAllLogFunc(calls, funcs, fmts, logger);

        logger.Close();
    }

    if (sinks & CmdSinks_RingBuffer)
    {
        RingBufferSink sink = new RingBufferSink(1024);
        Logger logger = new Logger("ring-buffer");
        logger.AddSink(sink);
        sink.Close();

        BenchAllLogFunc(calls, funcs, fmts, logger);

        logger.Close();
    }

    if (sinks & CmdSinks_RotateFile)
    {
        char filename[PLATFORM_MAX_PATH];
        BuildPath(Path_SM, filename, sizeof(filename), "logs/bench-log4sp/rotate-file.log");

        const int FILE_SIZE = 30 * 1024 * 1024;
        const int FILES     = 5;

        RotatingFileSink sink = new RotatingFileSink(filename, FILE_SIZE, FILES);
        Logger logger = new Logger("rotate-file");
        logger.AddSink(sink);
        sink.Close();

        BenchAllLogFunc(calls, funcs, fmts, logger);

        logger.Close();
    }

    if (sinks & CmdSinks_ServerConsole)
    {
        ServerConsoleSink sink = new ServerConsoleSink();
        Logger logger = new Logger("server-console");
        logger.AddSink(sink);
        sink.Close();

        BenchAllLogFunc(calls, funcs, fmts, logger);

        logger.Close();
    }

    BenchDB.Instance().ExportToFile();
    BenchDB.Instance().PrintToServer();
}


void BenchAllLogFunc(int calls, int funcs, bool mock, Logger logger)
{
    if (funcs & CmdFuncs_Log)
    {
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.Info("Hello logger: msg number 777");
        }
        g_hProfiler.Stop();

        char name[sizeof(BenchData::name)];
        logger.GetName(name, sizeof(name));
        BenchDB.Instance().Insert(name, "Log", NULL_STRING, calls, g_hProfiler.Time);
    }

    if (funcs & CmdFuncs_LogSrc)
    {
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.LogSrc(LogLevel_Info, "Hello logger: msg number 777");
        }
        g_hProfiler.Stop();

        char name[sizeof(BenchData::name)];
        logger.GetName(name, sizeof(name));
        BenchDB.Instance().Insert(name, "LogSrc", NULL_STRING, calls, g_hProfiler.Time);
    }

    if (funcs & CmdFuncs_LogLoc)
    {
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.LogLoc(__BINARY_PATH__, __LINE__, "BenchLogLoc", LogLevel_Info, "Hello logger: msg number 777");
        }
        g_hProfiler.Stop();

        char name[sizeof(BenchData::name)];
        logger.GetName(name, sizeof(name));
        BenchDB.Instance().Insert(name, "LogLoc", NULL_STRING, calls, g_hProfiler.Time);
    }

    if (funcs & CmdFuncs_LogEx)
        BenchLogEx(calls, mock, logger);

    if (funcs & CmdFuncs_LogAmxTpl)
        BenchLogAmxTpl(calls, mock, logger);
}


void BenchLogEx(int calls, bool fmts, Logger logger)
{
    char name[sizeof(BenchData::name)];
    logger.GetName(name, sizeof(name));

    // mock
    {
        int value = 777;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx("Hello logger: msg number %d", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", "mock", calls, g_hProfiler.Time);
    }

    if (!fmts)
        return;

    // Binary
    {
        char fmt[] = "%b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Integer
    {
        char fmt[] = "%d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer
    {
        char fmt[] = "%u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Float
    {
        char fmt[] = "%f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%6.3f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Special
    {
        char fmt[] = "%L";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%N";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%E";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // String
    {
        char fmt[] = "%s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%30s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Translation
    LoadTranslations("common.phrases");
    {
        char fmt[] = "%t";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%T";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value, LANG_SERVER);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Hex
    {
        char fmt[] = "%X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx("%X", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

#if defined SM_INT64_SUPPORTED
    // Binary 64
    {
        char fmt[] = "%lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Integer 64
    {
        char fmt[] = "%ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer 64
    {
        char fmt[] = "%lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }

    // Hex 64
    {
        char fmt[] = "%lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoEx(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoEx", fmt, calls, g_hProfiler.Time);
    }
#endif      // SM_INT64_SUPPORTED
}

void BenchLogAmxTpl(int calls, bool fmts, Logger logger)
{
    char name[sizeof(BenchData::name)];
    logger.GetName(name, sizeof(name));

    // mock
    {
        int value = 777;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl("Hello logger: msg number %d", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", "mock", calls, g_hProfiler.Time);
    }

    if (!fmts)
        return;

    // Binary
    {
        char fmt[] = "%b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Integer
    {
        char fmt[] = "%d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer
    {
        char fmt[] = "%u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Float
    {
        char fmt[] = "%f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%6.3f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Special
    {
        char fmt[] = "%L";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%N";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%E";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // String
    {
        char fmt[] = "%s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%30s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Translation
    LoadTranslations("common.phrases");
    {
        char fmt[] = "%t";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%T";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value, LANG_SERVER);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Hex
    {
        char fmt[] = "%X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl("%X", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

#if defined SM_INT64_SUPPORTED
    // Binary 64
    {
        char fmt[] = "%lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Integer 64
    {
        char fmt[] = "%ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer 64
    {
        char fmt[] = "%lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }

    // Hex 64
    {
        char fmt[] = "%lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            logger.InfoAmxTpl(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "InfoAmxTpl", fmt, calls, g_hProfiler.Time);
    }
#endif      // SM_INT64_SUPPORTED
}



void CallBackSink_CB(const char[] name, LogLevel lvl, const char[] msg)
{}



#define BENCH_DB_CONF   "storage-local"
#define BENCH_DB_TABLE  "bench_log4sp"

static Database         __hDatabase = null;

enum struct BenchData
{
    int   num;      // round OR runs
    char  name[128];
    char  func[128];
    char  fmt[128];
    int   calls;
    float delta;
}

methodmap BenchDB
{
    public void CreateTable() {
        if (!SQL_FastQuery(this.db, "CREATE TABLE " ... BENCH_DB_TABLE ... " (round INTEGER NOT NULL, name VARCHAR(128) NOT NULL, func VARCHAR(128) NOT NULL, fmt VARCHAR(128), calls INTEGER NOT NULL, delta REAL NOT NULL);")) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("Create table error %s.", error);
        }
    }

    // public void DeleteTable() {
    //     if (!SQL_FastQuery(this.db, "DELETE TABLE " ... BENCH_DB_TABLE ... ";")) {
    //         char error[256];
    //         SQL_GetError(this.db, error, sizeof(error));
    //         ThrowError("Delete table error %s.", error);
    //     }
    // }

    public void DropTable() {
        if (!SQL_FastQuery(this.db, "DROP TABLE IF EXISTS " ... BENCH_DB_TABLE ... ";")) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("Drop table error %s.", error);
        }
    }

    // <round, name, func, fmt, calls, delta>
    public DBResultSet SelectList() {
        // 所有测试的完整结果
        DBResultSet result = SQL_Query(this.db, "SELECT round, name, func, COALESCE(fmt, 'null'), calls, delta FROM " ... BENCH_DB_TABLE);
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectList error %s.", error);
        }
        return result;
    }

    // DBResultSet<round, name, func, fmt, calls, delta>
    public DBResultSet SelectMaxRound() {
        // 最近一次测试的完整结果
        DBResultSet result = SQL_Query(this.db, "SELECT round, name, func, COALESCE(fmt, 'null'), calls, delta FROM " ... BENCH_DB_TABLE ... " WHERE round = (SELECT MAX(round) FROM " ... BENCH_DB_TABLE ... ") GROUP BY name, func, fmt ORDER BY name, func, fmt;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectMaxRound error %s.", error);
        }
        return result;
    }

    // DBResultSet<runs, name, func, fmt, calls, delta>
    public DBResultSet SelectSumGroupyByNameFuncFmt() {
        // 所有 sink 不同 log 方法的不同 fmt 的测试结果
        DBResultSet result = SQL_Query(this.db, "SELECT COUNT(DISTINCT round), name, func, COALESCE(fmt, 'null'), SUM(calls), SUM(delta) FROM " ... BENCH_DB_TABLE ... " GROUP BY name, func, fmt ORDER BY name, func, fmt;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectSumGroupyByFmt error %s.", error);
        }
        return result;
    }

    // DBResultSet<runs, name, func, 'null', calls, delta>
    public DBResultSet SelectSumGroupyByNameFunc() {
        // 所有 sink 不同 log 方法的测试结果 (LogF 方法只取 mock fmt)
        DBResultSet result = SQL_Query(this.db, "SELECT COUNT(DISTINCT round), name, func, 'null', SUM(calls), SUM(delta) FROM " ... BENCH_DB_TABLE ... " WHERE (fmt IS NULL OR fmt = '' OR fmt = 'null' OR fmt = 'mock') GROUP BY name, func ORDER BY name, func;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectSumGroupyByFunc error %s.", error);
        }
        return result;
    }

    // DBResultSet<runs, name, func, 'null', calls, delta>
    public DBResultSet SelectSumGroupyByName() {
        // 所有 sink 的测试结果 (只取 LogF 方法的 mock fmt)
        DBResultSet result = SQL_Query(this.db, "SELECT COUNT(DISTINCT round), name, 'null', 'null', SUM(calls), SUM(delta) FROM " ... BENCH_DB_TABLE ... " WHERE (func = 'LogF' OR fmt = 'mock') GROUP BY name ORDER BY name;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectSumGroupyByFunc error %s.", error);
        }
        return result;
    }

    // ArrayList<BenchData<num, name, func, fmt, calls, delta>>
    public ArrayList ParseResultSetToDatas(DBResultSet result) {
        if (!result)
            ThrowError("Invalid DBResultSet.");

        BenchData data;
        ArrayList datas = new ArrayList(.blocksize=sizeof(BenchData));

        for (int i = 0; result.FetchRow(); ++i) {
            data.num = result.FetchInt(0);
            result.FetchString(1, data.name, sizeof(BenchData::name));
            result.FetchString(2, data.func, sizeof(BenchData::func));
            result.FetchString(3, data.fmt,  sizeof(BenchData::fmt));
            data.calls = result.FetchInt(4);
            data.delta = result.FetchFloat(5);
            datas.PushArray(data);
        }
        return datas;
    }

    public void Insert(const char[] name, const char[] func, const char[] fmt, int calls, float delta) {
        char query[512];
        FormatEx(query, sizeof(query),
                "INSERT INTO " ... BENCH_DB_TABLE ... " (round, name, func, fmt, calls, delta) VALUES (%d, '%s', '%s', '%s', %d, %f)",
                g_iBenchRound, name, func, fmt, calls, delta);

        if (!SQL_FastQuery(this.db, query)) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("Insert data error %s.", error);
        }
    }

    public void ExportToFile(const char[] filename="logs/bench-log4sp/output", const char[] ext="csv", const char[] sep=",") {
        {
            DBResultSet result = this.SelectList();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[%s] [%d] SelectList empty!", LOG4SP_LEVEL_NAME_WARN, __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_all.%s", filename, ext);
            PrintToServer("Export SelectList to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("round%sname%sfunc%sfmt%scalls%sdelta%ssecs", sep, sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.name,  sep, data.func,  sep, data.fmt, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectMaxRound();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[%s] [%d] SelectMaxRound empty!", LOG4SP_LEVEL_NAME_WARN, __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_latest.%s", filename, ext);
            PrintToServer("Export SelectMaxRound to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("round%sname%sfunc%sfmt%scalls%sdelta%ssecs", sep, sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.name,  sep, data.func,  sep, data.fmt, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectSumGroupyByNameFuncFmt();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[%s] [%d] SelectSumGroupyByNameFuncFmt empty!", LOG4SP_LEVEL_NAME_WARN, __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_sum_group_by_name_func_fmt.%s", filename, ext);
            PrintToServer("Export SelectSumGroupyByNameFuncFmt to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("runs%sname%sfunc%sfmt%scalls%sdelta%ssecs", sep, sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.name,  sep, data.func, sep, data.fmt, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectSumGroupyByNameFunc();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[%s] [%d] SelectSumGroupyByNameFunc empty!", LOG4SP_LEVEL_NAME_WARN, __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_sum_group_by_name_func.%s", filename, ext);
            PrintToServer("Export SelectSumGroupyByNameFunc to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("runs%sname%sfunc%scalls%sdelta%ssecs", sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.name,  sep, data.func, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectSumGroupyByName();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[%s] [%d] SelectSumGroupyByName empty!", LOG4SP_LEVEL_NAME_WARN, __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_sum_group_by_name.%s", filename, ext);
            PrintToServer("Export SelectSumGroupyByName to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("runs%sname%scalls%sdelta%ssecs", sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.name,  sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }
    }

    public void PrintToServer() {
        DBResultSet result = this.SelectSumGroupyByName();
        ArrayList   datas  = this.ParseResultSetToDatas(result);
        delete result;
        if (datas.Length == 0) {
            PrintToServer("[%s] [%d] SelectSumGroupyByName empty!", LOG4SP_LEVEL_NAME_WARN, __LINE__);
            delete datas;
            return;
        }

        BenchData data;
        datas.GetArray(0, data);

        for (int i = 0; i < datas.Length; ++i) {
            datas.GetArray(i, data);
            PrintToServer("[benchmark] %-15s Runs: %-3d  Calls: %-9d  Elapsed: %-8.3f %9d/sec",
                data.name, data.num, data.calls, data.delta, RoundToFloor(data.calls / data.delta));
        }
        delete datas;
    }

    public static void Initialize() {
        char error[256];
        __hDatabase = SQL_Connect(BENCH_DB_CONF, false, error, sizeof(error));
        if (!BenchDB.Instance().db)
            ThrowError("Creating an SQL connection for \"" ... BENCH_DB_CONF ... "\" failed (error: %s)", error);

        BenchDB.Instance().DropTable();
        BenchDB.Instance().CreateTable();
    }

    public static void Destroy() {
        BenchDB.Instance().DropTable();
        delete __hDatabase;
    }

    public static BenchDB Instance() {
        return view_as<BenchDB>(true);
    }

    property Database db {
        public get() { return __hDatabase; }
    }
};

#undef BENCH_DB_CONF
#undef BENCH_DB_TABLE
