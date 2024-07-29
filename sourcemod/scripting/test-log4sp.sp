#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Test Log4sp"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.0.0"
#define PLUGIN_DESCRIPTION                  "Test Logging for SourcePawn extension"
#define PLUGIN_URL                          "https://github.com/F1F88/Log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};

#include <log4sp>

public void OnPluginStart()
{
    PrintToServer("****************** Test Log4sp Load ******************");

    RegConsoleCmd("sm_tlog4sp", CB_CMD);
}

Action CB_CMD(int client, int args)
{
    // Logger logger = Logger.CreateServerConsoleLogger("F1F88");
    // TestLoggerLvl(logger);
    // delete logger;
    // logger = Logger.CreateServerConsoleLogger("F1F88");
    // TestLoggerLog(logger);
    // delete logger;
    // logger = Logger.CreateServerConsoleLogger("F1F88");
    // TestLoggerPattern(logger);
    // delete logger;
    Logger logger = Logger.CreateServerConsoleLogger("F1F88");
    TestLoggerFlush(logger);
    delete logger;
    // logger = Logger.CreateServerConsoleLogger("F1F88");
    // TestLoggerBacktrace(logger);
    // delete logger;

    // Sink sink = new ServerConsoleSinkST();
    // TestSinkLvl(sink);
    // delete sink;
    // sink = new ServerConsoleSinkST();
    // TestSinkLog(sink);
    // delete sink;
    // sink = new ServerConsoleSinkST();
    // TestSinkPattern(sink);
    // delete sink;

    // BenchmarkServerConsole1();
    // BenchmarkBaseFile1();
    // BenchmarkBaseFile2();

    // ClientConsoleSinkST sink = new ClientConsoleSinkST();
    // TestSinkLvl(sink);
    // TestSinkLog(sink);
    // sink.SetFilter(filter);
    // delete sink;

    // TestServerConsoleSTLogging();
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

    logger.LogLoc("testFile5.log", 5, "testFunc5", LogLevel_Warn, "Test log message 5.");
    logger.LogLocAmxTpl("testFile6.log", 6, "testFunc6", LogLevel_Fatal, "Test log message %d.", 6);

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

    Sink sink1 = new ServerConsoleSinkMT();
    sink1.SetPattern("This is Sink1 %v");
    logger.AddSink(sink1);

    Sink sink2 = new ServerConsoleSinkMT();
    sink1.SetPattern("This is Sink2 %v");
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
    sink.LogAmxTpl("name2", LogLevel_Trace, "Test log message %d.", 2);

    sink.LogSrc("name3", LogLevel_Trace, "Test log message 3.");
    sink.LogSrcAmxTpl("name4", LogLevel_Trace, "Test log message %d.", 4);

    sink.LogLoc("testFile5.log", 5, "testFunc5", "name2", LogLevel_Trace, "Test log message 5.");
    sink.LogLocAmxTpl("testFile6.log", 6, "testFunc6", "name2", LogLevel_Trace, "Test log message %d.", 6);

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

void BenchmarkServerConsole1()
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;

    Logger logger = Logger.CreateServerConsoleLogger("F1F88", true);
    logger.FlushOn(LogLevel_Error);
    startTime1 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        logger.InfoAmxTpl("This is a benchmark log message.");
    }
    endTime1 = GetEngineTime();
    delete logger;

    startTime2 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        PrintToServer("This is a benchmark log message.");
    }
    endTime2 = GetEngineTime();

    PrintToServer("[Log4sp] %d runs take time: %f - %f = %f", count, endTime1, startTime1, endTime1 - startTime1);
    PrintToServer("[PrintToServer] %d runs take time: %f - %f = %f", count, endTime2, startTime2, endTime2 - startTime2);

    /**
     * CPU: AMD Ryzen 7 6800H with Radeon Graphics 3.20 GHz
     * OS: Windows 11 + VMware + Linux ubantu 6.8.0-38-generic #38-Ubuntu SMP PREEMPT_DYNAMIC Fri Jun  7 15:25:01 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
     *
     * Single thread test 1
     * [Log4sp] 1000000 runs take time: 226.947418 - 221.519805 = 5.427612
     * [PrintToServer] 1000000 runs take time: 232.537643 - 226.947418 = 5.590225
     *
     * Single thread test 2
     * [Log4sp] 1000000 runs take time: 261.717285 - 256.228942 = 5.488342
     * [PrintToServer] 1000000 runs take time: 267.243652 - 261.717285 = 5.526367
     *
     * Single thread test 3
     * [Log4sp] 1000000 runs take time: 261.717285 - 256.228942 = 5.488342
     * [PrintToServer] 1000000 runs take time: 267.243652 - 261.717285 = 5.526367
     *
     * Multi thread test 1
     * [Log4sp] 1000000 runs take time: 368.297210 - 362.678802 = 5.618408
     * [PrintToServer] 1000000 runs take time: 373.852478 - 368.297210 = 5.555267
     *
     * Multi thread test 2
     * [Log4sp] 1000000 runs take time: 399.987976 - 394.048248 = 5.939727
     * [PrintToServer] 1000000 runs take time: 405.613250 - 399.987976 = 5.625274
     *
     * Multi thread test 3
     * [Log4sp] 1000000 runs take time: 431.161834 - 425.464477 = 5.697357
     * [PrintToServer] 1000000 runs take time: 436.848602 - 431.161834 = 5.686767
     */
}

