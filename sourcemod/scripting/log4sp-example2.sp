#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Log4sp Example 2"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.0.1"
#define PLUGIN_DESCRIPTION                  "Logging for SourcePawn example 2"
#define PLUGIN_URL                          "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};

#include <log4sp>

/**
 * ClientConsoleSink 的过滤器
 * 用于过滤掉死亡的玩家, 我们只输出日志信息给活着的玩家
 */
Action FilterAlivePlater(int client)
{
    /**
     * 拓展在调用这个 filter 前已经判断了玩家的有效性
     *  if(IsClientInGame(client) == false || IsFakeClient(client) == true)
     *      return;
     *  call filter...
     */
    return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
}

Logger myLogger;

public void OnPluginStart()
{
    // 创建一个 单线程的, 名为 "myLogger" 的 Logger 对象
    myLogger = Logger.CreateServerConsoleLogger("logger-example-2");

    // 配置 myLogger 的输出级别. 默认级别: LogLevel_Info
    myLogger.SetLevel(LogLevel_Debug);

    // 配置日志格式. 默认格式: [2014-10-31 23:46:59.678] [my_loggername] [info] Some message
    myLogger.SetPattern("[Date: %Y/%m/%d %H:%M:%S] [Name: %n] [LogLevel: %l] [message: %v]");

    // 创建一个输出到玩家控制台的输出源 mySink
    // 每个手动创建的 sink 都会注册到拓展内部的 sink register, 即引用数为 1
    ClientConsoleSinkST mySink = new ClientConsoleSinkST();

    // 单独设置 mySink 的级别. 默认为 LogLevel_Trace
    mySink.SetLevel(LogLevel_Info);

    // 设置 mySink 过滤规则为：只输出到活着的玩家的控制台. 默认输出到所有玩家控制台
    mySink.SetFilter(FilterAlivePlater);

    // 将 mySink 输出源添加到记录器 myLogger
    myLogger.AddSink(mySink);

    // 现在 myLogger 有 2 个输出源
    // 1 个是调用 CreateServerConsoleLogger 时添加的输出到服务器控制台的 sink
    // 1 个是调用 AddSink 时添加的输出到活着的玩家控制台的 sink

    // 现在 mySink 有 2 个引用
    // 1 个是 new ClientConsoleSinkST() 后注册到 sink register 的引用
    // 1 个是 myLogger.AddSink() 后 myLogger 对 mySink 的引用

    // 如果你不在需要对 mySink 修改, 可以直接 "删除" 它
    // 这不会发生错误, myLogger 还是可以输出到 server console 以及 client console
    delete mySink;

    // delete mySink; 实际上做的只是从拓展内部的 sink register 里注销 mySink
    // 注销后, mySink 的引用从 2 个减少为 1 个
    // 只有引用归 0 后，mySink 对象才会真正的被 "删除"

    // 在本示例中即为: "delete mySink;"  +  "delete myLogger;"
    // 你也可以使用: "myLogger.DropSink(mySink);"  +  "delete mySink;"

    // 现在我们自定义的 Logger 配置完成了，尽情使用吧
    myLogger.Info("===== Example 2 code initialization is complete! =====");

    // 注册一个命令用于演示
    RegConsoleCmd("sm_log4sp_example2", CommandCallback);
}

Action CommandCallback(int client, int args)
{
    static int count = 0;
    // mySink 的级别是 LogLevel_Warn 所以这条消息不会输出到 client console
    myLogger.DebugAmxTpl("Command 'sm_log4sp_example2' is invoked %d times.", ++count);

    // myLogger 的级别是 LogLevel_Debug 所以这条消息不会输出到任何地方
    myLogger.TraceAmxTpl("CommandCallback param client = %L param args = %d.", client, args);

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
