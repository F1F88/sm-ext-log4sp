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

#include <sourcemod>
#include <log4sp>

#undef  REQUIRE_PLUGIN
#include <log4sp/registry>
#define REQUIRE_PLUGIN

#include "../assert"
#include "../test_sink"
#include "../test_utils"

#define LOGGER_NAME_1   "registry_logger_1"
#define LOGGER_NAME_2   "registry_logger_2"


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_registry", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------------ Started testing Registry ------------");

    SetTestContext("Registry");

    AssertTrue("[library exists]", LoggerRegistry.LibraryExists());

    Test_GlobalLogger();

    Test_RegisterLogger();

    Test_RegisterOrReplaceLogger();

    Test_InitializeLogger();

    Test_DropAll();

    Test_ApplyAll();

    Test_FlushAll();

    Test_ApplyLoggerEnvLevels();

    // 必须是最后一个测试
    Test_FlushEvery();

    PrintToServer("--------------- Test Registry ended --------------");
}


void Test_GlobalLogger()
{
    // To default
    if (LoggerRegistry.Instance().Get())
        LoggerRegistry.Instance().Drop();

    if (!LoggerRegistry.Instance().Get(""))
    {
        Logger logger = new Logger("");
        LoggerRegistry.Instance().SetGlobalLogger(logger);
        LoggerCloseAndDelete(logger);
    }

    // Get
    Logger logger1 = LoggerRegistry.Instance().Get(NULL_STRING);
    Logger logger2 = LoggerRegistry.Instance().Get("");
    Logger logger3 = LoggerRegistry.Instance().GetGlobal();
    Logger logger4 = LoggerRegistry.Instance().GetGlobalRaw();

    AssertTrue("[global global eq]", logger2.Equals(logger3));
    AssertTrue("[global global eq]", logger2.Equals(logger4));

    // Drop
    LoggerRegistry.Instance().Drop("");

    Logger logger5 = LoggerRegistry.Instance().Get("");
    Logger logger6 = LoggerRegistry.Instance().GetGlobal();
    Logger logger7 = LoggerRegistry.Instance().GetGlobalRaw();

    // To default
    {
        Logger logger = new Logger("");
        LoggerRegistry.Instance().SetGlobalLogger(logger);
        LoggerCloseAndDelete(logger);
    }

    // DropAll
    Logger logger8 = LoggerRegistry.Instance().Get("");
    Logger logger9 = LoggerRegistry.Instance().GetGlobal();
    Logger logger10 = LoggerRegistry.Instance().GetGlobalRaw();

    AssertTrue("[global global eq]", logger8.Equals(logger8));
    AssertTrue("[global global eq]", logger8.Equals(logger10));

    LoggerRegistry.Instance().DropAll();

    Logger logger11 = LoggerRegistry.Instance().Get("");
    Logger logger12 = LoggerRegistry.Instance().GetGlobal();
    Logger logger13 = LoggerRegistry.Instance().GetGlobalRaw();

    AssertEq("[global null]",           logger1, INVALID_HANDLE);
    AssertNe("[global before drop]",    logger2, INVALID_HANDLE);
    AssertNe("[global before drop]",    logger3, INVALID_HANDLE);
    AssertNe("[global before drop]",    logger4, INVALID_HANDLE);
    AssertEq("[global after  drop]",    logger5, INVALID_HANDLE);
    AssertEq("[global after  drop]",    logger6, INVALID_HANDLE);
    AssertEq("[global after  drop]",    logger7, INVALID_HANDLE);
    AssertNe("[global before drop all]",logger8, INVALID_HANDLE);
    AssertNe("[global before drop all]",logger9, INVALID_HANDLE);
    AssertNe("[global before drop all]",logger10, INVALID_HANDLE);
    AssertEq("[global after  drop all]",logger11, INVALID_HANDLE);
    AssertEq("[global after  drop all]",logger12, INVALID_HANDLE);
    AssertEq("[global after  drop all]",logger13, INVALID_HANDLE);
}