void BenchmarkServerConsole2()
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;

    Logger logger = Logger.CreateServerConsoleLogger("F1F88", true);
    startTime1 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        logger.InfoAmxTpl("This is a benchmark log message.( %d )", i);
    }
    endTime1 = GetEngineTime();
    delete logger;

    startTime2 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        PrintToServer("This is a benchmark log message %d.", i);
    }
    endTime2 = GetEngineTime();

    PrintToServer("[Log4sp] %d runs take time: %f - %f = %f", count, endTime1, startTime1, endTime1 - startTime1);
    PrintToServer("[PrintToServer] %d runs take time: %f - %f = %f", count, endTime2, startTime2, endTime2 - startTime2);

    /**
     * CPU: AMD Ryzen 7 6800H with Radeon Graphics 3.20 GHz
     * OS: Windows 11 + VMware + Linux ubantu 6.8.0-38-generic #38-Ubuntu SMP PREEMPT_DYNAMIC Fri Jun  7 15:25:01 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
     *
     * Single thread test 1
     * [Log4sp] 1000000 runs take time: 72.754776 - 67.308731 = 5.446044
     * [PrintToServer] 1000000 runs take time: 78.265388 - 72.754791 = 5.510597
     *
     * Single thread test 2
     * [Log4sp] 1000000 runs take time: 105.481781 - 100.066749 = 5.415031
     * [PrintToServer] 1000000 runs take time: 111.136497 - 105.481796 = 5.654701
     *
     * Single thread test 3
     * [Log4sp] 1000000 runs take time: 143.637191 - 138.247833 = 5.389358
     * [PrintToServer] 1000000 runs take time: 149.229171 - 143.637207 = 5.591964
     */
}

