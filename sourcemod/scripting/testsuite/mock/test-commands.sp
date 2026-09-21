#pragma semicolon 1
#pragma newdecls required

#if !defined DEBUG
    #define  DEBUG
#endif

#if !defined _DEBUG
    #define  _DEBUG
#endif

#if defined NDEBUG
    #undef  NDEBUG
#endif

#include <regex>
#include <sourcemod>
#include <log4sp>

#undef  REQUIRE_PLUGIN
#include <log4sp/registry>
#define REQUIRE_PLUGIN

#include "../assert"
#include "../test_sink"
#include "../test_utils"

#if !defined LOGGER_NAME
    #define  LOGGER_NAME "test-commands"
#endif


public void OnAllPluginsLoaded()
{
    Test();
    RegServerCmd("sm_log4sp_test_commands", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------------ Started testing Commands ------------");

    SetTestContext("Commands");

    AssertTrue("[log4sp manager library]", Log4spRegistry.LibraryExists());
    RegisterLogger();

    RequestFrame(TestCommandLog);
    RequestFrame(TestCommandShouldLog);
    RequestFrame(TestCommandGetLvl);
    RequestFrame(TestCommandSetLvl);
    RequestFrame(TestCommandSetPattern);
    RequestFrame(TestCommandFlush);
    RequestFrame(TestCommandShouldFlush);
    RequestFrame(TestCommandGetFlushLvl);
    RequestFrame(TestCommandSetFlushLvl);
    RequestFrame(TestCommandApplyAll);
    RequestFrame(TestCommandList);
    RequestFrame(TestCommandVersion);
    RequestFrame(DropLogger);

    PrintToServer("--------------- Test Commands ended --------------");
}

void TestCommandLog()
{
    TestSink sink = GetTestSinkFromTestLogger();
    Log4spRegistry.Instance().Get(LOGGER_NAME);

    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp log "...LOGGER_NAME..." trace \"Hello Log4sp 1.\"");
    AssertStrMatch("[log]", buffer, "Logger \""...LOGGER_NAME..."\" log a \"trace\" level message \"Hello Log4sp 1\\.\"\\.[^\\S ]");
    AssertEq("[log]", sink.GetLogCount(), 0);

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp log "...LOGGER_NAME..." 2 \"Hello Log4sp 2.\"");
    AssertStrMatch("[log]", buffer, "Logger \""...LOGGER_NAME..."\" log a \"info\" level message \"Hello Log4sp 2\\.\"\\.[^\\S ]");
    AssertEq("[log]", sink.GetLogCount(), 1);

    SinkCleanupAndDelete(sink);
}

void TestCommandShouldLog()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp should_log "...LOGGER_NAME..." debug");
    AssertStrMatch("[should_log]", buffer, "Logger \""...LOGGER_NAME..."\" are not enabled logging for \"debug\" level\\.[^\\S ]");
    AssertFalse("[should_log]", Log4spRegistry.Instance().Get(LOGGER_NAME).ShouldLog(LogLevel_Debug));

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp should_log "...LOGGER_NAME..." 3");
    AssertStrMatch("[should_log]", buffer, "Logger \""...LOGGER_NAME..."\" are enabled logging for \"warn\" level\\.[^\\S ]");
    AssertTrue("[should_log]", Log4spRegistry.Instance().Get(LOGGER_NAME).ShouldLog(LogLevel_Warn));
}

void TestCommandGetLvl()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp get_lvl " ... LOGGER_NAME);
    AssertStrMatch("[get_lvl]", buffer, "Logger \""...LOGGER_NAME..."\" log level is \"info\"\\.[^\\S ]");
    AssertEq("[get_lvl]", Log4spRegistry.Instance().Get(LOGGER_NAME).GetLevel(), LogLevel_Info);
}

void TestCommandSetLvl()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp set_lvl "...LOGGER_NAME..." info");
    AssertStrMatch("[set_lvl]", buffer, "Logger \""...LOGGER_NAME..."\" set log level to \"info\"\\. \\(original: \"info\"\\)[^\\S ]");
    AssertEq("[set_lvl]", Log4spRegistry.Instance().Get(LOGGER_NAME).GetLevel(), LogLevel_Info);

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp set_lvl "...LOGGER_NAME..." 4");
    AssertStrMatch("[set_lvl]", buffer, "Logger \""...LOGGER_NAME..."\" set log level to \"error\"\\. \\(original: \"info\"\\)[^\\S ]");
    AssertEq("[set_lvl]", Log4spRegistry.Instance().Get(LOGGER_NAME).GetLevel(), LogLevel_Error);
}

void TestCommandSetPattern()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp set_pattern "...LOGGER_NAME..." %%v");
    AssertStrMatch("[set_pattern]", buffer, "Logger \""...LOGGER_NAME..."\" set pattern to \"%v\"\\.[^\\S ]");
}