void Test_RegisterLogger()
{
    if (LoggerRegistry.Instance().Get())
        LoggerRegistry.Instance().Drop();

    if (LoggerRegistry.Instance().Get(LOGGER_NAME_1))
        LoggerRegistry.Instance().Drop();

    if (LoggerRegistry.Instance().Get(LOGGER_NAME_2))
        LoggerRegistry.Instance().Drop();

    Logger logger = new Logger();
    Logger logger1 = new Logger(LOGGER_NAME_1);
    Logger logger2 = new Logger(LOGGER_NAME_2);

    LoggerRegistry.Instance().RegisterLogger(logger);
    LoggerRegistry.Instance().RegisterLogger(logger1);
    LoggerRegistry.Instance().RegisterLogger(logger2);

    LoggerCloseAndDelete(logger);
    LoggerCloseAndDelete(logger1);
    LoggerCloseAndDelete(logger2);

    AssertNe("[before drop]", LoggerRegistry.Instance().Get(), INVALID_HANDLE);
    AssertNe("[before drop]", LoggerRegistry.Instance().Get(LOGGER_NAME_1), INVALID_HANDLE);
    AssertNe("[before drop]", LoggerRegistry.Instance().Get(LOGGER_NAME_2), INVALID_HANDLE);

    LoggerRegistry.Instance().Drop();
    LoggerRegistry.Instance().Drop(LOGGER_NAME_1);
    LoggerRegistry.Instance().Drop(LOGGER_NAME_2);

    AssertEq("[after drop]", LoggerRegistry.Instance().Get(), INVALID_HANDLE);
    AssertEq("[after drop]", LoggerRegistry.Instance().Get(LOGGER_NAME_1), INVALID_HANDLE);
    AssertEq("[after drop]", LoggerRegistry.Instance().Get(LOGGER_NAME_2), INVALID_HANDLE);
}

void Test_RegisterOrReplaceLogger()
{
    if (LoggerRegistry.Instance().Get())
        LoggerRegistry.Instance().Drop();

    Logger logger1 = new Logger();
    Logger logger2 = new Logger();

    logger1.SetLevel(LogLevel_Trace);
    logger2.SetLevel(LogLevel_Debug);

    LoggerRegistry.Instance().RegisterLogger(logger1);
    LogLevel lvl1 = LoggerRegistry.Instance().Get().GetLevel();

    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger2);
    LogLevel lvl2 = LoggerRegistry.Instance().Get().GetLevel();

    LoggerRegistry.Instance().Drop();
    LoggerCloseAndDelete(logger1);
    LoggerCloseAndDelete(logger2);

    AssertEq("[before register or replace]", lvl1, LogLevel_Trace);
    AssertEq("[after register or replace]",  lvl2, LogLevel_Debug);
}

int __iCustomErrCnt = 0;
void Test_InitializeLogger()
{
    // To default
    char[] name = "init";
    if (LoggerRegistry.Instance().Get(name))
        LoggerRegistry.Instance().Drop(name);

    StringMap levels = new StringMap();
    LoggerRegistry.Instance().SetLevels(levels, LogLevel_Info);

    Logger logger = new Logger(name);

    // SetAutomaticRegistration
    LoggerRegistry.Instance().SetAutomaticRegistration(true);   // default true
    LoggerRegistry.Instance().InitializeLogger(logger);
    bool autoRegEnable = LoggerRegistry.Instance().Get(name) != INVALID_HANDLE;

    LoggerRegistry.Instance().Drop(name);
    LoggerRegistry.Instance().SetAutomaticRegistration(false);  // default true
    LoggerRegistry.Instance().InitializeLogger(logger);
    bool autoRegDisable = LoggerRegistry.Instance().Get(name) == INVALID_HANDLE;

    // SetLevel
    LoggerRegistry.Instance().SetLevel(LogLevel_Trace);         // default info
    LoggerRegistry.Instance().InitializeLogger(logger);
    LogLevel logLvlBySetLevel = logger.GetLevel();

    // SetLevels
    levels.SetValue(name, LogLevel_Warn);
    LoggerRegistry.Instance().SetLevels(levels, LogLevel_Error);// default empty (null) & info
    LoggerRegistry.Instance().InitializeLogger(logger);
    LogLevel logLvlBySetLevels = logger.GetLevel();
    delete levels;

    // SetFlushLevel
    LoggerRegistry.Instance().SetFlushLevel(LogLevel_Trace);    // default off
    LoggerRegistry.Instance().InitializeLogger(logger);
    LogLevel flushLvlBySetLevel = logger.GetFlushLevel();

    // SetErrorHandler
    __iCustomErrCnt = 0;
    TestSink sink = new TestSink();
    logger.SetLevel(LogLevel_Trace);
    logger.AddSink(sink);

    sink.SetLogError("Error!");
    SinkCloseAndDelete(sink);

    LoggerRegistry.Instance().SetErrorHandler(null, CB_ErrorHandler); // default INVALID_FUNCTION
    LoggerRegistry.Instance().InitializeLogger(logger);
    logger.Error("Error messages.");
    logger.Fatal("Fatal messages.");

    LoggerCloseAndDelete(logger);

    // Restore to default
    StringMap map = new StringMap();
    LoggerRegistry.Instance().SetAutomaticRegistration(true);
    LoggerRegistry.Instance().SetLevels(map, LogLevel_Info);
    LoggerRegistry.Instance().SetFlushLevel(LogLevel_Off);
    delete map;

    AssertTrue("[init auto reg disable]", autoRegDisable);
    AssertTrue("[init auto reg enable]", autoRegEnable);
    AssertEq("[init log level]", logLvlBySetLevel, LogLevel_Trace);
    AssertEq("[init log levels]", logLvlBySetLevels, LogLevel_Warn);
    AssertEq("[init flush level]", flushLvlBySetLevel, LogLevel_Trace);
    AssertEq("[init err counter]", __iCustomErrCnt, 2);
}