void BenchmarkBaseFile1()
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;
    bool mt = true;

    Logger logger = Logger.CreateBaseFileLogger("F1F88", "logs/benchmark_BaseFile.log", .mt=mt);
    Sink sink = mt ? new ServerConsoleSinkMT() : new ServerConsoleSinkST();
    logger.AddSink(sink);
    startTime1 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        logger.InfoAmxTpl("This is a benchmark log message.( %d )", i);
    }
    endTime1 = GetEngineTime();
    delete logger;

    startTime2 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        LogMessage("This is a benchmark log message %d.", i);
    }
    endTime2 = GetEngineTime();

    PrintToServer("[Log4sp] %d runs take time: %f - %f = %f", count, endTime1, startTime1, endTime1 - startTime1);
    PrintToServer("[LogMessage] %d runs take time: %f - %f = %f", count, endTime2, startTime2, endTime2 - startTime2);

    /**
     * CPU: AMD Ryzen 7 6800H with Radeon Graphics 3.20 GHz
     * OS: Windows 11 + VMware + Linux ubantu 6.8.0-38-generic #38-Ubuntu SMP PREEMPT_DYNAMIC Fri Jun  7 15:25:01 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
     *
     * Single thread test 1
     * [Log4sp] 1000000 runs take time: 27.164484 - 20.316698 = 6.847785
     * [LogMessage] 1000000 runs take time: 57.080307 - 27.164508 = 29.915798
     *
     * Single thread test 2
     * [Log4sp] 1000000 runs take time: 86.557907 - 80.351654 = 6.206253
     * [LogMessage] 1000000 runs take time: 117.214317 - 86.557945 = 30.656372
     *
     * Single thread test 3
     * [Log4sp] 1000000 runs take time: 138.137557 - 131.995086 = 6.142471
     * [LogMessage] 1000000 runs take time: 168.817291 - 138.137588 = 30.679702
     *
     * Multi thread test 1
     * [Log4sp] 1000000 runs take time: 19.409643 - 13.768821 = 5.640821
     * [LogMessage] 1000000 runs take time: 49.667854 - 19.409666 = 30.258188
     *
     * Multi thread test 2
     * [Log4sp] 1000000 runs take time: 74.528450 - 68.739273 = 5.789176
     * [LogMessage] 1000000 runs take time: 104.856529 - 74.528678 = 30.327850
     *
     * Multi thread test 3
     * [Log4sp] 1000000 runs take time: 136.293975 - 130.272277 = 6.021697
     * [LogMessage] 1000000 runs take time: 169.586532 - 136.294006 = 33.292526
     */
}

void BenchmarkBaseFile2()
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;
    bool mt = true;

    Logger logger = Logger.CreateBaseFileLogger("F1F88", "logs/benchmark_BaseFile.log", .mt=mt);
    startTime1 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        logger.InfoAmxTpl("This is a benchmark log message.( %d )", i);
    }
    endTime1 = GetEngineTime();
    delete logger;

    startTime2 = GetEngineTime();
    for (int i=0; i<count; ++i)
    {
        LogToFileEx("addons/sourcemod/logs/benchmark_LogToFileEx.log", "This is a benchmark log message %d.", i);
    }
    endTime2 = GetEngineTime();

    PrintToServer("[Log4sp] %d runs take time: %f - %f = %f", count, endTime1, startTime1, endTime1 - startTime1);
    PrintToServer("[LogToFileEx] %d runs take time: %f - %f = %f", count, endTime2, startTime2, endTime2 - startTime2);

    /**
     * CPU: AMD Ryzen 7 6800H with Radeon Graphics 3.20 GHz
     * OS: Windows 11 + VMware + Linux ubantu 6.8.0-38-generic #38-Ubuntu SMP PREEMPT_DYNAMIC Fri Jun  7 15:25:01 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
     *
     * Single thread test 1
     * [Log4sp] 1000000 runs take time: 25.216350 - 24.491693 = 0.724657
     * [LogToFileEx] 1000000 runs take time: 51.250938 - 25.216367 = 26.034570
     *
     * Single thread test 2
     * [Log4sp] 1000000 runs take time: 88.619285 - 88.291938 = 0.327346
     * [LogToFileEx] 1000000 runs take time: 114.989753 - 88.619300 = 26.370452
     *
     * Single thread test 3
     * [Log4sp] 1000000 runs take time: 125.556961 - 125.226394 = 0.330566
     * [LogToFileEx] 1000000 runs take time: 151.940872 - 125.556983 = 26.383888
     *
     * Multi thread test 1
     * [Log4sp] 1000000 runs take time: 12.956454 - 12.609822 = 0.346632
     * [LogToFileEx] 1000000 runs take time: 41.840663 - 12.956472 = 28.884191
     *
     * Multi thread test 2
     * [Log4sp] 1000000 runs take time: 77.214035 - 76.871261 = 0.342773
     * [LogToFileEx] 1000000 runs take time: 104.955268 - 77.214057 = 27.741210
     *
     * Multi thread test 3
     * [Log4sp] 1000000 runs take time: 120.821243 - 120.471275 = 0.349967
     * [LogToFileEx] 1000000 runs take time: 148.050079 - 120.821258 = 27.228820
     */
}

