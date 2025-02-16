#include <testing>

#include <log4sp>

#include "test_sink"

#pragma semicolon 1
#pragma newdecls required


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_commands", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START COMMANDS ----");

    RequestFrame(TestCommands);

    PrintToServer("---- STOP COMMANDS ----");
    return Plugin_Handled;
}


void TestCommands()
{
    SetTestContext("Test Commands");
    if (Logger.Get("test-commands"))
        delete Logger.Get("test-commands");
    Logger logger = ServerConsoleSink.CreateLogger("test-commands");

    char buffer[TEST_MAX_MSG_LENGTH];

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp list");
    AssertStrRegexEq("Commands list", buffer, "\\[SM\\] List of all logger names: \\[log4sp, test-commands\\]\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp apply_all list");
    AssertStrRegexEq("Commands apply_all", buffer, "\\[SM\\] Command function name \"list\" not exists\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp apply_all get_lvl");
    AssertStrRegexEq("Commands apply_all", buffer, "\\[SM\\] Logger 'log4sp' log level is '.*'\\.\\s\\[SM\\] Logger 'test-commands' log level is 'info'\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp get_lvl test-commands");
    AssertStrRegexEq("Commands get_lvl", buffer, "\\[SM\\] Logger 'test-commands' log level is 'info'\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_lvl test-commands trace");
    AssertStrRegexEq("Commands set_lvl", buffer, "\\[SM\\] Logger 'test-commands' will set log level to 'trace'\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_lvl test-commands 1");
    AssertStrRegexEq("Commands set_lvl", buffer, "\\[SM\\] Logger 'test-commands' will set log level to 'debug'\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_pattern test-commands %%v");
    AssertStrRegexEq("Commands set_pattern", buffer, "\\[SM\\] Logger 'test-commands' will set log pattern to '%v'\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp should_log test-commands trace");
    AssertStrRegexEq("Commands should_log", buffer, "\\[SM\\] Logger 'test-commands' has disabled 'trace' log level\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp should_log test-commands debug");
    AssertStrRegexEq("Commands should_log", buffer, "\\[SM\\] Logger 'test-commands' has enabled 'debug' log level\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp log test-commands debug \"Hello Log4sp.\"");
    AssertStrRegexEq("Commands log", buffer, "\\[SM\\] Logger 'test-commands' will log a message 'Hello Log4sp\\.' with log level 'debug'\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp flush test-commands");
    AssertStrRegexEq("Commands flush", buffer, "\\[SM\\] Logger 'test-commands' will flush its contents\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp get_flush_lvl test-commands");
    AssertStrRegexEq("Commands get_flush_lvl", buffer, "\\[SM\\] Logger 'test-commands' flush level is 'off'\\.\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_flush_lvl test-commands trace");
    AssertStrRegexEq("Commands set_flush_lvl", buffer, "\\[SM\\] Logger 'test-commands' will set flush level to 'trace'\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_flush_lvl test-commands 1");
    AssertStrRegexEq("Commands set_flush_lvl", buffer, "\\[SM\\] Logger 'test-commands' will set flush level to 'debug'\\s");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp version");
    AssertStrRegexEq("Commands set_flush_lvl", buffer, "Sourcemod extension Log4sp version information:\\s    Version         [0-9]+\\.[0-9]+\\.[0-9]+.*");

    delete logger;
}