void TestCommandFlush()
{
    TestSink sink = GetTestSinkFromTestLogger();

    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp flush " ... LOGGER_NAME);
    AssertStrMatch("[flush]", buffer, "Logger \""...LOGGER_NAME..."\" flush its contents\\.[^\\S ]");
    AssertEq("[flush]", sink.GetFlushCount(), 1);

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp flush " ... LOGGER_NAME);
    AssertStrMatch("[flush]", buffer, "Logger \""...LOGGER_NAME..."\" flush its contents\\.[^\\S ]");
    AssertEq("[flush]", sink.GetFlushCount(), 2);

    SinkCleanupAndDelete(sink);
}

void TestCommandShouldFlush()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp should_flush "...LOGGER_NAME..." warn");
    AssertStrMatch("[should_flush]", buffer, "Logger \""...LOGGER_NAME..."\" are not trigger automatic flush for \"warn\" level\\.[^\\S ]");
    AssertFalse("[should_flush]", Log4spRegistry.Instance().Get(LOGGER_NAME).ShouldFlush(LogLevel_Warn));

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp should_flush "...LOGGER_NAME..." 5");
    AssertStrMatch("[should_flush]", buffer, "Logger \""...LOGGER_NAME..."\" are not trigger automatic flush for \"fatal\" level\\.[^\\S ]");
    AssertFalse("[should_flush]", Log4spRegistry.Instance().Get(LOGGER_NAME).ShouldFlush(LogLevel_Fatal));
}

void TestCommandGetFlushLvl()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp get_flush_lvl " ... LOGGER_NAME);
    AssertStrMatch("[get_flush_lvl]", buffer, "Logger \""...LOGGER_NAME..."\" flush level is \"off\"\\.[^\\S ]");
    AssertEq("[get_flush_lvl]", Log4spRegistry.Instance().Get(LOGGER_NAME).GetFlushLevel(), LogLevel_Off);
}

void TestCommandSetFlushLvl()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp set_flush_lvl "...LOGGER_NAME..." error");
    AssertStrMatch("[set_flush_lvl]", buffer, "Logger \""...LOGGER_NAME..."\" set flush level to \"error\"\\. \\(original: \"off\"\\)[^\\S ]");
    AssertEq("[set_flush_lvl]", Log4spRegistry.Instance().Get(LOGGER_NAME).GetFlushLevel(), LogLevel_Error);

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp set_flush_lvl "...LOGGER_NAME..." 6");
    AssertStrMatch("[set_flush_lvl]", buffer, "Logger \""...LOGGER_NAME..."\" set flush level to \"off\"\\. \\(original: \"error\"\\)[^\\S ]");
    AssertEq("[set_flush_lvl]", Log4spRegistry.Instance().Get(LOGGER_NAME).GetFlushLevel(), LogLevel_Off);
}

void TestCommandApplyAll()
{
    TestSink sink = GetTestSinkFromTestLogger();

    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp apply_all log");
    AssertStrMatch("[apply_all log]", buffer, "\\[SM\\] Usage: sm_log4sp apply_all log <level> \\[message\\][^\\S ]");
    AssertEq("[apply_all log]", sink.GetLogCount(), 1);

    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp apply_all log fatal message");
    AssertStrMatch("[apply_all log]", buffer, "[\\[SM\\] Logger \".*\" log a \"fatal\" level message \".*\".[^\\S ]]+");
    AssertEq("[apply_all log]", sink.GetLogCount(), 2);

    SinkCleanupAndDelete(sink);
}

void TestCommandList()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp list");
    AssertStrMatch("[list]", buffer, "List of all logger names: \\[.*\\]*\\.[^\\S ]");
}

void TestCommandVersion()
{
    char buffer[1024];
    ServerCommandEx(buffer, sizeof(buffer), "sm_log4sp version");
    AssertStrMatch("[version]", buffer, " Log4sp version information:[^\\S]    Version: 2\\.[0-9]+\\.[0-9]+[^\\S]    Build time:.*[^\\S]    Build tags: .*[^\\S]    Manager version: 2\\.[0-9]+\\.[0-9]+[^\\S]    Manager build time:.*[^\\S]");
}



static TestSink GetTestSinkFromTestLogger()
{
    Sink sink[1];
    Log4spRegistry.Instance().Get(LOGGER_NAME).GetSinks(sink, sizeof(sink));
    return view_as<TestSink>(sink[0]);
}

static void RegisterLogger()
{
    Logger logger = new Logger(LOGGER_NAME);
    Log4spRegistry.Instance().RegisterLogger(logger);

    TestSink sink = new TestSink();
    logger.AddSink(sink);

    SinkCleanupAndDelete(sink);
    LoggerCleanupAndDelete(logger);
}

static void DropLogger()
{
    Log4spRegistry.Instance().Drop(LOGGER_NAME);
}