void Test_DropAll()
{
    Logger logger = new Logger();
    Logger logger1 = new Logger(LOGGER_NAME_1);
    Logger logger2 = new Logger(LOGGER_NAME_2);

    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger);
    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger1);
    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger2);

    LoggerCloseAndDelete(logger);
    LoggerCloseAndDelete(logger1);
    LoggerCloseAndDelete(logger2);

    AssertNe("[before drop all]", LoggerRegistry.Instance().Get(), INVALID_HANDLE);
    AssertNe("[before drop all]", LoggerRegistry.Instance().Get(LOGGER_NAME_1), INVALID_HANDLE);
    AssertNe("[before drop all]", LoggerRegistry.Instance().Get(LOGGER_NAME_2), INVALID_HANDLE);

    LoggerRegistry.Instance().DropAll();

    AssertEq("[after drop all]", LoggerRegistry.Instance().Get(), INVALID_HANDLE);
    AssertEq("[after drop all]", LoggerRegistry.Instance().Get(LOGGER_NAME_1), INVALID_HANDLE);
    AssertEq("[after drop all]", LoggerRegistry.Instance().Get(LOGGER_NAME_2), INVALID_HANDLE);
}

void Test_ApplyAll()
{
    Logger logger = new Logger();
    Logger logger1 = new Logger(LOGGER_NAME_1);
    Logger logger2 = new Logger(LOGGER_NAME_2);

    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger);
    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger1);
    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger2);

    LogLevel beforeLvl = logger.GetLevel();
    LogLevel beforeLvl1 = logger1.GetLevel();
    LogLevel beforeLvl2 = logger2.GetLevel();

    LoggerRegistry.Instance().ApplyAll(null, CB_ApplyAll);

    LogLevel afterLvl = logger.GetLevel();
    LogLevel afterLvl1 = logger1.GetLevel();
    LogLevel afterLvl2 = logger2.GetLevel();

    LoggerCloseAndDelete(logger);
    LoggerCloseAndDelete(logger1);
    LoggerCloseAndDelete(logger2);

    LoggerRegistry.Instance().Drop();
    LoggerRegistry.Instance().Drop(LOGGER_NAME_1);
    LoggerRegistry.Instance().Drop(LOGGER_NAME_2);

    AssertEq("[before apply all]", beforeLvl,  LogLevel_Info);
    AssertEq("[before apply all]", beforeLvl1, LogLevel_Info);
    AssertEq("[before apply all]", beforeLvl2, LogLevel_Info);
    AssertEq("[after apply all]",  afterLvl,  LogLevel_Debug);
    AssertEq("[after apply all]",  afterLvl1, LogLevel_Debug);
    AssertEq("[after apply all]",  afterLvl2, LogLevel_Debug);
}

