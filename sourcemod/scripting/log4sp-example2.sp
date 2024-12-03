#include <sourcemod>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Log4sp Example 2"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Logging for SourcePawn example 2"
#define PLUGIN_URL              "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};


Logger myLogger;


public void OnPluginStart()
{
    // 创建一个 单线程的, 名为 "myLogger" 的 Logger 对象
    myLogger = Logger.CreateServerConsoleLogger("logger-example-2");

    // 配置 myLogger 的输出级别. 默认级别: LogLevel_Info
    myLogger.SetLevel(LogLevel_Debug);

    // 配置日志格式. 默认格式: [2014-10-31 23:46:59.678] [my_loggername] [info] Some message
    myLogger.SetPattern("[Date: %Y/%m/%d %H:%M:%S] [Name: %n] [LogLevel: %l] [message: %v]");

    /**
     * 每个手动创建的 sink 都会注册到拓展内部的 sink register, 即引用数至少为 1
     */
    // 创建一个 Sink 对象, 用于输出到玩家控制台
    ClientConsoleSink mySink = new ClientConsoleSink();

    // 修改 mySink 的级别. 默认为 LogLevel_Trace
    mySink.SetLevel(LogLevel_Info);

    // 修改 mySink 过滤器. 默认输出给所有在游戏内的玩家
    mySink.SetFilter(FilterAlivePlater);

    // 添加 mySink 到 myLogger 的 Sink 列表里
    myLogger.AddSink(mySink);

    /**
     * 现在 myLogger 有 2 个 Sink
     * 1 个是 "Logger.CreateServerConsoleLogger()" 时, 添加的 ServerConsoleSink
     * 1 个是 "myLogger.AddSink()" 时, 添加的 ClientConsoleSink
     *
     * 现在 mySink 的引用数为 2
     * 1 个是 "new ClientConsoleSinkST()" 时, 注册到 SinkRegister 的引用
     * 1 个是 "myLogger.AddSink()" 时, myLogger 对 mySink 的引用
     */

    // 如果后续不需要 mySink 修改, 可以直接 '删除' 它
    delete mySink;

    /**
     * 这不会导致 INVALID_HANDLE, 因为 myLogger 还引用着 ClientConsoleSink
     *
     * "delete mySink;" 实际上做的只是从拓展内部的 SinkRegister 里删除对 mySink 的引用
     * 所以 mySink 对象的引用数将 -1
     * 但只有引用数减少至 0 时，mySink 对象才会真正的被 '删除'
     *
     * 在本示例中即为   "delete mySink;"    +   "myLogger.DropSink(mySink);"
     * 也可以是         "delete mySink;"    +   "delete myLogger;"
     */

    RegConsoleCmd("sm_log4sp_example2", CommandCallback);

    myLogger.Info("===== Example 2 code initialization is complete! =====");
}

/**
 * ClientConsoleSink 的过滤器
 * 用于过滤掉死亡的玩家, 我们只输出日志信息给活着的玩家
 *
 * 只有 (IsClientInGame(client) && !IsFakeClient(client)) == true 时, 拓展才会调用 filter
 * 所以不必在 filter 内重复 IsClientInGame(client) 或 IsFakeClient(client)
 */
Action FilterAlivePlater(int client)
{
    return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
}

Action CommandCallback(int client, int args)
{
    static int count = 0;
    ++count;

    /**
     * 由于 "mySink.SetLevel(LogLevel_Info);" 修改了级别
     * 所以这条消息只会输出到 server console, 不会输出到 client console
     */
    myLogger.DebugAmxTpl("Command 'sm_log4sp_example2' was called (%d) times.", count);

    /**
     * 由于 "myLogger.SetLevel(LogLevel_Debug);" 修改后的日志级别大于 LogLevel_Trace
     * 所以这条消息不会输出到任何地方
     */
    myLogger.TraceAmxTpl("CommandCallback params: client (%d), args (%d)", client, args);

    if (args >= 2)
    {
        LogLevel lvl = view_as<LogLevel>(GetCmdArgInt(1));
        char buffer[256];
        GetCmdArg(2, buffer, sizeof(buffer));

        switch (lvl)
        {
            case LogLevel_Trace: myLogger.TraceAmxTpl("[%d] %s", count, buffer);
            case LogLevel_Debug: myLogger.DebugAmxTpl("[%d] %s", count, buffer);
            case LogLevel_Info:  myLogger.InfoAmxTpl("[%d] %s", count, buffer);
            case LogLevel_Warn:  myLogger.WarnAmxTpl("[%d] %s", count, buffer);
            case LogLevel_Error: myLogger.ErrorAmxTpl("[%d] %s", count, buffer);
            case LogLevel_Fatal: myLogger.FatalAmxTpl("[%d] %s", count, buffer);
            case LogLevel_Off:   myLogger.LogAmxTpl(LogLevel_Off, "[%d] %s", count, buffer);
            default:             myLogger.WarnAmxTpl("[%d] The level (%d) cannot be resolved.", count, lvl);
        }
    }
    return Plugin_Handled;
}
