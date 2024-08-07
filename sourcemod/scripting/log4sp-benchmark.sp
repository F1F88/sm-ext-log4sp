#include <sourcemod>
#include <profiler>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Log4sp Benchmark"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.2.0"
#define PLUGIN_DESCRIPTION                  "Logging for SourcePawn Benchmark"
#define PLUGIN_URL                          "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};

#include <log4sp>


static const int g_iFileSize      = 30 * 1024 * 1024;
static const int g_iRotatingFiles = 5;


public void OnPluginStart()
{
    RegConsoleCmd("sm_log4sp_bench_files_st",               CMD_BenchLog4spFilesST);
    RegConsoleCmd("sm_log4sp_bench_server_console_st",      CMD_BenchLog4spServerConsoleST);

    RegConsoleCmd("sm_log4sp_bench_files_async",            CMD_BenchLog4spFilesAsync);
    RegConsoleCmd("sm_log4sp_bench_server_console_async",   CMD_BenchLog4spServerConsoleAsync);
}


Action CMD_BenchLog4spFilesST(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    PrintToServer("[log4sp-benchmark] **************************************************************");
    PrintToServer("[log4sp-benchmark] Log4sp.ext API Single thread, %d iterations", iters);
    PrintToServer("[log4sp-benchmark] **************************************************************");

    Logger baseFileST = Logger.CreateBaseFileLogger("base-file-st__________", "logs/benchmark/base-file-st_______________.log", true);
    BenchLogger(iters, baseFileST);
    delete baseFileST;

    Logger rotatingFileST = Logger.CreateRotatingFileLogger("rotating-file-st______", "logs/benchmark/rotating-file-st___________.log", g_iFileSize, g_iRotatingFiles);
    BenchLogger(iters, rotatingFileST);
    delete rotatingFileST;

    Logger dailyFileST = Logger.CreateDailyFileLogger("daily-file-st_________", "logs/benchmark/daily-file-st_____________.log");
    BenchLogger(iters, dailyFileST);
    delete dailyFileST;

    return Plugin_Handled;
}

Action CMD_BenchLog4spServerConsoleST(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    char[] file = "logs/benchmark/result-server-console-st___.txt";
    LogToFileEx(file, "[log4sp-benchmark] **************************************************************");
    LogToFileEx(file, "[log4sp-benchmark] Log4sp.ext Server Console Single thread, %d iterations", iters);
    LogToFileEx(file, "[log4sp-benchmark] **************************************************************");

    Logger console = Logger.CreateServerConsoleLogger("server-console-st_____");
    BenchLoggerServerConsole(iters, console, file);
    delete console;

    return Plugin_Handled;
}


Action CMD_BenchLog4spFilesAsync(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    PrintToServer("[log4sp-benchmark] **************************************************************");
    PrintToServer("[log4sp-benchmark] Log4sp.ext API Asynchronous mode, %d iterations", iters);
    PrintToServer("[log4sp-benchmark] **************************************************************");

    PrintToServer("[log4sp-benchmark] ");
    PrintToServer("[log4sp-benchmark] *********************************");
    PrintToServer("[log4sp-benchmark] Queue Overflow Policy: block");
    PrintToServer("[log4sp-benchmark] *********************************");

    Logger baseFileBlock = Logger.CreateBaseFileLogger("base-file-block_______", "logs/benchmark/base-file-block____________.log", true, .async=true, .policy=AsyncOverflowPolicy_Block);
    BenchLogger(iters, baseFileBlock);
    delete baseFileBlock;

    Logger rotatingFileBlock = Logger.CreateRotatingFileLogger("rotating-file-block___", "logs/benchmark/rotating-file-block________.log", g_iFileSize, g_iRotatingFiles, .async=true, .policy=AsyncOverflowPolicy_Block);
    BenchLogger(iters, rotatingFileBlock);
    delete rotatingFileBlock;

    Logger dailyFileBlock = Logger.CreateDailyFileLogger("daily-file-block______", "logs/benchmark/daily-file-block___________.log", .async=true, .policy=AsyncOverflowPolicy_Block);
    BenchLogger(iters, dailyFileBlock);
    delete dailyFileBlock;

    PrintToServer("[log4sp-benchmark] ");
    PrintToServer("[log4sp-benchmark] *********************************");
    PrintToServer("[log4sp-benchmark] Queue Overflow Policy: overrun");
    PrintToServer("[log4sp-benchmark] *********************************");

    Logger baseFileOverrun = Logger.CreateBaseFileLogger("base-file-overrun_____", "logs/benchmark/base-file-overrun__________.log", true, .async=true, .policy=AsyncOverflowPolicy_OverrunOldest);
    BenchLogger(iters, baseFileOverrun);
    delete baseFileOverrun;

    Logger rotatingFileOverrun = Logger.CreateRotatingFileLogger("rotating-file-overrun_", "logs/benchmark/rotating-file-overrun______.log", g_iFileSize, g_iRotatingFiles, .async=true, .policy=AsyncOverflowPolicy_OverrunOldest);
    BenchLogger(iters, rotatingFileOverrun);
    delete rotatingFileOverrun;

    Logger dailyFileOverrun = Logger.CreateDailyFileLogger("daily-file-overrun____", "logs/benchmark/daily-file-overrun_________.log", .async=true, .policy=AsyncOverflowPolicy_OverrunOldest);
    BenchLogger(iters, dailyFileOverrun);
    delete dailyFileOverrun;

    return Plugin_Handled;
}

