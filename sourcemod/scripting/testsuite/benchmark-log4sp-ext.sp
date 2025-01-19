#include <sourcemod>
#include <profiler>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Benchmark Log4sp.ext"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Benchmark Log for SourcePawn extension api"
#define PLUGIN_URL              "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};


static const int g_iFileSize      = 30 * 1024 * 1024;
static const int g_iRotatingFiles = 5;


public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    RegConsoleCmd("sm_log4sp_bench_base_files",                     Command_BenchBaseFiles);
    RegConsoleCmd("sm_log4sp_bench_daily_files",                    Command_BenchDailyFiles);
    RegConsoleCmd("sm_log4sp_bench_rotating_file",                  Command_BenchRotatingFile);
    RegConsoleCmd("sm_log4sp_bench_server_console",                 Command_BenchServerConsole);

    RegConsoleCmd("sm_log4sp_bench_callback",                       Command_BenchCallback);
}

Action Command_BenchBaseFiles(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, client, BaseFileSink.CreateLogger("name-A", "logs/benchmark/file-A.log", .truncate=true));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "base-file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchDailyFiles(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, client, DailyFileSink.CreateLogger("name-B", "logs/benchmark/file-B.log", .truncate=true));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "daily-file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchRotatingFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, client, RotatingFileSink.CreateLogger("name-C", "logs/benchmark/file-C.log", g_iFileSize, g_iRotatingFiles));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "rotating-file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchServerConsole(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, client, ServerConsoleSink.CreateLogger("name-D"));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "server-console", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


Action Command_BenchCallback(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    Logger logger = new Logger("name-E");
    logger.AddSinkEx(new CallbackSink(CallBackSink_CB));

    float delta = BenchLogger(iters, client, logger);

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "callback-sink", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

void CallBackSink_CB(const char[] name, LogLevel lvl, const char[] msg)
{
    // Do something
}


float BenchLogger(int howmany, int client, Logger logger)
{
    Profiler profiler = new Profiler();

    profiler.Start();
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
            case 6:     logger.InfoEx("|  6 |     34b:     %34b |      b:      %b |", float(i), float(-i));
            case 7:     logger.InfoEx("|  7 |    034b:    %034b |      b:      %b |", float(-i), float(i));
            case 8:     logger.InfoEx("|  8 |    -34b:    %-34b |      b:      %b |", float(i), float(-i));
            case 9:     logger.InfoEx("|  9 |   -034b:   %-034b |      b:      %b |", float(-i), float(i));
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
            case 21:    logger.InfoEx("| 21 |  16.10s:  %16.10s |   .10s:   %.10f |", "some messages", "some messages");
            case 22:    logger.InfoEx("| 22 | -16.10s: %-16.10s |  -.10s:  %-.10f |", "some messages", "some messages");
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
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    delete logger;

    return delta;
}


