#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_utils"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_update_sinks", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START UPDATE SINKS ----");

    TestUpdateSinks();

    PrintToServer("---- STOP UPDATE SINKS ----");
    return Plugin_Handled;
}


void TestUpdateSinks()
{
    SetTestContext("Test Update Sinks");

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("update-sinks/simple_file.log");

    Logger logger = new Logger("test-update-sinks");
    logger.SetPattern("%v");

    for (int i = 0; i < 7777; ++i)
    {
        BasicFileSink sink = new BasicFileSink(path);
        DailyFileSink sink2 = new DailyFileSink(path);

        logger.AddSink(sink);
        logger.AddSink(sink2);

        logger.InfoAmxTpl("Test message %d", i);

        logger.DropSink(sink);
        logger.DropSink(sink2);

        delete sink;
        delete sink2;
    }

    delete logger;

    AssertEq("Basic file, count lines", CountLines(path), 7777);

    FormatTime(path, sizeof(path), "update-sinks/simple_file_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);
    AssertEq("Daily file, count lines", CountLines(path), 7777);
}
