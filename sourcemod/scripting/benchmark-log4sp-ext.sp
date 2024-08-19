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
    RegConsoleCmd("sm_log4sp_bench_st_base_files",              Command_Bench_ST_BaseFiles);
    RegConsoleCmd("sm_log4sp_bench_st_daily_files",             Command_Bench_ST_DailyFiles);
    RegConsoleCmd("sm_log4sp_bench_st_rotating_files",          Command_Bench_ST_RotatingFiles);
    RegConsoleCmd("sm_log4sp_bench_st_server_console",          Command_Bench_ST_ServerConsole);

    RegConsoleCmd("sm_log4sp_bench_mt_block_base_file",         Command_Bench_MT_Block_BaseFile);
    RegConsoleCmd("sm_log4sp_bench_mt_block_daily_file",        Command_Bench_MT_Block_DailyFile);
    RegConsoleCmd("sm_log4sp_bench_mt_block_rotating_file",     Command_Bench_MT_Block_RotatingFile);
    RegConsoleCmd("sm_log4sp_bench_mt_block_server_console",    Command_Bench_MT_Block_ServerConsole);

    RegConsoleCmd("sm_log4sp_bench_mt_overrun_base_file",       Command_Bench_MT_Overrun_BaseFile);
    RegConsoleCmd("sm_log4sp_bench_mt_overrun_daily_file",      Command_Bench_MT_Overrun_DailyFile);
    RegConsoleCmd("sm_log4sp_bench_mt_overrun_rotating_file",   Command_Bench_MT_Overrun_RotatingFile);
    RegConsoleCmd("sm_log4sp_bench_mt_overrun_server_console",  Command_Bench_MT_Overrun_ServerConsole);
}

Action Command_Bench_ST_BaseFiles(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateBaseFileLogger("name-A", "logs/benchmark/file-A.log"));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "single thread", "base file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


Action Command_Bench_ST_DailyFiles(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateDailyFileLogger("name-B", "logs/benchmark/file-B.log"));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "single thread", "daily file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


Action Command_Bench_ST_RotatingFiles(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateRotatingFileLogger("name-C", "logs/benchmark/file-C.log", g_iFileSize, g_iRotatingFiles));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "single thread", "rotating file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_ST_ServerConsole(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateServerConsoleLogger("name-D"));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "single thread", "server console", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Block_BaseFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateBaseFileLogger("name-E", "logs/benchmark/file-E.log", .async=true, .policy=AsyncOverflowPolicy_Block));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "multi thread", "block", "base file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Block_DailyFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateDailyFileLogger("name-F", "logs/benchmark/file-F.log", .async=true, .policy=AsyncOverflowPolicy_Block));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "multi thread", "block", "daily file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Block_RotatingFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateRotatingFileLogger("name-G", "logs/benchmark/file-G.log", g_iFileSize, g_iRotatingFiles, .async=true, .policy=AsyncOverflowPolicy_Block));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "multi thread", "block", "rotating file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Block_ServerConsole(int client, int args)
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
    CreateTimer(5.0, Timer_Output_MT_Block_ServerConsole, data);
    return Plugin_Handled;
}

Action Timer_Output_MT_Block_ServerConsole(Handle timer, DataPack data)
{
    data.Reset();
    int iters = data.ReadCell();
    float delta = data.ReadFloat();
    delete data;

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "multi thread", "block", "server console", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}




Action Command_Bench_MT_Overrun_BaseFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateBaseFileLogger("name-I", "logs/benchmark/file-I.log", .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "Multi thread", "overrun", "base file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Overrun_DailyFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateDailyFileLogger("name-J", "logs/benchmark/file-J.log", .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "Multi thread", "overrun", "daily file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Overrun_RotatingFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    float delta = BenchLogger(iters, Logger.CreateRotatingFileLogger("name-K", "logs/benchmark/file-K.log", g_iFileSize, g_iRotatingFiles, .async=true, .policy=AsyncOverflowPolicy_OverrunOldest));

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "Multi thread", "overrun", "rotating file", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}

Action Command_Bench_MT_Overrun_ServerConsole(int client, int args)
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
    CreateTimer(1.5, Timer_Output_MT_Overrun_ServerConsole, data);
    return Plugin_Handled;
}

Action Timer_Output_MT_Overrun_ServerConsole(Handle timer, DataPack data)
{
    data.Reset();
    int iters = data.ReadCell();
    float delta = data.ReadFloat();
    delete data;

    PrintToServer("");
    PrintToServer("[benchmark-log4sp]  Mode %13s  |  Policy %7s  |  Sink %14s  |  Iters %7d  |  Elapsed %6.3f secs  %9d/sec",
                  "Multi thread", "overrun", "server console", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


stock float BenchLogger(int howmany, Logger logger)
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


