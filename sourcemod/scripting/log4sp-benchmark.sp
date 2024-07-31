#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Log4sp Benchmark"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.0.0"
#define PLUGIN_DESCRIPTION                  "Logging for SourcePawn Benchmark"
#define PLUGIN_URL                          "https://github.com/F1F88/sm-ext-log4sp"

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
    PrintToServer("****************** Log4sp Benchmark ******************");

    RegConsoleCmd("sm_log4sp_benchmark", CB_CMD);
}

Action CB_CMD(int client, int args)
{
    int type = 0;
    bool mt = false;
    if (args >= 2)
    {
        type = GetCmdArgInt(1);
        mt = view_as<bool>(GetCmdArgInt(1));
    }

    switch (type)
    {
        case 2:  BenchmarkServerConsole2(mt);
        case 3:  BenchmarkBaseFile3(mt);
        case 4:  BenchmarkBaseFile4(mt);
        default: BenchmarkServerConsole1(mt);
    }

    return Plugin_Handled;
}

stock void BenchmarkServerConsole1(bool mt = false)
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;

    Logger logger = Logger.CreateServerConsoleLogger("logger-benchmark-1", mt);
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

stock void BenchmarkServerConsole2(bool mt = false)
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;

    Logger logger = Logger.CreateServerConsoleLogger("logger-benchmark-2", mt);
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

stock void BenchmarkBaseFile3(bool mt = false)
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;

    Logger logger = Logger.CreateBaseFileLogger("logger-benchmark-3", "logs/benchmark_BaseFile_3.log", .mt=mt);
    Sink sink = mt ? view_as<Sink>(new ServerConsoleSinkMT()) : view_as<Sink>(new ServerConsoleSinkST());
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

stock void BenchmarkBaseFile4(bool mt = false)
{
    float startTime1, endTime1;
    float startTime2, endTime2;
    const int count = 1000000;

    Logger logger = Logger.CreateBaseFileLogger("logger-benchmark-4", "logs/benchmark_BaseFile_4.log", .mt=mt);
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

