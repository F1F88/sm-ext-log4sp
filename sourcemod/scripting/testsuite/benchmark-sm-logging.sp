#include <sourcemod>
#include <profiler>

#undef REQUIRE_EXTENSIONS
#include <log4sp>

#pragma semicolon 1
#pragma newdecls required


Profiler g_hProfiler = null;


public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    RegConsoleCmd("sm_log4sp_bench_logmessage",     CMD_Bench_LogMessage);
    RegConsoleCmd("sm_log4sp_bench_logtofile",      CMD_Bench_LogToFile);
    RegConsoleCmd("sm_log4sp_bench_logtofileEx",    CMD_Bench_LogToFileEx);
    RegConsoleCmd("sm_log4sp_bench_printtoserver",  CMD_Bench_PrintToServer);

    g_hProfiler = new Profiler();
}

Action CMD_Bench_LogMessage(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;
    int logEcho = (args >= 2) ? GetCmdArgInt(2) : 0;

    char BENCH_TITLE[]  = "LogMessage";

    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(logEcho); // 如果为 1, 运行时长会大幅增加

    // 测速
    g_hProfiler.Start();
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
            case 6:     LogMessage("|  6 |     34b:     %34b |      b:      %b |", i, -i);
            case 7:     LogMessage("|  7 |    034b:    %034b |      b:      %b |", -i, i);
            case 8:     LogMessage("|  8 |    -34b:    %-34b |      b:      %b |", i, -i);
            case 9:     LogMessage("|  9 |   -034b:   %-034b |      b:      %b |", -i, i);
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
            case 21:    LogMessage("| 21 |  16.10s:  %16.10s |   .10s:   %.10s |", "some messages", "some messages");
            case 22:    LogMessage("| 22 | -16.10s: %-16.10s |  -.10s:  %-.10s |", "some messages", "some messages");
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
    g_hProfiler.Stop();
    float delta = g_hProfiler.Time;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, iters, delta, RoundToFloor(iters / delta));

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}

