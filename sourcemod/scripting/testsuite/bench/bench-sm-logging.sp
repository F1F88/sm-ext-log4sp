#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <profiler>

enum
{
    CmdFuncs_LogMessage     = (1 << 0),
    CmdFuncs_LogToFile      = (1 << 1),
    CmdFuncs_LogToFileEx    = (1 << 2),
    CmdFuncs_PrintToServer  = (1 << 3),
    CmdFuncs_All            = (~0)
};


Profiler g_hProfiler = null;
int      g_iBenchRound = 0;


public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    RegConsoleCmd("sm_bench_sm_logging", CMD_Bench);

    g_hProfiler = new Profiler();

    BenchDB.Initialize();
}

public void OnPluginEnd()
{
    BenchDB.Destroy();
}


Action CMD_Bench(int client, int args)
{
    // sm_bench_sm_logging <calls> <functions> <fmts>
    //    calls: Integer - Default 1_000_000
    //    funcs: Integer - Default All(~0)
    //       LogMessage     1
    //       LogToFile      2
    //       LogToFileEx    4
    //       PrintToServer  8
    //    fmts: Boolean - Default false
    //       true           all
    //       false          mock
    //    echo: Boolean - Default false
    //       true           logecho
    //       false          silent
    int calls = (args >= 1) ? GetCmdArgInt(1) : 1_000_000;
    int funcs = (args >= 2) ? GetCmdArgInt(2) : CmdFuncs_All;
    bool fmts = (args >= 3) ? (!!GetCmdArgInt(3)) : false;
    bool echo = (args >= 4) ? (!!GetCmdArgInt(4)) : false;

    int val = FindConVar("sv_logecho").IntValue;
    FindConVar("sv_logecho").SetInt(echo ? 1 : 0); // 如果为 1, 运行时长会大幅增加

    BenchAll(calls, funcs, fmts);

    // 恢复原始值
    FindConVar("sv_logecho").SetInt(val);
    return Plugin_Handled;
}


void BenchAll(int calls, int funcs, bool fmts)
{
    g_iBenchRound++;

    if (funcs & CmdFuncs_LogMessage)
    {
        char filename[PLATFORM_MAX_PATH];
        FormatTime(filename, sizeof(filename), "addons/sourcemod/logs/L%Y%m%d.log");

        if (FileExists(filename))
            DeleteFile(filename);

        BenchLogMessage(calls, fmts);
    }

    if (funcs & CmdFuncs_LogToFile)
    {
        char filename[PLATFORM_MAX_PATH];
        BuildPath(Path_SM, filename, sizeof(filename), "logs/log-to-file.log");

        if (FileExists(filename))
            DeleteFile(filename);

        BenchLogToFile(filename, calls, fmts);
    }

    if (funcs & CmdFuncs_LogToFileEx)
    {
        char filename[PLATFORM_MAX_PATH];
        BuildPath(Path_SM, filename, sizeof(filename), "logs/log-to-file-ex.log");

        if (FileExists(filename))
            DeleteFile(filename);

        BenchLogToFileEx(filename, calls, fmts);
    }

    if (funcs & CmdFuncs_PrintToServer)
        BenchPrintToServer(calls, fmts);

    BenchDB.Instance().ExportToFile();
    BenchDB.Instance().PrintToServer();
}


