**[英语](./readme.md) | [中文](./readme-chi.md)**

# Logging for SourcePawn

这是一个包装了 [spdlog](https://github.com/gabime/spdlog) 库的 Sourcemod 拓展，用于增强 SourcePawn 记录日志和调试功能。

### 特点

1. 非常快，比 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) 快得多

   - [spdlog 性能测试](https://github.com/gabime/spdlog#benchmarks)  |  [log4sp 性能测试](https://github.com/F1F88/sm-ext-log4sp/blob/main/readme-chi.md#%E6%80%A7%E8%83%BD%E6%B5%8B%E8%AF%95)

2. 每个 `Logger` 和 `Sink` 都能够自定义日志级别

3. 每个 `Logger` 和 `Sink` 都能够自定义[日志模板](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. 每个 `Logger` 都能够自定义[刷新策略](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)

5. 每个 `Logger` 都能够拥有多个 `Sink`

   - 例如： `ServerConsoleSink` + `DailyFileSink` 相当于 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

6. 每个 `Logger` 都能够动态调整日志级别、模板

   - 详见指令 `"sm log4sp"`

7. 支持异步 `Logger`

8. 支持格式化可变个数的参数

   - [参数格式化用法](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) 与 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) 相同

   - 可变参数的字符串最大长度为 **2048**，超过这个长度的字符会被截断
     如需要记录更长的日志消息，可以使用非 `AmxTpl` 的方法, 例如: `void Info(const char[] msg)`

9. 支持[日志回溯](https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support)

   - 启用后 `Trace` 和 `Debug` 级别的日志消息存储在一个环形缓冲区中, 只在显示调用 `DumpBacktrace()` 后才会输出

10. 支持多种 Sink

    - ServerConsoleSink（类似于 [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer)）

    - ClientConsoleSink（类似于 [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole)）

    - BaseFileSink （类似于 [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) 为 0 时的 [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile)）

    - DailyFileSink（类似于 [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) 为 0 时的 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)）

    - RotatingFileSink

### 安装

