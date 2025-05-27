#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_sink"
#include "test_utils"


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

    Logger logger = ServerConsoleSink.CreateLogger("test-commands");

    char buffer[2048];

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp list");
    AssertStrMatch("Commands list match", buffer, "\\[SM\\] List of all logger names: \\[.*log4sp.*\\]\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp apply_all list");
    AssertStrMatch("Commands apply_all match", buffer, "\\[SM\\] Command function name \"list\" not exists\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp apply_all get_lvl");
    AssertStrMatch("Commands apply_all match", buffer, "[\\[SM\\] Logger '.*' log level is '(trace|debug|info|warn|error|fatal|off)'\\.(\n|\r\n)]+");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp get_lvl test-commands");
    AssertStrMatch("Commands get_lvl match", buffer, "\\[SM\\] Logger 'test-commands' log level is 'info'\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_lvl test-commands trace");
    AssertStrMatch("Commands set_lvl match", buffer, "\\[SM\\] Logger 'test-commands' will set log level to 'trace'(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_lvl test-commands 1");
    AssertStrMatch("Commands set_lvl match", buffer, "\\[SM\\] Logger 'test-commands' will set log level to 'debug'(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_pattern test-commands %%v");
    AssertStrMatch("Commands set_pattern match", buffer, "\\[SM\\] Logger 'test-commands' will set log pattern to '%v'(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp should_log test-commands trace");
    AssertStrMatch("Commands should_log match", buffer, "\\[SM\\] Logger 'test-commands' has disabled 'trace' log level\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp should_log test-commands debug");
    AssertStrMatch("Commands should_log match", buffer, "\\[SM\\] Logger 'test-commands' has enabled 'debug' log level\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp log test-commands debug \"Hello Log4sp.\"");
    AssertStrMatch("Commands log match", buffer, "\\[SM\\] Logger 'test-commands' will log a message 'Hello Log4sp\\.' with log level 'debug'\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp flush test-commands");
    AssertStrMatch("Commands flush match", buffer, "\\[SM\\] Logger 'test-commands' will flush its contents\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp get_flush_lvl test-commands");
    AssertStrMatch("Commands get_flush_lvl match", buffer, "\\[SM\\] Logger 'test-commands' flush level is 'off'\\.(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_flush_lvl test-commands trace");
    AssertStrMatch("Commands set_flush_lvl match", buffer, "\\[SM\\] Logger 'test-commands' will set flush level to 'trace'(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp set_flush_lvl test-commands 1");
    AssertStrMatch("Commands set_flush_lvl match", buffer, "\\[SM\\] Logger 'test-commands' will set flush level to 'debug'(\n|\r\n)");

    ServerCommandEx(buffer, sizeof(buffer), "sm log4sp version");
    AssertStrMatch("Commands set_flush_lvl match", buffer, "SourceMod extension log4sp version information:\\s+ Version .*[0-9]+\\.[0-9]+\\.[0-9]+.*\\s+ Compiled on .* [0-9]+ [0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}\\s+ Built from \\s+ https://github.com/F1F88/sm-ext-log4sp/commit/.*");

    delete logger;
}
