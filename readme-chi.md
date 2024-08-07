**[英语](./readme.md) | [中文](./readme-chi.md)**

# Logging for SourcePawn

这是一个包装了 [spdlog](https://github.com/gabime/spdlog) 库的 Sourcemod 拓展，用于增强 SourcePawn 记录日志和调试功能。

### 使用

1. 从 [Github Action](https://github.com/F1F88/sm-ext-log4sp/actions) 下载最新版压缩包，注意选择与操作系统和 sourcemod 版本匹配的版本
2. 将压缩包中的 `addons/sourcemod/extension/log4sp.ext.XXX` 上传到服务器的 `game/addons/sourcemod/extension` 文件夹

### 特点

1. 非常快，比 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) 快得多

   - [spdlog 性能测试](https://github.com/gabime/spdlog#benchmarks)  |  [log4sp 性能测试](https://github.com/F1F88/sm-ext-log4sp#benchmarks)

2. 每个 `Logger` (记录器) 和 `Sink` (输出源) 都可以自定义日志级别

3. 每个 `Logger` (记录器) 和 `Sink` (输出源) 都可以自定义[日志模板](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. 每个 `Logger` (记录器) 和 `Sink` (输出源) 都可以自定义[刷新策略](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)

5. 每个 `Logger` 都可以拥有多个 `Sink`

   - 例如： `ServerConsoleSink` + `DailyFileSink` 相当于 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

6. 每个 `Logger` (记录器)  都可以动态调整日志级别、模板

   - 详见指令`"sm log4sp"`

7. 支持异步 `Logger` (记录器)

8. 支持格式化可变个数的参数

   - [参数格式化用法](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) 与 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) 相同

   - 可变参数的字符串最大长度为 **2048**，超过这个长度的字符会被截断
     如需要记录更长的日志消息，可以使用非 `AmxTpl` 的方法, 例如: `void Info(const char[] msg)`

9. 支持[日志回溯](https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support)

10. 支持多种 Sink

   - ServerConsoleSink （类似于 [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer)）

   - ClientConsoleSink （类似于 [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole)）

   - BaseFileSink （类似于 [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) 为 0 时的 [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile))

   - DailyFileSink (类似于 [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) 为 0 时的 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage))

   - RotatingFileSink

### 使用示例

##### 一个简单的示例

```sourcepawn
#include <sourcemod>
#include <sdktools>
#include <log4sp>

Logger myLogger;

public void OnPluginStart()
{
    // 默认日志级别: LogLevel_Info
    // 默认日志模板: [%Y-%m-%d %H:%M%S.e] [%n] [%l] %v
    myLogger = Logger.CreateServerConsoleLogger("logger-example-1");

    RegConsoleCmd("sm_log4sp_example1", CommandCallback);

    // Debug 小于默认日志级别 Info, 所以这行代码不会输出日志消息
    myLogger.Debug("===== 示例 1 代码初始化成功! =====");
}

/**
 * 获取玩家瞄准的实体信息
 */
Action CommandCallback(int client, int args)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client))
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [info] 指令只能在游戏中使用
        myLogger.Info("指令只能在游戏中使用");
        return Plugin_Handled;
    }

    int entity = GetClientAimTarget(client, false);
    if (entity == -2)
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [fatal] 当前游戏不支持 GetClientAimTarget() 函数
        myLogger.Fatal("当前游戏不支持 GetClientAimTarget() 函数");
        return Plugin_Handled;
    }

    if (entity == -1)
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [warn] 玩家 (1) 没有瞄准实体
        myLogger.Warn("玩家 (%d) 没有瞄准实体", client);
        return Plugin_Handled;
    }

    if (!IsValidEntity(entity))
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [error] 实体 (444) 是无效的
        myLogger.ErrorAmxTpl("实体 (%d) 是无效的", entity);
        return Plugin_Handled;
    }

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));

    // [2024-08-01 12:34:56.789] [logger-example-1] [info] 玩家 (1) 瞄准了一个 (403 - prop_door_breakable) 实体
    myLogger.InfoAmxTpl("玩家 (%d) 瞄准了 1 个 (%d - %s) 实体", client, entity, classname);

    return Plugin_Handled;
}
```

