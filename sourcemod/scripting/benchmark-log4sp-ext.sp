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

    RegConsoleCmd("sm_log4sp_bench_base_file_block",        Command_BenchBaseFileBlock);
    RegConsoleCmd("sm_log4sp_bench_daily_file_block",       Command_BenchDailyFileBlock);
    RegConsoleCmd("sm_log4sp_bench_rotating_file_block",    Command_BenchRotatingFileBlock);
    RegConsoleCmd("sm_log4sp_bench_server_console_block",   Command_BenchServerConsoleBlock);

    RegConsoleCmd("sm_log4sp_bench_base_file_overrun",      Command_BenchBaseFileOverrun);
    RegConsoleCmd("sm_log4sp_bench_daily_file_overrun",     Command_BenchDailyFileOverrun);
    RegConsoleCmd("sm_log4sp_bench_rotating_file_overrun",  Command_BenchRotatingFileOverrun);
    RegConsoleCmd("sm_log4sp_bench_server_console_overrun", Command_BenchServerConsoleOverrun);
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


Action Command_BenchBaseFileBlock(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateBaseFileLogger("name-E", "logs/benchmark/file-E.log", .async=true, .policy=AsyncOverflowPolicy_Block));

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "base-file-block", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchDailyFileBlock(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateDailyFileLogger("name-F", "logs/benchmark/file-F.log", .async=true, .policy=AsyncOverflowPolicy_Block));

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "daily-file-block", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchRotatingFileBlock(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateRotatingFileLogger("name-G", "logs/benchmark/file-G.log", g_iFileSize, g_iRotatingFiles, .async=true, .policy=AsyncOverflowPolicy_Block));

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "rotating-file-block", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchServerConsoleBlock(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateServerConsoleLogger("name-H", .async=true, .policy=AsyncOverflowPolicy_Block));

    // 延后输出, 缓解测试消息和输出结果混淆在一起难以阅读的问题
    DataPack data = new DataPack();
    data.WriteCell(iters);
    data.WriteFloat(delta);
    CreateTimer(7.0, Timer_OutputServerConsoleBlock, data);
    return Plugin_Handled;
}

Action Timer_OutputServerConsoleBlock(Handle timer, DataPack data)
{
    data.Reset();
    int iters = data.ReadCell();
    float delta = data.ReadFloat();
    delete data;

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "server-console-overrun", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


Action Command_BenchBaseFileOverrun(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateBaseFileLogger("name-I", "logs/benchmark/file-I.log", .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "base-file-overrun", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchDailyFileOverrun(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateDailyFileLogger("name-J", "logs/benchmark/file-J.log", .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "daily-file-overrun", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchRotatingFileOverrun(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateRotatingFileLogger("name-K", "logs/benchmark/file-K.log", g_iFileSize, g_iRotatingFiles, .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "rotating-file-overrun", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_BenchServerConsoleOverrun(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateServerConsoleLogger("name-L", .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    // 延后输出, 缓解测试消息和输出结果混淆在一起难以阅读的问题
    DataPack data = new DataPack();
    data.WriteCell(iters);
    data.WriteFloat(delta);
    CreateTimer(1.0, Timer_OutputServerConsoleOverrun, data);
    return Plugin_Handled;
}

Action Timer_OutputServerConsoleOverrun(Handle timer, DataPack data)
{
    data.Reset();
    int iters = data.ReadCell();
    float delta = data.ReadFloat();
    delete data;

    PrintToServer("");
    PrintToServer("[benchmark] %22s | Iters %7d | Elapsed %6.3f secs %9d/sec", "server-console-overrun", iters, delta, RoundToFloor(iters / delta));
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