void BenchLogMessage(int calls, bool fmts)
{
    // mock
    {
        int value = 777;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage("Hello logger: msg number %d", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", "mock", calls, g_hProfiler.Time);
    }

    if (!fmts)
        return;

    // Binary
    {
        char fmt[] = "%b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Integer
    {
        char fmt[] = "%d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer
    {
        char fmt[] = "%u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Float
    {
        char fmt[] = "%f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%6.3f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Special
    {
        char fmt[] = "%L";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%N";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%E";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // String
    {
        char fmt[] = "%s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%30s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Translation
    LoadTranslations("common.phrases");
    {
        char fmt[] = "%t";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%T";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value, LANG_SERVER);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Hex
    {
        char fmt[] = "%X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage("%X", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

#if defined SM_INT64_SUPPORTED
    // Binary 64
    {
        char fmt[] = "%lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Integer 64
    {
        char fmt[] = "%ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer 64
    {
        char fmt[] = "%lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }

    // Hex 64
    {
        char fmt[] = "%lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogMessage(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogMessage", fmt, calls, g_hProfiler.Time);
    }
#endif      // SM_INT64_SUPPORTED
}


void BenchLogToFile(const char[] filename, int calls, bool fmts)
{
    // mock
    {
        int value = 777;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, "Hello logger: msg number %d", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", "mock", calls, g_hProfiler.Time);
    }

    if (!fmts)
        return;

    // Binary
    {
        char fmt[] = "%b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Integer
    {
        char fmt[] = "%d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer
    {
        char fmt[] = "%u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Float
    {
        char fmt[] = "%f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%6.3f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Special
    {
        char fmt[] = "%L";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%N";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%E";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // String
    {
        char fmt[] = "%s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%30s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Translation
    LoadTranslations("common.phrases");
    {
        char fmt[] = "%t";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%T";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value, LANG_SERVER);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Hex
    {
        char fmt[] = "%X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, "%X", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

#if defined SM_INT64_SUPPORTED
    // Binary 64
    {
        char fmt[] = "%lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Integer 64
    {
        char fmt[] = "%ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer 64
    {
        char fmt[] = "%lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }

    // Hex 64
    {
        char fmt[] = "%lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFile(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFile", fmt, calls, g_hProfiler.Time);
    }
#endif      // SM_INT64_SUPPORTED
}

void BenchLogToFileEx(const char[] filename, int calls, bool fmts)
{
    // mock
    {
        int value = 777;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, "Hello logger: msg number %d", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", "mock", calls, g_hProfiler.Time);
    }

    if (!fmts)
        return;

    // Binary
    {
        char fmt[] = "%b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Integer
    {
        char fmt[] = "%d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer
    {
        char fmt[] = "%u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Float
    {
        char fmt[] = "%f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%6.3f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Special
    {
        char fmt[] = "%L";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%N";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%E";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // String
    {
        char fmt[] = "%s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%30s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Translation
    LoadTranslations("common.phrases");
    {
        char fmt[] = "%t";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%T";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value, LANG_SERVER);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Hex
    {
        char fmt[] = "%X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, "%X", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

#if defined SM_INT64_SUPPORTED
    // Binary 64
    {
        char fmt[] = "%lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Integer 64
    {
        char fmt[] = "%ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer 64
    {
        char fmt[] = "%lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }

    // Hex 64
    {
        char fmt[] = "%lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            LogToFileEx(filename, fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert("LogToFileEx", fmt, calls, g_hProfiler.Time);
    }
#endif      // SM_INT64_SUPPORTED
}

void BenchPrintToServer(int calls, bool fmts)
{
    char name[] = "PrintToServer";

    // mock
    {
        int value = 777;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer("Hello logger: msg number %d", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, "mock", calls, g_hProfiler.Time);
    }

    if (!fmts)
        return;

    // Binary
    {
        char fmt[] = "%b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032b";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Integer
    {
        char fmt[] = "%d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010d";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer
    {
        char fmt[] = "%u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010u";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Float
    {
        char fmt[] = "%f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%6.3f";
        float value = 3.1415926;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Special
    {
        char fmt[] = "%L";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%N";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%E";
        int value = 0;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // String
    {
        char fmt[] = "%s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%30s";
        char value[] = "some message...";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Translation
    LoadTranslations("common.phrases");
    {
        char fmt[] = "%t";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%T";
        char value[] = "Unable to target";
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value, LANG_SERVER);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Hex
    {
        char fmt[] = "%X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer("%X", value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08X";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08x";
        int value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

#if defined SM_INT64_SUPPORTED
    // Binary 64
    {
        char fmt[] = "%lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%032lb";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Integer 64
    {
        char fmt[] = "%ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010ld";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Unsigned Integer 64
    {
        char fmt[] = "%lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%010lu";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }

    // Hex 64
    {
        char fmt[] = "%lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lX";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
    {
        char fmt[] = "%08lx";
        int64 value = 1234567;
        g_hProfiler.Start();
        for (int i = 0; i < calls; ++i)
        {
            PrintToServer(fmt, value);
        }
        g_hProfiler.Stop();
        BenchDB.Instance().Insert(name, fmt, calls, g_hProfiler.Time);
    }
#endif      // SM_INT64_SUPPORTED
}


#define BENCH_DB_CONF   "storage-local"
#define BENCH_DB_TABLE  "bench_sm_logging"

static Database         __hDatabase = null;

enum struct BenchData
{
    int   num;      // round OR runs
    char  func[128];
    char  fmt[128];
    int   calls;
    float delta;
}

methodmap BenchDB
{
    public void CreateTable() {
        if (!SQL_FastQuery(this.db, "CREATE TABLE " ... BENCH_DB_TABLE ... " (round INTEGER NOT NULL, func VARCHAR(128) NOT NULL, fmt VARCHAR(128), calls INTEGER NOT NULL, delta REAL NOT NULL);")) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("Create table error %s.", error);
        }
    }

    // public void DeleteTable() {
    //     if (!SQL_FastQuery(this.db, "DELETE TABLE " ... BENCH_DB_TABLE ... ";")) {
    //         char error[256];
    //         SQL_GetError(this.db, error, sizeof(error));
    //         ThrowError("Delete table error %s.", error);
    //     }
    // }

    public void DropTable() {
        if (!SQL_FastQuery(this.db, "DROP TABLE IF EXISTS " ... BENCH_DB_TABLE ... ";")) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("Drop table error %s.", error);
        }
    }

    // <round, func, fmt, calls, delta>
    public DBResultSet SelectList() {
        // 所有测试的完整结果
        DBResultSet result = SQL_Query(this.db, "SELECT round, func, COALESCE(fmt, 'null'), calls, delta FROM " ... BENCH_DB_TABLE);
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectList error %s.", error);
        }
        return result;
    }

    // DBResultSet<round, func, fmt, calls, delta>
    public DBResultSet SelectMaxRound() {
        // 最近一次测试的完整结果
        DBResultSet result = SQL_Query(this.db, "SELECT round, func, COALESCE(fmt, 'null'), calls, delta FROM " ... BENCH_DB_TABLE ... " WHERE round = (SELECT MAX(round) FROM " ... BENCH_DB_TABLE ... ") GROUP BY func, fmt ORDER BY func, fmt;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectMaxRound error %s.", error);
        }
        return result;
    }

    // DBResultSet<runs, func, fmt, calls, delta>
    public DBResultSet SelectSumGroupyByFuncFmt() {
        // 所有 sink 不同 log 方法的不同 fmt 的测试结果
        DBResultSet result = SQL_Query(this.db, "SELECT COUNT(DISTINCT round), func, COALESCE(fmt, 'null'), SUM(calls), SUM(delta) FROM " ... BENCH_DB_TABLE ... " GROUP BY func, fmt ORDER BY func, fmt;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectSumGroupyByFmt error %s.", error);
        }
        return result;
    }

    // DBResultSet<runs, func, 'null', calls, delta>
    public DBResultSet SelectSumGroupyByFunc() {
        // 所有 sink 不同 log 方法的测试结果 (LogF 方法只取 mock fmt)
        DBResultSet result = SQL_Query(this.db, "SELECT COUNT(DISTINCT round), func, 'null', SUM(calls), SUM(delta) FROM " ... BENCH_DB_TABLE ... " WHERE (fmt IS NULL OR fmt = '' OR fmt = 'null' OR fmt = 'mock') GROUP BY func ORDER BY func;");
        if (!result) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("SelectSumGroupyByFunc error %s.", error);
        }
        return result;
    }

    // ArrayList<BenchData<num, func, fmt, calls, delta>>
    public ArrayList ParseResultSetToDatas(DBResultSet result) {
        if (!result)
            ThrowError("Invalid DBResultSet.");

        BenchData data;
        ArrayList datas = new ArrayList(.blocksize=sizeof(BenchData));

        for (int i = 0; result.FetchRow(); ++i) {
            data.num = result.FetchInt(0);
            result.FetchString(1, data.func, sizeof(BenchData::func));
            result.FetchString(2, data.fmt,  sizeof(BenchData::fmt));
            data.calls = result.FetchInt(3);
            data.delta = result.FetchFloat(4);
            datas.PushArray(data);
        }
        return datas;
    }

    public void Insert(const char[] func, const char[] fmt, int calls, float delta) {
        char query[512];
        FormatEx(query, sizeof(query),
                "INSERT INTO " ... BENCH_DB_TABLE ... " (round, func, fmt, calls, delta) VALUES (%d, '%s', '%s', %d, %f)",
                g_iBenchRound, func, fmt, calls, delta);

        if (!SQL_FastQuery(this.db, query)) {
            char error[256];
            SQL_GetError(this.db, error, sizeof(error));
            ThrowError("Insert data error %s.", error);
        }
    }

    public void ExportToFile(const char[] filename="logs/output-bench-sm-logging", const char[] ext="csv", const char[] sep=",") {
        {
            DBResultSet result = this.SelectList();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[%d] SelectList empty!", __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_all.%s", filename, ext);
            PrintToServer("Export SelectList to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("round%sfunc%sfmt%scalls%sdelta%ssecs", sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.func,  sep, data.fmt, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectMaxRound();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[Line::%d] SelectMaxRound empty!", __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_latest.%s", filename, ext);
            PrintToServer("Export SelectMaxRound to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("round%sfunc%sfmt%scalls%sdelta%ssecs", sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.func,  sep, data.fmt, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectSumGroupyByFuncFmt();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[Line::%d] SelectSumGroupyByFuncFmt empty!", __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_sum_group_by_func_fmt.%s", filename, ext);
            PrintToServer("Export SelectSumGroupyByFuncFmt to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("runs%sfunc%sfmt%scalls%sdelta%ssecs", sep, sep, sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%s%s%d%s%f%s%d",
                    data.num,   sep, data.func,  sep, data.fmt, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }

        {
            DBResultSet result = this.SelectSumGroupyByFunc();
            ArrayList   datas  = this.ParseResultSetToDatas(result);
            delete result;
            if (datas.Length == 0) {
                PrintToServer("[Line::%d] SelectSumGroupyByFunc empty!", __LINE__);
                delete datas;
                return;
            }

            char buffer[PLATFORM_MAX_PATH];
            BuildPath(Path_SM, buffer, sizeof(buffer), "%s_sum_group_by_func.%s", filename, ext);
            PrintToServer("Export SelectSumGroupyByFunc to file: \"%s\".", buffer);

            File file = OpenFile(buffer, "w");
            file.WriteLine("runs%sfunc%scalls%sdelta%ssecs", sep, sep, sep, sep);

            for (int i = 0; i < datas.Length; ++i) {
                BenchData data;
                datas.GetArray(i, data);
                file.WriteLine("%d%s%s%s%d%s%f%s%d",  data.num, sep, data.func, sep,
                    data.calls, sep, data.delta, sep, RoundToFloor(data.calls / data.delta));
            }
            delete datas;
            delete file;
        }
    }

    public void PrintToServer() {
        DBResultSet result = this.SelectSumGroupyByFunc();
        ArrayList   datas  = this.ParseResultSetToDatas(result);
        delete result;
        if (datas.Length == 0) {
            PrintToServer("[Line::%d] SelectSumGroupyByFunc empty!", __LINE__);
            delete datas;
            return;
        }

        BenchData data;
        datas.GetArray(0, data);

        for (int i = 0; i < datas.Length; ++i) {
            datas.GetArray(i, data);
            PrintToServer("[benchmark] %-15s Runs: %-2d   Calls: %-7d   Elapsed: %-8.3f %7d/sec",
                data.func, data.num, data.calls, data.delta, RoundToFloor(data.calls / data.delta));
        }
        delete datas;
    }

    public static void Initialize() {
        char error[256];
        __hDatabase = SQL_Connect(BENCH_DB_CONF, false, error, sizeof(error));
        if (!BenchDB.Instance().db)
            ThrowError("Creating an SQL connection for \"" ... BENCH_DB_CONF ... "\" failed (error: %s)", error);

        BenchDB.Instance().DropTable();
        BenchDB.Instance().CreateTable();
    }

    public static void Destroy() {
        BenchDB.Instance().DropTable();
        delete __hDatabase;
    }

    public static BenchDB Instance() {
        return view_as<BenchDB>(true);
    }

    property Database db {
        public get() { return __hDatabase; }
    }
};

#undef BENCH_DB_CONF
#undef BENCH_DB_TABLE
