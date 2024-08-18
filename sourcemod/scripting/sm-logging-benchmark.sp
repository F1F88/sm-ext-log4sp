#include <sourcemod>
#include <profiler>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "SM-Logging-Benchmark"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Sourcemod Logging lib Benchmark"
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
    RegConsoleCmd("sm_log4sp_bench_sm_logging", CMD_BenchSMLogging);
    RegConsoleCmd("sm_log4sp_bench_sm_console", CMD_BenchSMConsole);
}


Action CMD_BenchSMLogging(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    ConVar convar = FindConVar("sv_logecho");
    int val = convar.IntValue;
    convar.SetInt(0); // 如果为 1, 运行时长会大幅增加

    PrintToServer("[log4sp-benchmark] **************************************************************");
    PrintToServer("[log4sp-benchmark] Sourcemod Logging API, %d iterations", iters);
    PrintToServer("[log4sp-benchmark] **************************************************************");

    BenchLogMessage(iters);
    BenchLogToFile(iters,   "logs/benchmark/LogToFile.log");
    BenchLogToFileEx(iters, "logs/benchmark/LogToFileEx.log");

    convar.SetInt(val);
    return Plugin_Handled;
}

Action CMD_BenchSMConsole(int client, int args)
{
    int iters = 1_000_000;
    if (args >= 1)
    {
        iters = GetCmdArgInt(1);
    }

    ConVar convar = FindConVar("sv_logecho");
    int val = convar.IntValue;
    convar.SetInt(0);

    BenchPrintToServer(iters);

    convar.SetInt(val);
    return Plugin_Handled;
}


void BenchLogMessage(int howmany)
{
    ConVar convar = FindConVar("sv_logecho");
    int val = convar.IntValue;
    convar.SetInt(0);

    Profiler profiler = new Profiler();

    profiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        LogMessage("Hello log4sp: msg number %d", i);
    }
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    int delta_d = RoundToFloor(howmany / delta);

    PrintToServer("[log4sp-benchmark] %-24s Elapsed: %5.2f secs %12d /sec", "LogMessage", delta, delta_d);

    convar.SetInt(val);
}

void BenchLogToFile(int howmany, const char[] file)
{
    Profiler profiler = new Profiler();

    profiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        LogToFile(file, "Hello log4sp: msg number %d", i);
    }
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    int delta_d = RoundToFloor(howmany / delta);

    PrintToServer("[log4sp-benchmark] %-24s Elapsed: %5.2f secs %12d /sec", "LogToFile", delta, delta_d);
}

void BenchLogToFileEx(int howmany, const char[] file)
{
    Profiler profiler = new Profiler();

    profiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        LogToFileEx(file, "Hello log4sp: msg number %d", i);
    }
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    int delta_d = RoundToFloor(howmany / delta);

    PrintToServer("[log4sp-benchmark] %-24s Elapsed: %5.2f secs %12d /sec", "LogToFileEx", delta, delta_d);
}

void BenchPrintToServer(int howmany)
{
    Profiler profiler = new Profiler();

    profiler.Start();
    for (int i = 0; i < howmany; ++i)
    {
        PrintToServer("[%s] [sm-server-console_____] Hello log4sp: msg number %d", "2024-08-01 12:34:56", i);
    }
    profiler.Stop();

    float delta = profiler.Time;
    delete profiler;

    int delta_d = RoundToFloor(howmany / delta);

    PrintToServer("[log4sp-benchmark] **************************************************************");
    PrintToServer("[log4sp-benchmark] Sourcemod Console API, %d iterations", howmany);
    PrintToServer("[log4sp-benchmark] **************************************************************");
    PrintToServer("[log4sp-benchmark] %-24s Elapsed: %5.2f secs %12d /sec", "PrintToServer", delta, delta_d);
}



/**
 * sm_log4sp_bench_sm_logging
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] Sourcemod Logging API, 1000000 iterations
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] LogMessage               Elapsed: 10.99 secs        90979 /sec
 * [log4sp-benchmark] LogToFile                Elapsed:  8.91 secs       112111 /sec
 * [log4sp-benchmark] LogToFileEx              Elapsed:  9.07 secs       110141 /sec
 *
 * sm_log4sp_bench_sm_console
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] Sourcemod Console API, 1000000 iterations
 * [log4sp-benchmark] **************************************************************
 * [log4sp-benchmark] PrintToServer            Elapsed:  5.86 secs       170446 /sec
 */