void Test_FlushAll()
{
    TestSink sink = new TestSink();
    Logger logger = new Logger();
    Logger logger1 = new Logger(LOGGER_NAME_1);
    Logger logger2 = new Logger(LOGGER_NAME_2);

    logger.AddSink(sink);
    logger1.AddSink(sink);
    logger2.AddSink(sink);

    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger);
    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger1);
    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger2);

    LoggerRegistry.Instance().FlushAll();
    int flushCount = sink.GetFlushCount();

    SinkCloseAndDelete(sink);
    LoggerCloseAndDelete(logger);
    LoggerCloseAndDelete(logger1);
    LoggerCloseAndDelete(logger2);

    LoggerRegistry.Instance().Drop();
    LoggerRegistry.Instance().Drop(LOGGER_NAME_1);
    LoggerRegistry.Instance().Drop(LOGGER_NAME_2);

    AssertEq("[flush all]", flushCount, 3);
}

void Test_ApplyLoggerEnvLevels()
{
    // To default
    StringMap levels = new StringMap();
    LoggerRegistry.Instance().SetLevels(levels, LogLevel_Info);

    char[] name = "apply-env-levels";
    Logger logger = new Logger(name);
    logger.SetLevel(LogLevel_Trace);

    LogLevel lvl1 = logger.GetLevel();

    LoggerRegistry.Instance().ApplyLoggerEnvLevels(logger);
    LogLevel lvl2 = logger.GetLevel();

    LoggerRegistry.Instance().SetLevels(levels, LogLevel_Warn);
    LoggerRegistry.Instance().ApplyLoggerEnvLevels(logger);
    LogLevel lvl3 = logger.GetLevel();

    levels.SetValue(name, LogLevel_Error);
    LoggerRegistry.Instance().SetLevels(levels, LogLevel_Fatal);
    LoggerRegistry.Instance().ApplyLoggerEnvLevels(logger);
    LogLevel lvl4 = logger.GetLevel();

    LoggerCloseAndDelete(logger);

    AssertEq("[apply env levels]", lvl1, LogLevel_Trace);
    AssertEq("[apply env levels]", lvl2, LogLevel_Info);
    AssertEq("[apply env levels]", lvl3, LogLevel_Warn);
    AssertEq("[apply env levels]", lvl4, LogLevel_Error);
}

void Test_FlushEvery()
{
    TestSink sink = new TestSink();
    Logger logger = new Logger();

    logger.AddSink(sink);

    LoggerRegistry.Instance().SetFlushEvery(0.0);
    Handle beforeTimer = LoggerRegistry.Instance().GetFlusher();

    LoggerRegistry.Instance().RegisterOrReplaceLogger(logger);

    LoggerRegistry.Instance().SetFlushEvery(1.0);
    Handle afterTimer = LoggerRegistry.Instance().GetFlusher();

    SinkCloseAndDelete(sink);
    LoggerCloseAndDelete(logger);

    // 必须是最后一个测试
    CreateTimer(3.5, Timer_AssertFlushEvery);

    AssertEq("[flush before timer]", beforeTimer, INVALID_HANDLE);
    AssertNe("[flush after timer]", afterTimer, INVALID_HANDLE);
}

public void CB_ErrorHandler(const char[] origin, LOG4SP_CONST_ES SourceLoc loc, const char[] msg)
{
    __iCustomErrCnt++;

    AssertStrEndsWith("[custom err handler origin]", origin, "init");
    AssertStrEndsWith("[custom err handler file  ]", loc.filename, "test-registry.sp");
    AssertStrEq("[custom err handler func  ]", loc.funcname, "Test_InitializeLogger");
    AssertStrEq("[custom err handler msg   ]", msg, "Error!");
}

public void CB_ApplyAll(Logger logger)
{
    logger.SetLevel(LogLevel_Debug);
}

public Action Timer_AssertFlushEvery(Handle timer)
{
    Logger logger = LoggerRegistry.Instance().Get();

    TestSink sink = view_as<TestSink>(logger.GetSink(0));
    int flushCount = sink.GetFlushCount();
    SinkCloseAndDelete(sink);

    // Restore
    LoggerRegistry.Instance().Drop();
    LoggerRegistry.Instance().SetFlushEvery(0.0);

    AssertEq("[flush every]", flushCount, 3);
    return Plugin_Stop;
}