##### 更详细一点的示例

```sourcepawn
#include <sourcemod>
#include <log4sp>

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
    ClientConsoleSinkST mySink = new ClientConsoleSinkST();

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

    myLogger.Info("===== 示例 2 代码初始化成功! =====");
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
    myLogger.DebugAmxTpl("指令 'sm_log4sp_example2' 调用了 (%d) 次", count);

    /**
     * 由于 "myLogger.SetLevel(LogLevel_Debug);" 修改后的日志级别大于 LogLevel_Trace
     * 所以这条消息不会输出到任何地方
     */
    myLogger.TraceAmxTpl("CommandCallback 参数： client (%d), args (%d)", client, args);

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
```

##### 更多示例

- [./sourcemod/scripting/log4sp-test.sp](./sourcemod/scripting/log4sp-test.sp)

- [./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)

### 性能测试

测试平台: Windows 11 23H2 + VMware + Ubuntu 24.04 LTS + NMRIH Dedicated Server v1.13.6 + SM 1.11

宿主机硬件配置: AMD Ryzen 7 6800H + 32GB 内存

VMware 配置: 1 CPU + 8 核心 + 4GB 内存

##### 单线程(同步)

[./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)

```
sm_log4sp_bench_files_st
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext API Single thread, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] base-file-st__________   Elapsed:  0.29 secs      3415020 /sec
[log4sp-benchmark] rotating-file-st______   Elapsed:  0.30 secs      3239957 /sec
[log4sp-benchmark] daily-file-st_________   Elapsed:  0.30 secs      3314001 /sec

sm_log4sp_bench_server_console_st
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext Server Console Single thread, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] server-console-st_____   Elapsed:  5.74 secs       174034 /sec
```

##### 多线程(异步)

[./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)

```
sm_log4sp_bench_files_async
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext API Asynchronous mode, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: block
[log4sp-benchmark] *********************************
[log4sp-benchmark] base-file-block_______   Elapsed:  0.49 secs      2031570 /sec
[log4sp-benchmark] rotating-file-block___   Elapsed:  0.49 secs      2006179 /sec
[log4sp-benchmark] daily-file-block______   Elapsed:  0.49 secs      2020079 /sec
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: overrun
[log4sp-benchmark] *********************************
[log4sp-benchmark] base-file-overrun_____   Elapsed:  0.46 secs      2158032 /sec
[log4sp-benchmark] rotating-file-overrun_   Elapsed:  0.44 secs      2236701 /sec
[log4sp-benchmark] daily-file-overrun____   Elapsed:  0.46 secs      2169291 /sec

sm_log4sp_bench_server_console_async
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext Server Console Asynchronous mode, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: block
[log4sp-benchmark] *********************************
[log4sp-benchmark] server-console-block__   Elapsed:  8.56 secs       116712 /sec
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: overrun
[log4sp-benchmark] *********************************
[log4sp-benchmark] server-console-overrun   Elapsed:  8.44 secs       118378 /sec
```

##### Sourcemod logging API

