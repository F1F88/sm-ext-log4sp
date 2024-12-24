#include <sourcemod>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Log4sp Test"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Logging for SourcePawn test"
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
    PrintToServer("****************** Log4sp Test ******************");

    RegConsoleCmd("sm_log4sp_test", CB_CMD);

    RegConsoleCmd("sm_log4sp_throw_error1", CB_CMD_ThrowError1);
    RegConsoleCmd("sm_log4sp_throw_error2", CB_CMD_ThrowError2);
}

Action CB_CMD(int client, int args)
{
    Logger logger1 = Logger.CreateServerConsoleLogger("logger-test-1");
    TestLoggerLvl(logger1);
    delete logger1;

    Logger logger2 = Logger.CreateServerConsoleLogger("logger-test-2", true);
    TestLoggerLog(logger2);
    delete logger2;

    Logger logger3 = Logger.CreateServerConsoleLogger("logger-test-3");
    TestLoggerPattern(logger3);
    delete logger3;

    Logger logger4 = Logger.CreateServerConsoleLogger("logger-test-4", true);
    TestLoggerFlush(logger4);
    delete logger4;

    Logger logger5 = Logger.CreateServerConsoleLogger("logger-test-5");
    TestLoggerBacktrace(logger5);
    delete logger5;

    Logger logger6 = Logger.CreateServerConsoleLogger("logger-test-6", true);
    TestLoggerSink(logger6);
    delete logger6;


    Sink sink1 = new ServerConsoleSink();
    TestSinkLvl(sink1);
    delete sink1;

    Sink sink2 = new ServerConsoleSink(true);
    TestSinkLog(sink2);
    delete sink2;

    Sink sink3 = new ServerConsoleSink();
    TestSinkPattern(sink3);
    delete sink3;

    ClientConsoleSink sink4 = new ClientConsoleSink(true);
    TestSinkLvl(sink4);
    TestSinkLog(sink4);
    sink4.SetFilter(filter);
    TestSinkLog(sink4);
    delete sink4;

    TestNullSink();

    StaticFactory();

    return Plugin_Handled;
}

Action filter(int client, const char[] name, LogLevel lvl, const char[] msg)
{
    // PrintToServer("[SP] Client Msg: client=%d | name=%s | lvl=%d | msg=%s |", client, name, lvl, msg);
    return Plugin_Continue;
}

void TestLoggerLvl(Logger logger)
{
    char name[64];
    logger.GetName(name, sizeof(name));
    PrintToServer("========== Test Logger Level Start | name=%s |==========", name);

    LogLevel oldLvl, newLvl;
    bool shouldLogTrace, shouldLogInfo;

    oldLvl = logger.GetLevel(); // info 2
    logger.Debug("This message should be ignored. 1");

    logger.SetLevel(LogLevel_Debug);
    newLvl = logger.GetLevel(); // debug 1
    logger.Debug("This message should be log. 2");

    shouldLogTrace = logger.ShouldLog(LogLevel_Trace); // false
    shouldLogInfo = logger.ShouldLog(LogLevel_Info); // true

    PrintToServer(" | oldLvl=%d | newLvl=%d | shouldLogTrace=%s | shouldLogInfo=%s",
        oldLvl, newLvl, shouldLogTrace ? "true" : "false", shouldLogInfo ? "true" : "false");

    PrintToServer("========== Test Logger Level End | name=%s | ==========", name);
}

void TestLoggerLog(Logger logger)
{
    char name[64];
    logger.GetName(name, sizeof(name));
    PrintToServer("========== Test Logger Log Start | name=%s |==========", name);

    logger.Log(LogLevel_Off, "Test log message 1.");
    logger.LogAmxTpl(LogLevel_Trace, "Test log message %d.", 2);

    logger.LogSrc(LogLevel_Debug, "Test log message 3.");
    logger.LogSrcAmxTpl(LogLevel_Info, "Test log message %d.", 4);

    logger.LogLoc("/home/sm-ext-log4sp/Linux-testFile5.log", 5, "testFunc5", LogLevel_Warn, "Test log message 5.");
    logger.LogLocAmxTpl("C:\\user\\sm-ext-log4sp\\Win-testFile6.log", 6, "testFunc6", LogLevel_Fatal, "Test log message %d.", 6);

    logger.Trace("Test log message 7.");
    logger.TraceAmxTpl("Test log message %d.", 8);

    logger.Trace("Test log message 9.");
    logger.TraceAmxTpl("Test log message %d.", 10);

    logger.Info("Test log message 11.");
    logger.InfoAmxTpl("Test log message %d.", 12);

    logger.Warn("Test log message 13.");
    logger.WarnAmxTpl("Test log message %d.", 14);

    logger.Error("Test log message 15.");
    logger.ErrorAmxTpl("Test log message %d.", 16);

    logger.Fatal("Test log message 17.");
    logger.FatalAmxTpl("Test log message %d.", 18);

    logger.LogStackTrace(LogLevel_Fatal, "Test log message 19.");
    logger.LogStackTraceAmxTpl(LogLevel_Warn, "Test log message %d.", 20);

    PrintToServer("========== Test Logger Log End | name=%s | ==========", name);
}

