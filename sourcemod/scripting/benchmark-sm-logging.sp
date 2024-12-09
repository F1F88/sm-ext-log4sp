#include <sourcemod>
#include <profiler>

#define LOG4sp_NO_EXT
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
    LoadTranslations("common.phrases");

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
        switch (i & 31)
        {
            case 0:     LogMessage("|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     LogMessage("|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     LogMessage("|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     LogMessage("|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     LogMessage("|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     LogMessage("|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     LogMessage("|  6 |     34b:     %34b |      b:      %b |", float(i), float(-i));
            case 7:     LogMessage("|  7 |    034b:    %034b |      b:      %b |", float(-i), float(i));
            case 8:     LogMessage("|  8 |    -34b:    %-34b |      b:      %b |", float(i), float(-i));
            case 9:     LogMessage("|  9 |   -034b:   %-034b |      b:      %b |", float(-i), float(i));
            case 10:    LogMessage("| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    LogMessage("| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    LogMessage("| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    LogMessage("| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    LogMessage("| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    LogMessage("| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    LogMessage("| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    LogMessage("| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    LogMessage("| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    LogMessage("| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    LogMessage("| 21 |  16.10s:  %16.10s |   .10s:   %.10f |", "some messages", "some messages");
            case 22:    LogMessage("| 22 | -16.10s: %-16.10s |  -.10s:  %-.10f |", "some messages", "some messages");
            case 23:    LogMessage("| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    LogMessage("| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    LogMessage("| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    LogMessage("| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    LogMessage("| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    LogMessage("| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    LogMessage("| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    LogMessage("| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    LogMessage("| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", "LogMessage", iters, delta, RoundToFloor(iters / delta));

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
        switch (i & 31)
        {
            case 0:     LogToFile("logs/benchmark/file-Z.log", "|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     LogToFile("logs/benchmark/file-Z.log", "|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     LogToFile("logs/benchmark/file-Z.log", "|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     LogToFile("logs/benchmark/file-Z.log", "|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     LogToFile("logs/benchmark/file-Z.log", "|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     LogToFile("logs/benchmark/file-Z.log", "|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     LogToFile("logs/benchmark/file-Z.log", "|  6 |     34b:     %34b |      b:      %b |", float(i), float(-i));
            case 7:     LogToFile("logs/benchmark/file-Z.log", "|  7 |    034b:    %034b |      b:      %b |", float(-i), float(i));
            case 8:     LogToFile("logs/benchmark/file-Z.log", "|  8 |    -34b:    %-34b |      b:      %b |", float(i), float(-i));
            case 9:     LogToFile("logs/benchmark/file-Z.log", "|  9 |   -034b:   %-034b |      b:      %b |", float(-i), float(i));
            case 10:    LogToFile("logs/benchmark/file-Z.log", "| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    LogToFile("logs/benchmark/file-Z.log", "| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    LogToFile("logs/benchmark/file-Z.log", "| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    LogToFile("logs/benchmark/file-Z.log", "| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    LogToFile("logs/benchmark/file-Z.log", "| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    LogToFile("logs/benchmark/file-Z.log", "| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    LogToFile("logs/benchmark/file-Z.log", "| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    LogToFile("logs/benchmark/file-Z.log", "| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    LogToFile("logs/benchmark/file-Z.log", "| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    LogToFile("logs/benchmark/file-Z.log", "| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    LogToFile("logs/benchmark/file-Z.log", "| 21 |  16.10s:  %16.10s |   .10s:   %.10f |", "some messages", "some messages");
            case 22:    LogToFile("logs/benchmark/file-Z.log", "| 22 | -16.10s: %-16.10s |  -.10s:  %-.10f |", "some messages", "some messages");
            case 23:    LogToFile("logs/benchmark/file-Z.log", "| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    LogToFile("logs/benchmark/file-Z.log", "| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    LogToFile("logs/benchmark/file-Z.log", "| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    LogToFile("logs/benchmark/file-Z.log", "| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    LogToFile("logs/benchmark/file-Z.log", "| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    LogToFile("logs/benchmark/file-Z.log", "| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    LogToFile("logs/benchmark/file-Z.log", "| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    LogToFile("logs/benchmark/file-Z.log", "| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    LogToFile("logs/benchmark/file-Z.log", "| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", "LogToFile", iters, delta, RoundToFloor(iters / delta));

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
        switch (i & 31)
        {
            case 0:     LogToFileEx("logs/benchmark/file-Y.log", "|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     LogToFileEx("logs/benchmark/file-Y.log", "|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     LogToFileEx("logs/benchmark/file-Y.log", "|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     LogToFileEx("logs/benchmark/file-Y.log", "|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     LogToFileEx("logs/benchmark/file-Y.log", "|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     LogToFileEx("logs/benchmark/file-Y.log", "|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     LogToFileEx("logs/benchmark/file-Y.log", "|  6 |     34b:     %34b |      b:      %b |", float(i), float(-i));
            case 7:     LogToFileEx("logs/benchmark/file-Y.log", "|  7 |    034b:    %034b |      b:      %b |", float(-i), float(i));
            case 8:     LogToFileEx("logs/benchmark/file-Y.log", "|  8 |    -34b:    %-34b |      b:      %b |", float(i), float(-i));
            case 9:     LogToFileEx("logs/benchmark/file-Y.log", "|  9 |   -034b:   %-034b |      b:      %b |", float(-i), float(i));
            case 10:    LogToFileEx("logs/benchmark/file-Y.log", "| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    LogToFileEx("logs/benchmark/file-Y.log", "| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    LogToFileEx("logs/benchmark/file-Y.log", "| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    LogToFileEx("logs/benchmark/file-Y.log", "| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    LogToFileEx("logs/benchmark/file-Y.log", "| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    LogToFileEx("logs/benchmark/file-Y.log", "| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    LogToFileEx("logs/benchmark/file-Y.log", "| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    LogToFileEx("logs/benchmark/file-Y.log", "| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    LogToFileEx("logs/benchmark/file-Y.log", "| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    LogToFileEx("logs/benchmark/file-Y.log", "| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    LogToFileEx("logs/benchmark/file-Y.log", "| 21 |  16.10s:  %16.10s |   .10s:   %.10f |", "some messages", "some messages");
            case 22:    LogToFileEx("logs/benchmark/file-Y.log", "| 22 | -16.10s: %-16.10s |  -.10s:  %-.10f |", "some messages", "some messages");
            case 23:    LogToFileEx("logs/benchmark/file-Y.log", "| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    LogToFileEx("logs/benchmark/file-Y.log", "| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    LogToFileEx("logs/benchmark/file-Y.log", "| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    LogToFileEx("logs/benchmark/file-Y.log", "| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    LogToFileEx("logs/benchmark/file-Y.log", "| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    LogToFileEx("logs/benchmark/file-Y.log", "| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    LogToFileEx("logs/benchmark/file-Y.log", "| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    LogToFileEx("logs/benchmark/file-Y.log", "| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    LogToFileEx("logs/benchmark/file-Y.log", "| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", "LogToFileEx", iters, delta, RoundToFloor(iters / delta));

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
        switch (i & 31)
        {
            case 0:     PrintToServer("|  0 | [%s] |     010d:    %010d |    10d:    %10d | d: %d |", "name-X", i, -i, i);
            case 1:     PrintToServer("|  1 | [%s] |    -010i:   %-010i |   -10i:   %-10i | i: %i |", "name-X", -i, i, -i);
            case 2:     PrintToServer("|  2 | [%s] |     010u:    %010u |    10u:    %10u | u: %d |", "name-X", i, -i, i);
            case 3:     PrintToServer("|  3 | [%s] |    -010u:   %-010u |   -10u:   %-10u | u: %i |", "name-X", -i, i, -i);
            case 4:     PrintToServer("|  4 | [%s] |     010x:    %010x |    10x:    %10x | x: %x |", "name-X", i, -i, i);
            case 5:     PrintToServer("|  5 | [%s] |    -010x:   %-010x |   -10x:   %-10x | x: %x |", "name-X", -i, i, -i);
            case 6:     PrintToServer("|  6 | [%s] |      34b:     %34b |      b:      %b |", "name-X", float(i), float(-i));
            case 7:     PrintToServer("|  7 | [%s] |     034b:    %034b |      b:      %b |", "name-X", float(-i), float(i));
            case 8:     PrintToServer("|  8 | [%s] |     -34b:    %-34b |      b:      %b |", "name-X", float(i), float(-i));
            case 9:     PrintToServer("|  9 | [%s] |    -034b:   %-034b |      b:      %b |", "name-X", float(-i), float(i));
            case 10:    PrintToServer("| 10 | [%s] |      10f:     %10f |      f:      %f |", "name-X", float(i), float(-i));
            case 11:    PrintToServer("| 11 | [%s] |     010f:    %010f |      f:      %f |", "name-X", float(-i), float(i));
            case 12:    PrintToServer("| 12 | [%s] |    -010f:   %-010f |   -10f:   %-10f |", "name-X", float(i), float(-i));
            case 14:    PrintToServer("| 14 | [%s] |     0.3f:    %0.3f |    .3f:    %.3f |", "name-X", float(-i), float(i));
            case 15:    PrintToServer("| 15 | [%s] |    -0.3f:   %-0.3f |   -.3f:   %0.3f |", "name-X", float(i), float(-i));
            case 16:    PrintToServer("| 16 | [%s] |   010.3f:  %010.3f |  10.3f:  %10.3f |", "name-X", float(-i), float(i));
            case 17:    PrintToServer("| 17 | [%s] |  -010.3f: %-010.3f | -10.3f: %-10.3f |", "name-X", float(i), float(-i));
            case 18:    PrintToServer("| 18 | [%s] |  %% | %c | %c | %c | %c | %c | %c | %c |", "name-X", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    PrintToServer("| 19 | [%s] |      10s:     %10s |      s:      %s |", "name-X", "some messages", "some messages");
            case 20:    PrintToServer("| 20 | [%s] |     -10s:    %-10s |      s:      %s |", "name-X", "some messages", "some string messages");
            case 21:    PrintToServer("| 21 | [%s] |   16.10s:  %16.10s |   .10s:   %.10f |", "name-X", "some messages", "some messages");
            case 22:    PrintToServer("| 22 | [%s] |  -16.10s: %-16.10s |  -.10s:  %-.10f |", "name-X", "some messages", "some messages");
            case 23:    PrintToServer("| 23 | [%s] |      16t:     %16t |  0   t:      %t |", "name-X", "See console for output", "See console for output");
            case 24:    PrintToServer("| 24 | [%s] |     -16t:    %-16t | 1 d  t:      %t |", "name-X", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    PrintToServer("| 25 | [%s] |     .16t:    %.16t | 1 s  t:      %t |", "name-X", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    PrintToServer("| 26 | [%s] |   20.16t:  %20.16t | 1 N  t:      %t |", "name-X", "See console for output", "Chat to admins", client);
            case 27:    PrintToServer("| 27 | [%s] |    -.16t:   %-.16t | 2 N  t:      %t |", "name-X", "See console for output", "Private say to", client, client);
            case 28:    PrintToServer("| 28 | [%s] |  -20.16t: %-20.16t | 2 s  t:      %t |", "name-X", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    PrintToServer("| 29 | [%s] |      16T:     %16T |  0   T:      %T |", "name-X", "See console for output", client, "See console for output", client);
            case 30:    PrintToServer("| 30 | [%s] |     -16T:    %-16T | 1 d  T:      %T |", "name-X", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    PrintToServer("| 31 | [%s] |  -20.16T: %-20.16T | 2 s  T:      %T |", "name-X", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    profiler.Stop();
    float delta = profiler.Time;
    delete profiler;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", "PrintToServer", iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