作为参考, 也用 [./sourcemod/scripting/sm-logging-benchmark.sp](./sourcemod/scripting/sm-logging-benchmark.sp) 测试了 Sourcemod 的 [logging API](https://sm.alliedmods.net/new-api/logging)


```
sm_log4sp_bench_sm_logging
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Sourcemod Logging API, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] LogMessage               Elapsed: 10.99 secs        90979 /sec
[log4sp-benchmark] LogToFile                Elapsed:  8.91 secs       112111 /sec
[log4sp-benchmark] LogToFileEx              Elapsed:  9.07 secs       110141 /sec
 *
sm_log4sp_bench_sm_console
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Sourcemod Console API, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] PrintToServer            Elapsed:  5.86 secs       170446 /sec
```

### 依赖

- [sourcemod](https://github.com/alliedmodders/sourcemod/tree/1.11-dev)

- [spdlog](https://github.com/gabime/spdlog)

  仅需[头文件](https://github.com/gabime/spdlog/tree/v1.x/include/spdlog)，已包含在 [./extern/spdlog/include/spdlog](./extern/spdlog/include/spdlog)

### 基本编译步骤

##### Linux

```shell
cd sm-ext-log4sp
mkdir build && cd build
# 把 $SOURCEMOD_HOME 替换为你的 sourcemod 环境变量或路径. 如 "~/sourcemod"
python3 ../configure.py --enable-optimize --sm-path $SOURCEMOD_HOME
ambuild
```

##### Windows

我不知道

### 疑难解答

#### 加载插件时报错

##### [SM] Unable to load plugin "XXX.smx": Required extension "Logging for SourcePawn" file("log4sp.ext") not running

1. 检查是否已将 `log4sp.ext.XXX` 文件上传到 `addons/sourcemod/extensions`

2. 检查日志信息, 查看 log4sp.ext.XXX 加载失败的原因并解决

#### 加载拓展时报错

##### [SM] Unable to load extension "log4sp.ext": Could not find interface: XXX

1. 检查 `log4sp.ext.XXX` 与操作系统是否匹配
2. 检查 `log4sp.ext.XXX` 的版本与 sourcemod 版本是否匹配

##### bin/libstdc++.so.6: version 'GLIBCXX_3.4.20' not found

- 方案一

    ```shell
    # 参考链接：https://stackoverflow.com/questions/44773296/libstdc-so-6-version-glibcxx%203-4-20-not-found
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install gcc-4.9

    # 如果问题解决，则不需要下面这一步
    # sudo apt-get upgrade libstdc++6
    ```

- 方案二：

    ```shell
    # 先查看操作系统是否有需要的 GLIBCXX 版本
    strings /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep GLIBCXX
    # 再查看服务器是否有需要的 GLIBCXX 版本
    strings ./server/bin/libstdc++.so | grep GLIBCXX
    # 如果操作系统有，但服务器没有，可以尝试重命名服务器的 ./server/bin/libstdc++.so.6 文件，从而使用操作系统的版本
    mv ./server/bin/libstdc++.so ./server/bin/libstdc++.so.bk
    ```

## 制作人员

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 库实现了绝大部分功能，拓展将 spdlog API 包装后提供给 SourcePawn 使用

- Fyren, nosoop, Deathreus 为拓展管理 Sink Handle 提供了解决思路

如有遗漏，请联系我

## 待办

1. 支持通过配置文件配置全局线程池
2. 支持通过配置文件配置默认 Logger
3. server command 支持修改 flush level
4. 编译 Windows 系统的 log4sp.ext.dll 并测试
5. 支持 XXXAmxTpl 格式化的字符串无限长度
6. 支持参数格式不匹配时不抛出异常
    - 没记错的话 spdlog 也是这么做的
    - 目前 XXXAmlTpl 调用 sm 内部的 API，如果格式与参数不匹配会直接抛出异常
    - 值得考虑：其他API是否也可需要减少抛出异常？
7. 支持 fmt 风格的格式化方案
    - 可能需要自定义一个 cell_t 类型来识别参数
    - 如何处理参数个数的问题？也许可以参考 flink tuple
8. 实现更多不同类型的 sink
    - 如果有时间的话
9. 支持纯 sourcepawn 自定义 sink
    - 使用一个代理 sink，通过 forward 来调用 sp 自定义的 sink_it\_() 和 flush\_()
    - 自定义构造器、以及如何识别自定义的 sink 类型还需要进一步考虑
10. 实现更多的 SPDLOG_API
    - 使用 forward 实现 logger、sinks 的 file_event_handlers
