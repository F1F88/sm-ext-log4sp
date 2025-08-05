#include <sourcemod>
#include <profiler>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required


Profiler g_hProfiler = null;


public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    RegConsoleCmd("sm_log4sp_bench_base_files",                     Command_BenchBaseFiles);
    RegConsoleCmd("sm_log4sp_bench_daily_files",                    Command_BenchDailyFiles);
    RegConsoleCmd("sm_log4sp_bench_rotating_file",                  Command_BenchRotatingFile);
    RegConsoleCmd("sm_log4sp_bench_server_console",                 Command_BenchServerConsole);

    RegConsoleCmd("sm_log4sp_bench_callback",                       Command_BenchCallback);
    RegConsoleCmd("sm_log4sp_bench_callback2",                      Command_BenchCallback2);

    g_hProfiler = new Profiler();
}

Action Command_BenchBaseFiles(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "base-file";
    char LOGGER_NAME[]  = "name-A";
    char FILE_PATH[]    = "addons/sourcemod/logs/benchmark/file-A.log";

    Logger logger = new Logger(LOGGER_NAME);
    BasicFileSink sink = new BasicFileSink(FILE_PATH, .truncate=true);
    logger.AddSink(sink);

    sink.Truncate();
    float delta1 = BenchLog(iters, logger);

    sink.Truncate();
    float delta2 = BenchLogEx(iters, client, logger);

    sink.Truncate();
    float delta3 = BenchLogAmxTpl(iters, client, logger);

    delete sink;
    delete logger;

    PrintToServer("");
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "Log", iters, delta1, RoundToFloor(iters / delta1));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogEx", iters, delta2, RoundToFloor(iters / delta2));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogAmxTpl", iters, delta3, RoundToFloor(iters / delta3));
    return Plugin_Handled;
}

Action Command_BenchDailyFiles(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "daily-file";
    char LOGGER_NAME[]  = "name-B";
    char FILE_PATH[]    = "addons/sourcemod/logs/benchmark/file-B.log";

    Logger logger = DailyFileSink.CreateLogger(LOGGER_NAME, FILE_PATH, .truncate=true);

    float delta1 = BenchLog(iters, logger);
    delete logger;

    logger = DailyFileSink.CreateLogger(LOGGER_NAME, FILE_PATH, .truncate=true);
    float delta2 = BenchLogEx(iters, client, logger);
    delete logger;

    logger = DailyFileSink.CreateLogger(LOGGER_NAME, FILE_PATH, .truncate=true);
    float delta3 = BenchLogAmxTpl(iters, client, logger);
    delete logger;

    PrintToServer("");
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "Log", iters, delta1, RoundToFloor(iters / delta1));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogEx", iters, delta2, RoundToFloor(iters / delta2));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogAmxTpl", iters, delta3, RoundToFloor(iters / delta3));
    return Plugin_Handled;
}

Action Command_BenchRotatingFile(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "rotating-file";
    char LOGGER_NAME[]  = "name-C";
    char FILE_PATH[]    = "addons/sourcemod/logs/benchmark/file-C.log";
    const int FILE_SIZE = 30 * 1024 * 1024;
    const int FILES     = 5;

    Logger logger = new Logger(LOGGER_NAME);
    RotatingFileSink sink = new RotatingFileSink(FILE_PATH, FILE_SIZE, FILES);
    logger.AddSink(sink);

    sink.RotateNow();
    float delta1 = BenchLog(iters, logger);

    sink.RotateNow();
    float delta2 = BenchLogEx(iters, client, logger);

    sink.RotateNow();
    float delta3 = BenchLogAmxTpl(iters, client, logger);

    delete sink;
    delete logger;

    PrintToServer("");
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "Log", iters, delta1, RoundToFloor(iters / delta1));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogEx", iters, delta2, RoundToFloor(iters / delta2));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogAmxTpl", iters, delta3, RoundToFloor(iters / delta3));
    return Plugin_Handled;
}

Action Command_BenchServerConsole(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "server-console";
    char LOGGER_NAME[]  = "name-D";

    Logger logger = ServerConsoleSink.CreateLogger(LOGGER_NAME);

    float delta1 = BenchLog(iters, logger);
    float delta2 = BenchLogEx(iters, client, logger);
    float delta3 = BenchLogAmxTpl(iters, client, logger);

    delete logger;

    PrintToServer("");
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "Log", iters, delta1, RoundToFloor(iters / delta1));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogEx", iters, delta2, RoundToFloor(iters / delta2));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogAmxTpl", iters, delta3, RoundToFloor(iters / delta3));
    return Plugin_Handled;
}


Action Command_BenchCallback(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "callback-sink";
    char LOGGER_NAME[]  = "name-E";

    Logger logger = CallbackSink.CreateLogger(LOGGER_NAME, CB_OnLog, _, CB_OnFlush);

    float delta1 = BenchLog(iters, logger);
    float delta2 = BenchLogEx(iters, client, logger);
    float delta3 = BenchLogAmxTpl(iters, client, logger);

    delete logger;

    PrintToServer("");
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "Log", iters, delta1, RoundToFloor(iters / delta1));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogEx", iters, delta2, RoundToFloor(iters / delta2));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogAmxTpl", iters, delta3, RoundToFloor(iters / delta3));
    return Plugin_Handled;
}

