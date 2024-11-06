#include <sourcemod>
#include <profiler>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Benchmark Log4sp.ext"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Benchmark Logging for SourcePawn extension api"
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
    RegConsoleCmd("sm_log4sp_bench_base_files_st",          Command_BenchBaseFilesST);
    RegConsoleCmd("sm_log4sp_bench_daily_files_st",         Command_BenchDailyFilesST);
    RegConsoleCmd("sm_log4sp_bench_rotating_files_st",      Command_BenchRotatingFilesST);
    RegConsoleCmd("sm_log4sp_bench_server_console_st",      Command_BenchServerConsoleST);
}


Action Command_BenchBaseFilesST(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateBaseFileLogger("name-A", "logs/benchmark/file-A.log"));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "base-file-st", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchDailyFilesST(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateDailyFileLogger("name-B", "logs/benchmark/file-B.log"));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "daily-file-st", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchRotatingFilesST(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateRotatingFileLogger("name-C", "logs/benchmark/file-C.log", g_iFileSize, g_iRotatingFiles));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "rotating-file-st", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchServerConsoleST(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateServerConsoleLogger("name-D"));

    PrintToServer("");
    PrintToServer("[benchmark] %17s | Iters %7d | Elapsed %6.3f secs %9d/sec", "server-console-st", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


float BenchLogger(int howmany, Logger logger)
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

    delete logger;

    return delta;
}