Action CMD_BenchLog4spServerConsoleAsync(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    char[] file = "logs/benchmark/result-server-console-async.txt";
    LogToFileEx(file, "[log4sp-benchmark] **************************************************************");
    LogToFileEx(file, "[log4sp-benchmark] Log4sp.ext Server Console Asynchronous mode, %d iterations", iters);
    LogToFileEx(file, "[log4sp-benchmark] **************************************************************");

    LogToFileEx(file, "[log4sp-benchmark]");
    LogToFileEx(file, "[log4sp-benchmark] *********************************");
    LogToFileEx(file, "[log4sp-benchmark] Queue Overflow Policy: block");
    LogToFileEx(file, "[log4sp-benchmark] *********************************");

    Logger consoleBlock = Logger.CreateServerConsoleLogger("server-console-block__", .async=true, .policy=AsyncOverflowPolicy_Block);
    BenchLoggerServerConsole(iters, consoleBlock, file);
    delete consoleBlock;


    LogToFileEx(file, "[log4sp-benchmark]");
    LogToFileEx(file, "[log4sp-benchmark] *********************************");
    LogToFileEx(file, "[log4sp-benchmark] Queue Overflow Policy: overrun");
    LogToFileEx(file, "[log4sp-benchmark] *********************************");

    Logger consoleOverrun = Logger.CreateServerConsoleLogger("server-console-overrun", .async=true, .policy=AsyncOverflowPolicy_Block);
    BenchLoggerServerConsole(iters, consoleOverrun, file);
    delete consoleOverrun;

    return Plugin_Handled;
}


void BenchLogger(int howmany, Logger logger)
{
    Profiler profiler = new Profiler();

    profiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        logger.InfoAmxTpl("Hello log4sp: msg number %d", i);
    }
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    int delta_d = RoundToFloor(howmany / delta);

    char name[64];
    logger.GetName(name, sizeof(name));

    PrintToServer("[log4sp-benchmark] %-24s Elapsed: %5.2f secs %12d /sec", name, delta, delta_d);
}

void BenchLoggerServerConsole(int howmany, Logger logger, const char[] file)
{
    Profiler profiler = new Profiler();

    profiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        logger.InfoAmxTpl("Hello log4sp: msg number %d", i);
    }
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    int delta_d = RoundToFloor(howmany / delta);

    char name[64];
    logger.GetName(name, sizeof(name));

    LogToFileEx(file, "[log4sp-benchmark] %-24s Elapsed: %5.2f secs %12d /sec", name, delta, delta_d);
}



/**
 * sm_log4sp_bench_files_st
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] Log4sp.ext API Single thread, 1000000 iterations
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] base-file-st__________   Elapsed:  0.29 secs      3415020 /sec
 * [log4sp-benchmark] rotating-file-st______   Elapsed:  0.30 secs      3239957 /sec
 * [log4sp-benchmark] daily-file-st_________   Elapsed:  0.30 secs      3314001 /sec
 *
 *
 *
 * sm_log4sp_bench_server_console_st
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] Log4sp.ext Server Console Single thread, 1000000 iterations
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] server-console-st_____   Elapsed:  5.74 secs       174034 /sec
 *
 *
 *
 * sm_log4sp_bench_files_async
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] Log4sp.ext API Asynchronous mode, 1000000 iterations
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark]
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] Queue Overflow Policy: block
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] base-file-block_______   Elapsed:  0.49 secs      2031570 /sec
 * [log4sp-benchmark] rotating-file-block___   Elapsed:  0.49 secs      2006179 /sec
 * [log4sp-benchmark] daily-file-block______   Elapsed:  0.49 secs      2020079 /sec
 * [log4sp-benchmark]
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] Queue Overflow Policy: overrun
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] base-file-overrun_____   Elapsed:  0.46 secs      2158032 /sec
 * [log4sp-benchmark] rotating-file-overrun_   Elapsed:  0.44 secs      2236701 /sec
 * [log4sp-benchmark] daily-file-overrun____   Elapsed:  0.46 secs      2169291 /sec
 *
 *
 *
 * sm_log4sp_bench_server_console_async
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] Log4sp.ext Server Console Asynchronous mode, 1000000 iterations
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark]
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] Queue Overflow Policy: block
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] server-console-block__   Elapsed:  8.56 secs       116712 /sec
 * [log4sp-benchmark]
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] Queue Overflow Policy: overrun
 * [log4sp-benchmark] *********************************
 * [log4sp-benchmark] server-console-overrun   Elapsed:  8.44 secs       118378 /sec
 */