void TestLoggerPattern(Logger logger)
{
    char name[64];
    logger.GetName(name, sizeof(name));
    PrintToServer("========== Test Logger Pattern Start | name=%s |==========", name);

    logger.Info("Test SetPattern before. 1");

    logger.SetPattern("New Pattern: %v");
    logger.Info("Test SetPattern after. 2");

    PrintToServer("========== Test Logger Pattern End | name=%s | ==========", name);
}

void TestLoggerFlush(Logger logger)
{
    char name[64];
    logger.GetName(name, sizeof(name));
    PrintToServer("========== Test Logger Flush Start | name=%s |==========", name);

    logger.SetLevel(LogLevel_Trace);

    LogLevel oldLvl, newLvl;

    logger.Info("This message will flush immediately. 1");

    oldLvl = logger.GetFlushLevel(); // off 6
    logger.FlushOn(LogLevel_Info);
    newLvl = logger.GetFlushLevel(); // info 2

    logger.Info("This message will flush immediately 2.");
    logger.Debug("This message will not flush immediately 3.");

    PrintToServer(" | oldLvl=%d | newLvl=%d |", oldLvl, newLvl);

    PrintToServer("========== Test Logger Flush End | name=%s | ==========", name);
}

void TestLoggerBacktrace(Logger logger)
{
    char name[64];
    logger.GetName(name, sizeof(name));
    PrintToServer("========== Test Logger Backtrace Start | name=%s |==========", name);

    bool oldShouldBT, newShouldBT, endShouldBT;

    oldShouldBT = logger.ShouldBacktrace();
    logger.Trace("This message should be ignored. 1");
    logger.Debug("This message should be ignored. 2");

    logger.EnableBacktrace(3);
    newShouldBT = logger.ShouldBacktrace();
    logger.Trace("This message will not log immediately. 3");
    logger.Trace("This message will not log immediately. 4");
    logger.Debug("This message will not log immediately. 5");
    logger.Debug("This message will not log immediately. 6");
    logger.Info("This message will log immediately. 7");
    logger.DumpBacktrace();     // This will output
    // [trace] This message will not log immediately. 4
    // [debug] This message will not log immediately. 5
    // [debug] This message will not log immediately. 6

    logger.DisableBacktrace();
    endShouldBT = logger.ShouldBacktrace();
    logger.Trace("This message should be ignored. 8");
    logger.Debug("This message should be ignored. 9");
    logger.DumpBacktrace();     // This will not output

    PrintToServer(" | oldShouldBT=%s | newShouldBT=%s | endShouldBT=%s |"
        , oldShouldBT ? "true" : "false", newShouldBT ? "true" : "false", endShouldBT ? "true" : "false");

    PrintToServer("========== Test Logger Backtrace End | name=%s | ==========", name);
}

