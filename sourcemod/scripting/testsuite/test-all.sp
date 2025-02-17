#include <log4sp>

#include "test_utils"

#pragma semicolon 1
#pragma newdecls required


static const char g_sCommands[][] = {
    "sm_log4sp_test_basic_file_logger",
    "sm_log4sp_test_callback_logger",
    "sm_log4sp_test_daily_logger",
    "sm_log4sp_test_log_level",
    "sm_log4sp_test_format",
    "sm_log4sp_test_logger_err_handler",
    "sm_log4sp_test_ringbuffer_logger",
    "sm_log4sp_test_rotate_logger",
    "sm_log4sp_test_server_console_logger",
    "sm_log4sp_test_update_sinks",
};

public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_all", Command_Test);
}

Action Command_Test(int args)
{
    RequestFrame(ExecTestAll);
    return Plugin_Handled;
}


void ExecTestAll()
{
    PrintToServer("======= START TEST ALL =======");

    PrepareTestPath("./");

    for (int index = 0; index < sizeof(g_sCommands); ++index)
    {
        ServerCommand(g_sCommands[index]);
        ServerExecute();
    }

    PrintToServer("======= STOP TEST ALL =======");
}