Action CMD_Bench_LogToFile(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;
    int logEcho = (args >= 2) ? GetCmdArgInt(2) : 0;

    char BENCH_TITLE[]  = "LogToFile";
    char FILE_PATH[]    = "addons/sourcemod/logs/benchmark/file-Z.log";

    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(logEcho); // 如果为 1, 运行时长会大幅增加

    // 测速
    g_hProfiler.Start();
    for (int i = 0; i < iters; ++i)
    {
        switch (i & 31)
        {
            case 0:     LogToFile(FILE_PATH, "|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     LogToFile(FILE_PATH, "|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     LogToFile(FILE_PATH, "|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     LogToFile(FILE_PATH, "|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     LogToFile(FILE_PATH, "|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     LogToFile(FILE_PATH, "|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     LogToFile(FILE_PATH, "|  6 |     34b:     %34b |      b:      %b |", i, -i);
            case 7:     LogToFile(FILE_PATH, "|  7 |    034b:    %034b |      b:      %b |", -i, i);
            case 8:     LogToFile(FILE_PATH, "|  8 |    -34b:    %-34b |      b:      %b |", i, -i);
            case 9:     LogToFile(FILE_PATH, "|  9 |   -034b:   %-034b |      b:      %b |", -i, i);
            case 10:    LogToFile(FILE_PATH, "| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    LogToFile(FILE_PATH, "| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    LogToFile(FILE_PATH, "| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    LogToFile(FILE_PATH, "| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    LogToFile(FILE_PATH, "| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    LogToFile(FILE_PATH, "| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    LogToFile(FILE_PATH, "| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    LogToFile(FILE_PATH, "| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    LogToFile(FILE_PATH, "| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    LogToFile(FILE_PATH, "| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    LogToFile(FILE_PATH, "| 21 |  16.10s:  %16.10s |   .10s:   %.10s |", "some messages", "some messages");
            case 22:    LogToFile(FILE_PATH, "| 22 | -16.10s: %-16.10s |  -.10s:  %-.10s |", "some messages", "some messages");
            case 23:    LogToFile(FILE_PATH, "| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    LogToFile(FILE_PATH, "| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    LogToFile(FILE_PATH, "| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    LogToFile(FILE_PATH, "| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    LogToFile(FILE_PATH, "| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    LogToFile(FILE_PATH, "| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    LogToFile(FILE_PATH, "| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    LogToFile(FILE_PATH, "| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    LogToFile(FILE_PATH, "| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    g_hProfiler.Stop();
    float delta = g_hProfiler.Time;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, iters, delta, RoundToFloor(iters / delta));

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}

Action CMD_Bench_LogToFileEx(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;
    int logEcho = (args >= 2) ? GetCmdArgInt(2) : 0;

    char BENCH_TITLE[]  = "LogToFileEx";
    char FILE_PATH[]    = "addons/sourcemod/logs/benchmark/file-Y.log";

    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(logEcho); // 如果为 1, 运行时长会大幅增加

    // 测速
    g_hProfiler.Start();
    for (int i = 0; i < iters; ++i)
    {
        switch (i & 31)
        {
            case 0:     LogToFileEx(FILE_PATH, "|  0 |    010d:    %010d |    10d:    %10d | d: %d |", i, -i, i);
            case 1:     LogToFileEx(FILE_PATH, "|  1 |   -010i:   %-010i |   -10i:   %-10i | i: %i |", -i, i, -i);
            case 2:     LogToFileEx(FILE_PATH, "|  2 |    010u:    %010u |    10u:    %10u | u: %d |", i, -i, i);
            case 3:     LogToFileEx(FILE_PATH, "|  3 |   -010u:   %-010u |   -10u:   %-10u | u: %i |", -i, i, -i);
            case 4:     LogToFileEx(FILE_PATH, "|  4 |    010x:    %010x |    10x:    %10x | x: %x |", i, -i, i);
            case 5:     LogToFileEx(FILE_PATH, "|  5 |   -010x:   %-010x |   -10x:   %-10x | x: %x |", -i, i, -i);
            case 6:     LogToFileEx(FILE_PATH, "|  6 |     34b:     %34b |      b:      %b |", i, -i);
            case 7:     LogToFileEx(FILE_PATH, "|  7 |    034b:    %034b |      b:      %b |", -i, i);
            case 8:     LogToFileEx(FILE_PATH, "|  8 |    -34b:    %-34b |      b:      %b |", i, -i);
            case 9:     LogToFileEx(FILE_PATH, "|  9 |   -034b:   %-034b |      b:      %b |", -i, i);
            case 10:    LogToFileEx(FILE_PATH, "| 10 |     10f:     %10f |      f:      %f |", float(i), float(-i));
            case 11:    LogToFileEx(FILE_PATH, "| 11 |    010f:    %010f |      f:      %f |", float(-i), float(i));
            case 12:    LogToFileEx(FILE_PATH, "| 12 |   -010f:   %-010f |   -10f:   %-10f |", float(i), float(-i));
            case 14:    LogToFileEx(FILE_PATH, "| 14 |    0.3f:    %0.3f |    .3f:    %.3f |", float(-i), float(i));
            case 15:    LogToFileEx(FILE_PATH, "| 15 |   -0.3f:   %-0.3f |   -.3f:   %0.3f |", float(i), float(-i));
            case 16:    LogToFileEx(FILE_PATH, "| 16 |  010.3f:  %010.3f |  10.3f:  %10.3f |", float(-i), float(i));
            case 17:    LogToFileEx(FILE_PATH, "| 17 | -010.3f: %-010.3f | -10.3f: %-10.3f |", float(i), float(-i));
            case 18:    LogToFileEx(FILE_PATH, "| 18 | %% | %c | %c | %c | %c | %c | %c | %c |", 'a', 'b', 'c', 'd', 'e', 'f', 'g');
            case 19:    LogToFileEx(FILE_PATH, "| 19 |     10s:     %10s |      s:      %s |", "some messages", "some messages");
            case 20:    LogToFileEx(FILE_PATH, "| 20 |    -10s:    %-10s |      s:      %s |", "some messages", "some string messages");
            case 21:    LogToFileEx(FILE_PATH, "| 21 |  16.10s:  %16.10s |   .10s:   %.10s |", "some messages", "some messages");
            case 22:    LogToFileEx(FILE_PATH, "| 22 | -16.10s: %-16.10s |  -.10s:  %-.10s |", "some messages", "some messages");
            case 23:    LogToFileEx(FILE_PATH, "| 23 |     16t:     %16t |  0   t:      %t |", "See console for output", "See console for output");
            case 24:    LogToFileEx(FILE_PATH, "| 24 |    -16t:    %-16t | 1 d  t:      %t |", "See console for output", "Vote Delay Seconds", 234567890);
            case 25:    LogToFileEx(FILE_PATH, "| 25 |    .16t:    %.16t | 1 s  t:      %t |", "See console for output", "Unable to find cvar", "some_cvar");
            case 26:    LogToFileEx(FILE_PATH, "| 26 |  20.16t:  %20.16t | 1 N  t:      %t |", "See console for output", "Chat to admins", client);
            case 27:    LogToFileEx(FILE_PATH, "| 27 |   -.16t:   %-.16t | 2 N  t:      %t |", "See console for output", "Private say to", client, client);
            case 28:    LogToFileEx(FILE_PATH, "| 28 | -20.16t: %-20.16t | 2 s  t:      %t |", "See console for output", "Vote Select", "somebody", "somebuttom");
            case 29:    LogToFileEx(FILE_PATH, "| 29 |     16T:     %16T |  0   T:      %T |", "See console for output", client, "See console for output", client);
            case 30:    LogToFileEx(FILE_PATH, "| 30 |    -16T:    %-16T | 1 d  T:      %T |", "See console for output", client, "Vote Delay Seconds", client, 234567890);
            case 31:    LogToFileEx(FILE_PATH, "| 31 | -20.16T: %-20.16T | 2 s  T:      %T |", "See console for output", client, "Vote Select", client, "somebody", "somebuttom");
        }
    }
    g_hProfiler.Stop();
    float delta = g_hProfiler.Time;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, iters, delta, RoundToFloor(iters / delta));

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}

Action CMD_Bench_PrintToServer(int client, int args)
{
    int iters = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;

    char BENCH_TITLE[]  = "PrintToServer";

    // 测速
    g_hProfiler.Start();
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
            case 6:     PrintToServer("|  6 | [%s] |      34b:     %34b |      b:      %b |", "name-X", i, -i);
            case 7:     PrintToServer("|  7 | [%s] |     034b:    %034b |      b:      %b |", "name-X", -i, i);
            case 8:     PrintToServer("|  8 | [%s] |     -34b:    %-34b |      b:      %b |", "name-X", i, -i);
            case 9:     PrintToServer("|  9 | [%s] |    -034b:   %-034b |      b:      %b |", "name-X", -i, i);
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
            case 21:    PrintToServer("| 21 | [%s] |   16.10s:  %16.10s |   .10s:   %.10s |", "name-X", "some messages", "some messages");
            case 22:    PrintToServer("| 22 | [%s] |  -16.10s: %-16.10s |  -.10s:  %-.10s |", "name-X", "some messages", "some messages");
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
    g_hProfiler.Stop();
    float delta = g_hProfiler.Time;

    // 输出结果
    PrintToServer("");
    PrintToServer("[benchmark] %13s | Iters %7d | Elapsed %6.3f secs %9d/sec", BENCH_TITLE, iters, delta, RoundToFloor(iters / delta));
    return Plugin_Handled;
}