Action Command_BenchCallback2(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "callback-sink2";
    char LOGGER_NAME[]  = "name-F";

    Logger logger = CallbackSink2.CreateLogger(LOGGER_NAME, CB_OnLog, _, CB_OnFlush);

    float delta1 = BenchLog(iters, logger);
    float delta2 = BenchLogEx(iters, client, logger);
    float delta3 = BenchLogAmxTpl(iters, client, logger);

    delete logger;

    PrintToServer("");
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "Log", iters, delta1, RoundToFloor(iters / delta1));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogEx", iters, delta2, RoundToFloor(iters / delta2));
    PrintToServer("[benchmark] %17s | %9s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, "LogAmxTpl", iters, delta3, RoundToFloor(iters / delta3));
    return Plugin_Handled;
}


float BenchLog(int howmany, Logger logger)
{
    g_hProfiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        logger.Info("| 77 |       *******       This is a performance benchmark log message!!!!!!!       *******       |");
    }
    g_hProfiler.Stop();
    return g_hProfiler.Time;
}


float BenchLogEx(int howmany, int client, Logger logger)
{
    g_hProfiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        switch (i & 31)
        {
            case 0:     logger.InfoEx("|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     logger.InfoEx("|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     logger.InfoEx("|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     logger.InfoEx("|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     logger.InfoEx("|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     logger.InfoEx("|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     logger.InfoEx("|  6 |     34b:     %34b |      b:      %b |", i, -i);
            case 7:     logger.InfoEx("|  7 |    034b:    %034b |      b:      %b |", -i, i);
            case 8:     logger.InfoEx("|  8 |    -34b:    %-34b |      b:      %b |", i, -i);
            case 9:     logger.InfoEx("|  9 |   -034b:   %-034b |      b:      %b |", -i, i);
            case 10:    logger.InfoEx("| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    logger.InfoEx("| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    logger.InfoEx("| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    logger.InfoEx("| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    logger.InfoEx("| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    logger.InfoEx("| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    logger.InfoEx("| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    logger.InfoEx("| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    logger.InfoEx("| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    logger.InfoEx("| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    logger.InfoEx("| 21 |  16.10s:  %16.10s |   .10s:   %.10s |", "some messages", "some messages");
            case 22:    logger.InfoEx("| 22 | -16.10s: %-16.10s |  -.10s:  %-.10s |", "some messages", "some messages");
            case 23:    logger.InfoEx("| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    logger.InfoEx("| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    logger.InfoEx("| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    logger.InfoEx("| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    logger.InfoEx("| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    logger.InfoEx("| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    logger.InfoEx("| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    logger.InfoEx("| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    logger.InfoEx("| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    g_hProfiler.Stop();
    return g_hProfiler.Time;
}

float BenchLogAmxTpl(int howmany, int client, Logger logger)
{
    g_hProfiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        switch (i & 31)
        {
            case 0:     logger.InfoAmxTpl("|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     logger.InfoAmxTpl("|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     logger.InfoAmxTpl("|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     logger.InfoAmxTpl("|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     logger.InfoAmxTpl("|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     logger.InfoAmxTpl("|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     logger.InfoAmxTpl("|  6 |     34b:     %34b |      b:      %b |", i, -i);
            case 7:     logger.InfoAmxTpl("|  7 |    034b:    %034b |      b:      %b |", -i, i);
            case 8:     logger.InfoAmxTpl("|  8 |    -34b:    %-34b |      b:      %b |", i, -i);
            case 9:     logger.InfoAmxTpl("|  9 |   -034b:   %-034b |      b:      %b |", -i, i);
            case 10:    logger.InfoAmxTpl("| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    logger.InfoAmxTpl("| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    logger.InfoAmxTpl("| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    logger.InfoAmxTpl("| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    logger.InfoAmxTpl("| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    logger.InfoAmxTpl("| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    logger.InfoAmxTpl("| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    logger.InfoAmxTpl("| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    logger.InfoAmxTpl("| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    logger.InfoAmxTpl("| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    logger.InfoAmxTpl("| 21 |  16.10s:  %16.10s |   .10s:   %.10s |", "some messages", "some messages");
            case 22:    logger.InfoAmxTpl("| 22 | -16.10s: %-16.10s |  -.10s:  %-.10s |", "some messages", "some messages");
            case 23:    logger.InfoAmxTpl("| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    logger.InfoAmxTpl("| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    logger.InfoAmxTpl("| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    logger.InfoAmxTpl("| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    logger.InfoAmxTpl("| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    logger.InfoAmxTpl("| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    logger.InfoAmxTpl("| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    logger.InfoAmxTpl("| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    logger.InfoAmxTpl("| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    g_hProfiler.Stop();
    return g_hProfiler.Time;
}



static void CB_OnLog(const char[] name, LogLevel lvl, const char[] msg) {}
static void CB_OnFlush() {}
