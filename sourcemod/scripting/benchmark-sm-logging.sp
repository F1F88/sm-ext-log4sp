#include <sourcemod>
#include <profiler>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Benchmark-SMlogging"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Benchmark sourcemod logging library"
#define PLUGIN_URL              "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};


public void OnPluginStart()
{
    RegConsoleCmd("sm_log4sp_bench_logmessage",     CMD_Bench_LogMessage);
    RegConsoleCmd("sm_log4sp_bench_logtofile",      CMD_Bench_LogToFile);
    RegConsoleCmd("sm_log4sp_bench_logtofileEx",    CMD_Bench_LogToFileEx);
    RegConsoleCmd("sm_log4sp_bench_printtoserver",  CMD_Bench_PrintToServer);
}

Action CMD_Bench_LogMessage(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    int logEcho = 0;
    if (args >= 2)
    {
        logEcho = GetCmdArgInt(2);
    }
    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(logEcho); // 如果为 1, 运行时长会大幅增加

    // 测速
    Profiler profiler = new Profiler();
    profiler.Start();
    for (int i = 0; i < iters; ++i)
    {
        LogMessage("Hello log4sp: msg number %d", i);
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec",
                  "LogMessage", iters, delta, RoundToFloor(iters / delta));

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}

Action CMD_Bench_LogToFile(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    int logEcho = 0;
    if (args >= 2)
    {
        logEcho = GetCmdArgInt(2);
    }
    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(logEcho); // 如果为 1, 运行时长会大幅增加

    // 测速
    Profiler profiler = new Profiler();
    profiler.Start();
    for (int i = 0; i < iters; ++i)
    {
        LogToFile("logs/benchmark/file-Z.log", "Hello log4sp: msg number %d", i);
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec",
                  "LogToFile", iters, delta, RoundToFloor(iters / delta));

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}

Action CMD_Bench_LogToFileEx(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    int logEcho = 0;
    if (args >= 2)
    {
        logEcho = GetCmdArgInt(2);
    }
    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(logEcho); // 如果为 1, 运行时长会大幅增加

    // 测速
    Profiler profiler = new Profiler();
    profiler.Start();
    for (int i = 0; i < iters; ++i)
    {
        LogToFileEx("logs/benchmark/file-Y.log", "Hello log4sp: msg number %d", i);
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec",
                  "LogToFileEx", iters, delta, RoundToFloor(iters / delta));

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}

Action CMD_Bench_PrintToServer(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    // 测速
    Profiler profiler = new Profiler();
    profiler.Start();
    for (int i = 0; i < iters; ++i)
    {
        PrintToServer("[%s] [%s] [%s] Hello log4sp: msg number %d", "2024-08-01 12:34:56.789", "name-X", "info", i);
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec",
                  "PrintToServer", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