1. 从 [Github Action](https://github.com/F1F88/sm-ext-log4sp/actions) 里下载最新版压缩包，注意选择与操作系统以及 sourcemod 版本匹配的版本

2. 将压缩包中的 `"addons/sourcemod/extensions/log4sp.ext.XXX"` 上传到 `"game/addons/sourcemod/extensions"` 文件夹内

3. 使用 `#include <log4sp>` 引入头文件后，即可使用 log4sp.ext 提供的 API（natives 函数）

### 配置

通常 `log4sp.ext` 使用默认值进行初始化，你也可以在 `"game/addons/sourcemod/configs/core.cfg"` 中添加键值来修改默认设置：

```
"Log4sp_ThreadPoolQueueSize"	"<输入线程池队列大小>"
"Log4sp_ThreadPoolThreadCount"	"<输入线程池线程个数>"
```

修改后的 `core.cfg` 最终看起来像这样：

```
"Core"
{
	[...]

	"Log4sp_ThreadPoolQueueSize"	"8192"
	"Log4sp_ThreadPoolThreadCount"	"1"
}
```

下面是完整的可用键值及其默认值和文档，你应该只添加要修改的项到 `core.cfg` 中：

```
"Core"
{
	[...]

	/**
	 * 线程池的队列大小
	 */
	"Log4sp_ThreadPoolQueueSize"	"8192"

	/**
	 * 线程池的工作线程个数
	 */
	"Log4sp_ThreadPoolThreadCount"	"1"

	/**
	 * 默认 Logger 的名称
	 */
	"Log4sp_DefaultLoggerName"		"LOG4SP"

	/**
	 * 默认 Logger 的类型
	 * 0 = 异步，队列已满时阻塞
	 * 1 = 异步，队列已满时丢弃旧的消息
	 * 2 = 异步，队列已满时丢弃新的消息
	 * 其他 = 同步
	 */
	"Log4sp_DefaultLoggerType"		"1"

	/**
	 * 默认 Logger 的日志级别
	 * 可选项："trace", "debug", "info", "warn", "error", "fatal", "off"
	 */
	"Log4sp_DefaultLoggerLevel"		"info"

	/**
	 * 默认 Logger 的日志消息模板
     * 可选项：https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
	 */
	"Log4sp_DefaultLoggerPattern"	"[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v"

	/**
	 * 默认 Logger 的刷新级别
	 * 可选项："trace", "debug", "info", "warn", "error", "fatal", "off"
	 */
	"Log4sp_DefaultLoggerFlushOn"	"off"

	/**
	 * 默认 Logger 的回溯消息条数
	 * 0 = 关闭回溯
	 */
	"Log4sp_DefaultLoggerBacktrace"	"0"
}
```

### 使用

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

##### 更多使用示例

- [API 测试](./sourcemod/scripting/log4sp-test.sp)

- [性能测试](./sourcemod/scripting/log4sp-benchmark.sp)

### 支持的游戏

`log4sp.ext` 应该适用于所有游戏，但目前只提供了 Linux 二进制文件

对于 Windows 系统你需要额外做的是编译这个项目，然后把编译生成的 `log4sp.ext.dll` 上传到 `"game/addons/sourcemod/extensions"`

### 性能测试

测试平台: Windows 11 + VMware + Ubuntu 24.04 LTS + sourcemod 1.11

宿主机配置: AMD Ryzen 7 6800H + 32 GB 内存

VMware 配置: 1 CPU + 8 核心 + 4 GB 内存

测试用例1：[benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

测试用例2：[benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)

##### 单线程 （同步）

```
[benchmark] base-file-st      | Iters 1000000 | Elapsed  0.268 secs   3719518/sec
[benchmark] daily-file-st     | Iters 1000000 | Elapsed  0.278 secs   3589439/sec
[benchmark] rotating-file-st  | Iters 1000000 | Elapsed  0.279 secs   3578598/sec
[benchmark] server-console-st | Iters 1000000 | Elapsed  5.609 secs    178255/sec
```

##### 多线程 （异步）

```
# 队列大小：8192      线程数：1
[benchmark] base-file-block        | Iters 1000000 | Elapsed  0.479 secs   2084762/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed  0.488 secs   2046592/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed  0.462 secs   2162868/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed  8.422 secs    118725/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed  0.442 secs   2259856/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed  0.438 secs   2280891/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed  0.442 secs   2260684/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed  0.379 secs   2632167/sec


# 队列大小：8192      线程数：4
[benchmark] base-file-block        | Iters 1000000 | Elapsed  1.049 secs    952753/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed  1.086 secs    920584/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed  1.034 secs    967049/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed 15.784 secs     63354/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed  0.439 secs   2273952/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed  0.451 secs   2212609/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed  0.453 secs   2204658/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed  0.372 secs   2684282/sec


# 队列大小：131072    线程数：4
[benchmark] base-file-block        | Iters 1000000 | Elapsed   0.998 secs   1001216/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed   0.973 secs   1027070/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed   0.956 secs   1045255/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed  13.952 secs     71671/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed   0.472 secs   2116635/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed   0.441 secs   2264892/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed   0.478 secs   2091503/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed   0.385 secs   2592245/sec


# 队列大小：8192      线程数：8
[benchmark] base-file-block        | Iters 1000000 | Elapsed  1.135 secs    881010/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed  1.183 secs    845069/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed  1.193 secs    838199/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed 14.925 secs     67000/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed  0.533 secs   1875363/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed  0.569 secs   1754767/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed  0.508 secs   1967969/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed  0.394 secs   2532556/sec
```

##### Sourcemod logging

作为参考, 还测试了 sourcemod 的 [logging API](https://sm.alliedmods.net/new-api/logging)


```
[benchmark] LogMessage    | Iters 1000000 | Elapsed 10.740 secs     93108/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed  9.091 secs    109989/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed  8.823 secs    113336/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  5.779 secs    173024/sec
```

### 疑难解答

##### 加载插件时报错

错误信息：`"[SM] Unable to load plugin "XXX.smx": Required extension "Logging for SourcePawn" file("log4sp.ext") not running"`

- 检查是否已将 `log4sp.ext.XXX` 文件上传到 `"game/addons/sourcemod/extensions"`
- 检查日志信息, 查看 log4sp.ext.XXX 加载失败的原因并解决

##### 加载拓展时报错

错误信息：`"[SM] Unable to load extension "log4sp.ext": Could not find interface: XXX"`

- 检查 `log4sp.ext.XXX` 与操作系统是否匹配
- 检查 `log4sp.ext.XXX` 的版本与 sourcemod 版本是否匹配

错误信息：`"bin/libstdc++.so.6: version 'GLIBCXX_3.4.20' not found"`

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
    # 如果操作系统有，但服务器没有
    # 可以删除或者重命名服务器的 ./server/bin/libstdc++.so.6 文件，从而使用操作系统的版本
    mv ./server/bin/libstdc++.so ./server/bin/libstdc++.so.bk
    ```

- 也可以看看：[wiki](https://wiki.alliedmods.net/Installing_Metamod:Source#Normal_Installation)

### 编译依赖

- [sourcemod](https://github.com/alliedmodders/sourcemod/tree/1.11-dev)

- [spdlog](https://github.com/gabime/spdlog)（仅需[头文件](https://github.com/gabime/spdlog/tree/v1.x/include/spdlog)，已包含在 ["./extern/spdlog/include/spdlog"](./extern/spdlog/include/spdlog)）

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

### 制作人员

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 库实现了绝大部分功能，此拓展将 spdlog API 包装后提供给 SourcePawn 使用

- Fyren, nosoop, Deathreus 为拓展管理 Sink Handle 提供了解决思路

如有遗漏，请联系我

### 待办

1. 支持通过配置文件配置全局线程池

2. 支持通过配置文件配置默认 Logger

3. 编译 Windows 系统的 log4sp.ext.dll 并测试

4. 支持 XXXAmxTpl 格式化的字符串无限长度

    - 目前 log4sp::FormatToAmxTplString 的缓冲区长度是 2048

5. 支持参数格式不匹配时不抛出异常

    - 目前 XXXAmlTpl 调用 sm 的 API - `smutils->FormatString` 来格式化

    - 如果输入的模板与输入的参数类型不匹配，`smutils->FormatString` 会直接抛出异常

    - 值得考虑：其他API是否也可需要减少抛出异常？

6. 支持 fmt 风格的格式化方案

    - 可能需要自定义一个 cell_t 类型来识别参数

    - 如何处理参数个数的问题？也许可以参考 flink tuple

7. 实现更多的 SPDLOG_API

    - 如果有时间的话

    - 通过 forward 实现 logger、sinks 的 file_event_handlers

    - 实现更多类型 Sink

    - 支持纯 sourcepawn 自定义 sink

      - 使用一个代理 sink，通过 forward 来调用 sp 自定义的 sink_it\_() 和 flush\_()

      - 自定义构造器、以及如何识别自定义的 sink 类型还需要进一步考虑