void TestLoggerSink(Logger logger)
{
    char name[64];
    logger.GetName(name, sizeof(name));
    PrintToServer("========== Test Logger Sink Start | name=%s |==========", name);

    logger.Info("Test before add ServerConsoleSinkMT message. 1");

    Sink sink1 = new ServerConsoleSink(true);
    sink1.SetPattern("This is Sink1 %v");
    logger.AddSink(sink1);

    Sink sink2 = new ServerConsoleSink(true);
    sink2.SetPattern("This is Sink2 %v");
    logger.AddSink(sink2);

    // 这个操作至少会在 ServerConsole 输出 2 条 log message
    logger.Info("Test after add ServerConsoleSinkMT message. 2");

    // 从 logger 中移除 sink1
    logger.DropSink(sink1);
    // 从 extension 中的 sink handle register 移除 sink1
    // 由于 sink1 已经不再被引用, 所以 sink1 会从内存中释放
    delete sink1;

    // 从 extension 中的 sink handle register 移除 sink2
    // 由于 sink2 还被 logger 引用，所以 sink2 不会从内存中释放
    // 直到 logger 被释放后，sink2 的引用归 0，这时才会从内存中释放 sink2
    delete sink2;

    // 本次输出在 ServerConsole 的 log message 会减少 1 条
    // 因为 sink1 从 logger 中移除了
    logger.Info("Test after add ServerConsoleSinkMT message. 3");

    PrintToServer("========== Test Logger Sink End | name=%s | ==========", name);
}

void TestSinkLvl(Sink sink)
{
    PrintToServer("========== Test Sink Level Start ==========");

    LogLevel oldLvl, newLvl;
    bool shouldLogTrace, shouldLogInfo;

    oldLvl = sink.GetLevel(); // trace 0
    sink.Log("name1", LogLevel_Trace, "This message should be log. 1");

    sink.SetLevel(LogLevel_Debug);
    newLvl = sink.GetLevel(); // debug 1
    sink.Log("name2", LogLevel_Trace, "This message should be log. 2");
    // 这里设置的 lvl 不会生效，日志消息都会被输出
    // sink 添加到 logger 里以后， logger.log(...) 时，先检测 logger 自身的 level，然后再检测 sink 的level
    // 即不同的 sink 可以设置不同的 level

    shouldLogTrace = sink.ShouldLog(LogLevel_Trace); // false
    shouldLogInfo = sink.ShouldLog(LogLevel_Info); // true

    PrintToServer(" | oldLvl=%d | newLvl=%d | shouldLogTrace=%s | shouldLogInfo=%s",
        oldLvl, newLvl, shouldLogTrace ? "true" : "false", shouldLogInfo ? "true" : "false");

    PrintToServer("========== Test Sink Level End ==========");
}

void TestSinkLog(Sink sink)
{
    PrintToServer("========== Test Sink Log Start ==========");

    sink.Log("name1", LogLevel_Trace, "Test log message 1.");

    PrintToServer("========== Test Sink Log End ==========");
}

void TestSinkPattern(Sink sink)
{
    PrintToServer("========== Test Sink Pattern Start ==========");

    sink.Log("name1", LogLevel_Info, "Test SetPattern before. 1");

    sink.SetPattern("New Pattern: %v");
    sink.Log("name2", LogLevel_Info, "Test SetPattern before. 2");

    PrintToServer("========== Test Sink Pattern End ==========");
}


Action CB_CMD_ThrowError1(int client, int args)
{
    // 由于会被中断，所以存在内存泄漏
    static Logger log;
    if (log == INVALID_HANDLE)
        log = Logger.CreateServerConsoleLogger("logger-test-thorw-error-1");

    log.ThrowError(LogLevel_Fatal, "--- 测试 ThrowError 效果 1 ---");
    PrintToServer("--- 测试是否继续执行 1 ---");
    delete log;
    return Plugin_Handled;
}

Action CB_CMD_ThrowError2(int client, int args)
{
    // 由于会被中断，所以存在内存泄漏
    static Logger log;
    if (log == INVALID_HANDLE)
        log = Logger.CreateServerConsoleLogger("logger-test-thorw-error-2");

    log.ThrowErrorAmxTpl(LogLevel_Warn, "--- 测试 ThrowError 效果 %d---", 2);
    PrintToServer("--- 测试是否继续执行 2 ---");
    delete log;
    return Plugin_Handled;
}

void TestNullSink()
{
    PrintToServer("========== Test Null Sink ==========");
    Sink sinks[1];
    Logger log = new Logger("test-null-sink", sinks, 0);

    log.Info("lalala");
    delete log;
    PrintToServer("========== Test Null Sink End ==========");
}

void StaticFactory()
{
    Logger log = Logger.CreateClientChatLogger("test-factory-client-chat");
    log.Fatal("This is a ClientChatLogger");
    delete log;

    log = Logger.CreateClientConsoleLogger("test-factory-client-console");
    log.Fatal("This is a ClientConsoleLogger");
    delete log;
}
