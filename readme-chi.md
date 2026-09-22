**[English](./readme.md) | [中文](./readme-chi.md)**

# Log4sp

Log4sp 是基于 [spdlog](https://github.com/gabime/spdlog) 构建的高性能日志库，支持仅头文件和扩展库两种使用方式，旨在帮助开发者高效地记录、格式化和管理应用程序日志。Log4sp 充分利用了 spdlog 的卓越性能与灵活性，为 SourceMod 提供了一个更易于集成的日志接口，能够满足从轻量级插件到大型高性能项目的日志记录需求。

借助 log4sp，开发者可以轻松跟踪插件运行状态、定位问题并监控性能，同时利用高性能日志库降低日志记录对插件运行效率的影响。

## 特点

1. 非常快，比 [SourceMod API - Logging](https://sm.alliedmods.net/new-api/logging) 快得多
2. 支持日志过滤 - 可以在编译时与运行时修改 [日志级别](#日志级别)
3. 支持超大日志 - 超过 [1024](https://github.com/alliedmodders/sourcemod/blob/be89b25d96486900a57e07661f992836479f4fc4/core/logic/smn_filesystem.cpp#L936-L970) 个字符也不会被[截断](#参数格式化)
4. 支持自定义[日志样式](#模板格式化)
5. 支持日志操作[无错误中断](#错误处理器)
6. 支持多种[输出源](./sourcemod/scripting/include/log4sp/sinks)
7. 支持控制台指令以及管理菜单
8. 支持仅头文件模式（纯 SourcePawn）
9. 支持 int64
10. 支持 x64

## 安装

1. 从 [Releases](https://github.com/F1F88/sm-ext-log4sp/releases) 中下载合适的版本
    - `sm-ext-log4sp` 包含扩展库以及日志操作相关 API 的头文件
    - `sm-plugin-log4sp_manager` 包含管理插件以及管理记录器相关 API 的头文件
2. 把压缩包中的文件复制到服务器的 "addons/sourcemod" 目录下

## 用例

Natives 文档：[./sourcemod/scripting/include/log4sp/](./sourcemod/scripting/include/log4sp)

**Hello World**

```sourcepawn
#include <sourcemod>
#include <log4sp>

public void OnPluginStart()
{
    ServerConsoleSink sink = new ServerConsoleSink();
    Logger logger = new Logger("my-logger");
    logger.AddSink(sink);

    logger.Info("Hello World!");

    sink.Close();
    logger.Close();
}
```

服务器控制台输出：

> [2001-02-03 12:34:56.789] [my-logger] [info] Hello World!<br>

### 日志级别

Log4sp 定义了 **`7`** 个日志级别，从低往高依次为：**`trace`** < **`debug`** < **`info`** < **`warn`** < **`error`** < **`fatal`** < **`off`**。

仅当日志消息级别 **≥** Logger 的日志级别时，才会格式化日志消息并传递到 Sinks；

仅当日志消息级别 **≥** Sink 的日志级别时，才会记录日志消息到此 sink。

开发者可以根据日志级别筛选日志，确保仅记录相关日志消息，从而提高调试效率并保持日志文件的整洁有序。

**Logger** 的默认日志级别为 `info`；

**Sink** 的默认日志级别为 `trace`。

```sourcepawn
logger.SetLevel(LogLevel_Warn);     // 修改 Logger 的日志级别为 warn
logger.ShouldLog(LogLevel_Warn);    // true

sink.SetLevel(LogLevel_Debug);      // 修改 Sink 的日志级别为 debug
sink.ShouldLog(LogLevel_Trace);     // false
```

```sourcepawn
#include <sourcemod>
#include <log4sp>

public void OnPluginStart()
{
    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "logs/simple-file.log");
    BasicFileSink sink = new BasicFileSink(path);

    Logger logger = new Logger("my-logger");
    logger.AddSink(sink);
    logger.SetLevel(LogLevel_Warn);

    logger.Debug("Test ...");
    logger.Info("Information ...");
    logger.Warn("Warning ...");
    logger.Error("Oops ...");

    sink.Close();
    logger.Close();
}
```

文件 `game/addons/sourcemod/logs/simple-file.log`：

> [2001-02-03 12:34:56.789] [my-logger] [warn] Warning ...<br>
> [2001-02-03 12:34:56.789] [my-logger] [error] Oops ...<br>

### 参数格式化

以 [**Logger::Log**](./sourcemod/scripting/include/log4sp/logger.inc#L64) 为例，普通 `Log` 方法只会按原样输出日志消息，而 `LogF` 方法则先将参数格式化，再输出格式化后的日志消息。

参数格式化在 **Logger** 层执行，且仅当日志消息级别 **≥** logger 日志级别时才会触发。

|                                                              |                          SM Logging                          |                           仅头文件                           |                            扩展库                            |
| :----------------------------------------------------------- | :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| **最多字符数**                                               |                             1024                             |            宏 `LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH`            |                            无限制                            |
| **实现**                                                     | [SM Format](https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.h#L40) | [SM Format](https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.h#L40) |          [Log4sp Format](./src/log4sp/format.h#L10)          |
| **用法**                                                     | [Format wiki](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) | [Format wiki](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) | [Format wiki](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) |
| **格式错误**                                                 |                           抛出错误                           |                           抛出错误                           |                      调用 Error Handler                      |
| **填充 [BUG](https://github.com/alliedmodders/sourcemod/issues/2221)** | 修复于 [1.13.0.7198](https://github.com/alliedmodders/sourcemod/pull/2255) | 修复于 [1.13.0.7198](https://github.com/alliedmodders/sourcemod/pull/2255) |                        修复于 v1.5.0                         |
| **浮点 [Inf](https://github.com/alliedmodders/sourcemod/issues/2110)** | 新增于 [1.13.0.7269](https://github.com/alliedmodders/sourcemod/pull/2324) | 新增于 [1.13.0.7269](https://github.com/alliedmodders/sourcemod/pull/2324) |                        修复于 v1.10.0                        |
| **减号 [BUG](https://github.com/alliedmodders/sourcemod/issues/2328)** | 修复于 [1.13.0.7270](https://github.com/alliedmodders/sourcemod/pull/2329) | 修复于 [1.13.0.7270](https://github.com/alliedmodders/sourcemod/pull/2329) |                        修复于 v1.8.0                         |
| **对齐 [BUG](https://github.com/alliedmodders/sourcemod/issues/2331)** | 修复于 [1.13.0.7271](https://github.com/alliedmodders/sourcemod/pull/2332) | 修复于 [1.13.0.7271](https://github.com/alliedmodders/sourcemod/pull/2332) |                        修复于 v1.5.0                         |
| **通配符 [%E](https://github.com/alliedmodders/sourcemod/issues/2099)** | 新增于 [1.13.0.7276](https://github.com/alliedmodders/sourcemod/pull/2330) | 新增于 [1.13.0.7276](https://github.com/alliedmodders/sourcemod/pull/2330) |                        新增于 v1.10.0                        |
| **通配符 [%ld, %li, %lu](https://github.com/alliedmodders/sourcemod/issues/2413)** | 新增于 [1.13.0.7326](https://github.com/alliedmodders/sourcemod/pull/2421) | 新增于 [1.13.0.7326](https://github.com/alliedmodders/sourcemod/pull/2421) |                        新增于 v1.11.0                        |
| **浮点数 [-Inf](https://github.com/alliedmodders/sourcemod/issues/2444)** | 新增于 [1.13.0.7330](https://github.com/alliedmodders/sourcemod/pull/2444) | 新增于 [1.13.0.7330](https://github.com/alliedmodders/sourcemod/pull/2444) |                        新增于 v1.11.0                        |
| **填充 [BUG](https://github.com/alliedmodders/sourcemod/pull/2443)** | 修复于 [1.13.0.7331](https://github.com/alliedmodders/sourcemod/pull/2443) | 修复于 [1.13.0.7331](https://github.com/alliedmodders/sourcemod/pull/2443) |                        修复于 v1.8.0                         |
| **通配符 [%lb, %lX, %lx](https://github.com/alliedmodders/sourcemod/pull/2448)** | 新增于 [1.13.0.7342](https://github.com/alliedmodders/sourcemod/pull/2448) | 新增于 [1.13.0.7342](https://github.com/alliedmodders/sourcemod/pull/2448) |                        新增于 v1.11.0                        |

```sourcepawn
#include <sourcemod>
#include <log4sp>

public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "logs/daily-file.log");
    DailyFileSink sink = new DailyFileSink(path);

    Logger logger = new Logger("my-logger");
    logger.AddSink(sink);

    logger.InfoF("d: %d, u: %u, b: %b", 1, 2, 3);
    logger.WarnF("f: %f, x: %x, X: %X", 4.0, 5, 6);
    logger.ErrorF("s: %s, c: %c, T: %T", "Some String", '!', "Yes", LANG_SERVER);

    sink.Close();
    logger.Close();
}
```

文件 `game/addons/sourcemod/logs/daily-file_20010203.log`：

> [2001-02-03 12:34:56.789] [my-logger] [info] d: 1, u: 2, b: 11<br>
> [2001-02-03 12:34:56.789] [my-logger] [warn] f: 4.000000, x: 5, X: 6<br>
> [2001-02-03 12:34:56.789] [my-logger] [error] s: Some String, c: !, T: Yes<br>

### 模板格式化

日志模板是一种格式化日志消息的配置机制，允许通过时间戳、严重级别、记录器名称等元数据字段，自定义日志的呈现样式。

模板格式化在 **Sink** 层完成，该层先对模板进行解析和渲染，再将格式化后的日志消息输出。

Sink 的默认模板与输出样例如下：

```
[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#] %v
```

```
[2001-02-03 12:34:56.789] [my-logger] [info] [example.sp:123] Hello World!
```

您可以参考如下代码修改模板：

```sourcepawn
sink.SetPattern("[%Y-%m-%d %H:%M:%S] [%n] [%l] %v 1");   // 修改单个 sink 的模板
logger.SetPattern("[%Y-%m-%d %H:%M:%S] [%n] [%l] %v 2"); // 修改全部 sinks 的模板
```

完整通配符文档： [\<spdlog wiki> - Custom-formatting](https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags)

```sourcepawn
#include <sourcemod>
#include <log4sp>

public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "logs/rotate-file.log");
    const int maxFileSize = 1024 * 10;
    const int maxFiles = 3;
    RotatingFileSink sink = new RotatingFileSink(path, maxFileSize, maxFiles);

    Logger logger = new Logger("my-logger");
    logger.AddSink(sink);

    logger.LogSrc(LogLevel_Info, "Some message");
    logger.SetPattern("[%D %r] [%n] [%L] [%!:%#] %v");
    logger.LogSrc(LogLevel_Info, "Some message");

    sink.Close();
    logger.Close();
}
```

文件 `game/addons/sourcemod/logs/rotate-file.log`：

> [2001-02-03 12:34:56.789] [my-logger] [info] [test.sp:17] Some message<br>
> [02/03/01 12:34:56 PM] [my-logger] [I] [OnPluginStart:19] Some message<br>

> [!TIP]
> 仅头文件为固定模板，需要修改源码或者自定义 Sink 才能修改模板。

### 刷写策略

Log4sp 让底层 libc 在[认为合适时](https://github.com/gabime/spdlog/wiki/Flush-policy)刷写缓冲区，以实现良好的性能。

您可以通过以下方式覆盖这一点：

1. 手动刷写

    ```sourcepawn
    sink.Flush();   // 刷写单个 sink 的缓冲区
    logger.Flush(); // 刷写所有 sinks 的缓冲区
    ```

2. 自动刷写

    ```sourcepawn
    logger.SetFlushLevel(LogLevel_Warn); // 当日志消息级别 ≥ "Warn" 时，立即刷写缓冲区
    ```

    > [!TIP]
    > Logger 的默认自动刷写级别为 LogLevel_Off （不启用自动刷写）。

3. 定期刷写

    ```sourcepawn
    #include <sourcemod>
    #include <log4sp>

    Logger g_hLogger;

    public void OnPluginStart()
    {
        g_hLogger = new Logger("my-logger");
        CreateTimer(5.0, Timer_Flush, _, TIMER_REPEAT);
    }

    Action Timer_Flush(Handle timer)
    {
        g_hLogger.Flush();
        return Plugin_Continue;
    }
    ```

### 错误处理器

通常，Natives 会在参数无效时抛出错误并中断代码的执行。

但以下情况不会直接抛出错误，而是交由错误处理器处理，从而避免中断 SourcePawn 代码的执行：

1. Logger.LogF 格式化参数时的错误
2. Logger 遍历 Sinks 记录日志时的错误
3. Logger 遍历 Sinks 刷写日志时的错误

每个 Logger 都有一个错误处理器，默认处理器的方案是简单将错误信息记录到 SourceMod 的 errors.log 文件。

您可以参考如下代码覆盖 Logger 的默认错误处理器：

```sourcepawn
void SetMyErrorHandler(Logger logger)
{
    logger.SetErrorHandler(null, MyErrorHandler);
}

void MyErrorHandler(const char[] origin, SourceLoc loc, const char[] msg)
{
    // source 取文件名拼接行号
    char source[PLATFORM_MAX_PATH];
    if (!loc.IsEmpty())
    {
        int sepOffset = 0;
        for (int i = 0; i < sizeof(SourceLoc::filename); ++i)
        {
            if (!loc.filename[i])
                break;
            if (loc.filename[i] == '\\' || loc.filename[i] == '/')
                sepOffset = i + 1;
        }
        FormatEx(source, sizeof(source), "[%s::%d] ",
                 loc.filename[sepOffset], loc.line);
    }
    LogError("[%s] %s%s", origin, source, msg);
}
```

>  [!tip]
>
>  仅头文件 `Logger.LogF` 格式化参数错误将直接抛出错误信息。

### 多个输出源

该库支持将日志消息发送到多种目标位置，包括控制台、文件和自定义接收器。这使开发人员能够完全控制日志的存储、可见性和访问性，从而更轻松地监控、维护和排查应用程序故障。

```sourcepawn
#include <sourcemod>
#include <log4sp>

public void OnPluginStart()
{
    char basicFile[PLATFORM_MAX_PATH], dailyFile[PLATFORM_MAX_PATH], rotatingFile[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, basicFile, sizeof(basicFile), "logs/log4sp-multi-basic-sink.log");
    BuildPath(Path_SM, dailyFile, sizeof(dailyFile), "logs/log4sp-multi-daily-sink.log");
    BuildPath(Path_SM, rotatingFile, sizeof(rotatingFile), "logs/log4sp-multi-rotating-sink.log");

    BasicFileSink basicFileSink = new BasicFileSink(basicFile);
    DailyFileSink dailyFileSink = new DailyFileSink(dailyFile);
    RotatingFileSink rotatingFileSink = new RotatingFileSink(rotatingFile, 10, 10);
    ServerConsoleSink serverConsoleSink = new ServerConsoleSink();

    Logger logger = new Logger("multi-sink-logger");
    logger.AddSink(basicFileSink);
    logger.AddSink(dailyFileSink);
    logger.AddSink(rotatingFileSink);
    logger.AddSink(serverConsoleSink);

    logger.Info("Some message");
    logger.Warn("Some warning");

    basicFileSink.Close();
    dailyFileSink.Close();
    rotatingFileSink.Close();
    serverConsoleSink.Close();
    logger.Close();
}
```

文件 `game/addons/sourcemod/logs/log4sp-multi-basic-sink.log`：

> [2001-02-03 12:34:56.789] [multi-sink-logger] [info] Some message<br>
> [2001-02-03 12:34:56.789] [multi-sink-logger] [warn] Some warning<br>

文件 `game/addons/sourcemod/logs/log4sp-multi-daily-sink_20010203.log`：

> [2001-02-03 12:34:56.789] [multi-sink-logger] [info] Some message<br>
> [2001-02-03 12:34:56.789] [multi-sink-logger] [warn] Some warning<br>

文件 `game/addons/sourcemod/logs/log4sp-multi-rotating-sink.1.log`：

> [2001-02-03 12:34:56.789] [multi-sink-logger] [info] Some message<br>

文件 `game/addons/sourcemod/logs/log4sp-multi-rotating-sink.log`：

> [2001-02-03 12:34:56.789] [multi-sink-logger] [warn] Some warning<br>

控制台输出：

> [2001-02-03 12:34:56.789] [multi-sink-logger] [info] Some message<br>
> [2001-02-03 12:34:56.789] [multi-sink-logger] [warn] Some warning<br>

### 仅头文件

仅头文件的所有代码实现于 `.inc` 头文件中，不依赖任何外部组件，可以轻易的整合到任何插件中，从而简化设置于集成工作。

使用仅头文件模式需在包含 `<log4sp>` 头文件之前定义宏 `LOG4SP_HEADER_ONLY`，也可以添加编译参数启用：`LOG4SP_HEADER_ONLY=`

```sourcepawn
#include <sourcemod>

#define LOG4SP_HEADER_ONLY  // 启用仅头文件
#include <log4sp>

public void OnPluginStart()
{
    ServerConsoleSink sink = new ServerConsoleSink();

    Logger logger = new Logger("my-logger");
    logger.AddSink(sink);

    logger.Info("Hello World!");

    sink.Close();
    logger.Close();
}
```

仅头文件模式与扩展库模式主要区别如下：

|                             |     扩展库     |                           仅头文件                           |
| :-------------------------- | :------------: | :----------------------------------------------------------: |
| **LibraryExists("log4sp")** |      True      |                            False                             |
| **日志消息最大字符数**      |     无限制     |            宏 `LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH`            |
| **参数格式化错误处理**      | `ErrorHandler` |                   不支持<br>（抛出并中断）                   |
| **自定义模板**              |      支持      |                  不支持<br>（需要修改代码）                  |
| **克隆句柄**                |      支持      | 有限<br>（只能使用 methodmap 的 `Clone` 方法）<br>（创建者卸载后克隆体也会被破坏） |
| **关闭句柄**                |      支持      | 有限<br>（只能使用 methodmap 的 `Close` 方法）<br>（`delete` 、 `CloseHandle()` 都会泄漏句柄） |
| **要求**                    |    SM 1.12     |                           SM 1.11                            |

### 清理实例

拓展库模式可以直接使用 `delete` 关键字清理实例。

而仅头文件模式与 [sm-json](https://github.com/clugg/sm-json) 类似，底层使用 `StringMap`，您需要确保在实例使用完后妥善管理内存，清理它们。仅使用 `delete` 关键字清理实例是不够的，因为可能还嵌套了 ArrayList、PrivateForward、File 等 Handle。您需要使用对应 `methodmap` 的 `Close()` 方法来清理并删除所有 Handle。

此外，还有一些全局辅助函数 `LoggerCloseAndDelete()`，`LoggersCloseAndDelete()`， `SinkCloseAndDelete()`，`SinksCloseAndDelete()`，它会先调用对应 `methodmap` 的 `Close()` 方法，然后将传递的变量或数组元素设置为 `null`。

```sourcepawn
logger.Close();
logger = null;
// or
LoggerCloseAndDelete(logger);

sink.Close();
sink = null;
// or
SinkCloseAndDelete(sink);
```

如果一个共享 Handle 有多个引用，则只有在引用计数归零后这个共享对象才会真正清理。

对于 Logger Handle 而言，调用 `Logger.Clone()` 方法和注册到 `Registry` 会增加引用计数；调用 `Logger.Close()` 和从 `Registry` 注销会减少引用计数。

对于 Sink Handle 而言，调用 `Sink.Clone()` 方法和添加到 `Logger` 会增加引用计数；调用 `Sink.Close()`  和从 `Logger` 移除会减少引用计数。

## 架构

```mermaid
flowchart LR
 subgraph Sinks["`**Sink List**`"]
        SinkShouldJunction["Junction"]
        SinkShouldLog{"Should Log?"}
        SinkLog("Log")
        SinkPatternFormat["Pattern Format"]
        SinkFlush("Flush")
  end
 subgraph Logger["`**Logger**`"]
        LoggerShouldLog{"Should Log?"}
        LoggerShouldJunction["Junction"]
        LoggerLogJunction["Junction"]
        LoggerShouldFlush{"Should Flush?"}
        LoggerLog("Log")
        LoggerLogRaw["Raw Message"]
        LoggerLogF("LogF")
        LoggerLogFFormat["Params Format"]
  end
    Start((("`**Start**`"))) L_Start_LoggerShouldLog_0@== Log Message ==> LoggerShouldLog
    LoggerShouldLog -- Yes --- LoggerShouldJunction
    LoggerShouldJunction --> LoggerLogJunction & LoggerShouldFlush
    LoggerShouldLog -. No .-> Stop((("`**End**`")))
    LoggerLogJunction --- LoggerLog & LoggerLogF
    LoggerLog --- LoggerLogRaw
    LoggerLogRaw --- SinkShouldJunction
    LoggerLogF --- LoggerLogFFormat
    LoggerLogFFormat --- SinkShouldJunction
    LoggerShouldFlush -- Yes --- SinkFlush
    SinkFlush --> Stop
    LoggerShouldFlush -. No .-> Stop
    SinkShouldJunction --> SinkShouldLog
    SinkShouldLog -- Yes --- SinkPatternFormat
    SinkPatternFormat --- SinkLog
    SinkLog --> Stop
    SinkShouldLog -. No .-> Stop

    L_Start_LoggerShouldLog_0@{ animation: fast }
    LoggerShouldJunction@{ shape: junction}
    LoggerLogJunction@{ shape: junction}
    LoggerLogRaw@{ shape: das}
    LoggerLogFFormat@{ shape: das}
    SinkShouldJunction@{ shape: junction}
    SinkPatternFormat@{ shape: das}
    style Start stroke-width:4px,stroke-dasharray: 0,font-size:16px
    style Logger fill:transparent
    style LoggerShouldLog stroke-width:4px,stroke-dasharray: 0
    style LoggerShouldJunction fill:#00C853
    style LoggerLogJunction fill:#00C853
    style LoggerLog stroke-width:4px,stroke-dasharray: 0
    style LoggerLogRaw stroke-width:1px,stroke-dasharray:1
    style LoggerLogF stroke-width:4px,stroke-dasharray: 0
    style LoggerLogFFormat stroke-width:1px,stroke-dasharray:1
    style LoggerShouldFlush stroke-width:4px,stroke-dasharray: 0
    style Sinks fill:transparent
    style SinkShouldJunction fill:#00C853
    style SinkShouldLog stroke-width:4px,stroke-dasharray: 0
    style SinkPatternFormat stroke-width:1px,stroke-dasharray:1
    style SinkLog stroke-width:4px,stroke-dasharray: 0
    style SinkFlush stroke-width:4px,stroke-dasharray: 0
    style Stop stroke-width:3px,stroke-dasharray: 0
    linkStyle 1 stroke:#00C853,fill:none
    linkStyle 2 stroke:#00C853,fill:none
    linkStyle 3 stroke:#00C853,fill:none
    linkStyle 4 stroke:#D50000,fill:none
    linkStyle 5 stroke:#00C853,fill:none
    linkStyle 6 stroke:#00C853,fill:none
    linkStyle 7 stroke:#00C853,fill:none
    linkStyle 8 stroke:#00C853,fill:none
    linkStyle 9 stroke:#00C853,fill:none
    linkStyle 10 stroke:#00C853,fill:none
    linkStyle 11 stroke:#00C853,fill:none
    linkStyle 12 stroke:#00C853,fill:none
    linkStyle 13 stroke:#D50000,fill:none
    linkStyle 14 stroke:#00C853,fill:none
    linkStyle 15 stroke:#00C853,fill:none
    linkStyle 16 stroke:#00C853,fill:none
    linkStyle 17 stroke:#00C853,fill:none
    linkStyle 18 stroke:#D50000,fill:none
```

## 插件

### Manager

**log4sp_manager** 插件提供了多种管理方案用于管理各个插件的日志行为，只需要先将记录器注册到注册表即可。

然后就可以使用 Registry Natives，菜单或控制台指令来管理这些记录器。

- 注册记录器

    ```sourcepawn
    #include <sourcemod>
    #include <log4sp>
    #include <log4sp/registry>

    public void OnPluginStart()
    {
        if (Log4spRegistry.LibraryExists())
        {
            Logger logger = new Logger("my-logger");
            Log4spRegistry.Instance().RegisterLogger(logger);
            logger.Close();
        }
    }

    public void OnPluginEnd()
    {
        // Drop it when your plugin unloads.
        Log4spRegistry.Instance().Drop("my-logger");
    }
    ```

- 获取记录器

    ```sourcepawn
    #include <sourcemod>
    #include <log4sp>
    #include <log4sp/registry>

    public void OnPluginStart()
    {
        if (Log4spRegistry.LibraryExists())
        {
            Logger logger = Log4spRegistry.Instance().Get("my-logger");
            if (logger != INVALID_HANDLE)
            {
                logger.Info("Some message");
            }
        }
    }
    ```

- 全局记录器

    在插件加载时会创建一个空名 "" 的记录器作为全局记录器，全局记录器有一个 **ServerConsoleSink**，其余均为默认值，任何插件都可以直接使用。

    您也可以自行创建一个记录器并替换为全局记录器。

    ```sourcepawn
    #include <sourcemod>
    #include <log4sp>
    #include <log4sp/registry>

    public void OnPluginStart()
    {
        if (Log4spRegistry.LibraryExists())
        {
            Logger logger = new Logger("my-logger");
            Log4spRegistry.Instance().SetGlobalLogger(logger);
            logger.Close();
        }
    }
    ```

- 修改注册表中所有记录器属性

    ```sourcepawn
    #include <sourcemod>
    #include <log4sp>
    #include <log4sp/registry>

    public void OnPluginStart()
    {
        if (Log4spRegistry.LibraryExists())
        {
            // 修改注册表中所有记录器的日志级别为 Warn
            Log4spRegistry.Instance().SetLevel(LogLevel_Warn);

            // 修改注册表中所有记录器的自动刷写级别为 Error
            Log4spRegistry.Instance().SetFlushLevel(LogLevel_Error);
        }
    }
    ```

- 定期刷写注册表中所有记录器

    在 [刷写策略](#刷写策略) 章节中，定期刷写需要自行编写代码来实现，且方案不够优雅。

    如果借助 manager 插件则可以轻松实现类似的效果：

    ```sourcepawn
    #include <sourcemod>
    #include <log4sp/registry>

    public void OnPluginStart()
    {
        if (Log4spRegistry.LibraryExists())
        {
            // 每隔 5 秒刷写注册表中所有记录器
            Log4spRegistry.Instance().SetFlushEvery(5.0);
        }
    }
    ```

## 性能测试

测试平台：Windows 11 + VMware + Ubuntu 24.04 LTS + SourceMod 1.13.0.7410

主机配置：Intel(R) Core(TM) Ultra 7 255H + 32 GB 内存

VM Ubuntu 配置：8 vCPU（1 socket × 8 cores） + 8 GB 内存

测试用例：[./sourcemod/scripting/testsuite/bench/bench-log4sp.sp](./sourcemod/scripting/testsuite/bench/bench-log4sp.sp)

```shell
*****************************************************************************
* Bench log4sp v2.0.0  (release,git=d6a93cc,manual)                         *
*****************************************************************************
basic-file       Runs: 10   Calls: 10000000   Elapsed: 1.535      6511315/sec
callback         Runs: 10   Calls: 10000000   Elapsed: 3.064      3262996/sec
daily-file       Runs: 10   Calls: 10000000   Elapsed: 1.582      6318177/sec
ring-buffer      Runs: 10   Calls: 10000000   Elapsed: 0.750     13326972/sec
rotate-file      Runs: 10   Calls: 10000000   Elapsed: 1.379      7248320/sec
server-console   Runs: 10   Calls: 10000000   Elapsed: 22.818      438239/sec

*****************************************************************************
* Bench log4sp v2.0.0  (header-only,release,max-err=256,max-msg=1024)       *
*****************************************************************************
basic-file       Runs: 10   Calls: 10000000   Elapsed: 26.465      377856/sec
callback         Runs: 10   Calls: 10000000   Elapsed: 8.380      1193266/sec
daily-file       Runs: 10   Calls: 10000000   Elapsed: 28.817      347016/sec
ring-buffer      Runs: 10   Calls: 10000000   Elapsed: 299.795      33356/sec
rotate-file      Runs: 10   Calls: 10000000   Elapsed: 28.216      354407/sec
server-console   Runs: 10   Calls: 10000000   Elapsed: 48.795      204938/sec
```

作为参考, 还测试了 sourcemod 的 [logging API](https://sm.alliedmods.net/new-api/logging)

测试用例：[./sourcemod/scripting/testsuite/bench/bench-sm-logging.sp](./sourcemod/scripting/testsuite/bench/bench-sm-logging.sp)

```
[benchmark] LogMessage      Runs: 10   Calls: 10000000   Elapsed: 58.743    170230/sec
[benchmark] LogToFile       Runs: 10   Calls: 10000000   Elapsed: 54.598    183156/sec
[benchmark] LogToFileEx     Runs: 10   Calls: 10000000   Elapsed: 53.439    187128/sec
[benchmark] PrintToServer   Runs: 10   Calls: 10000000   Elapsed: 18.596    537744/sec
```

## 编译构建

### Linux

1. 下载依赖与项目文件

    ```bash
    mkdir alliedmodders && cd alliedmodders
    git clone https://github.com/alliedmodders/ambuild
    git clone --recursive https://github.com/alliedmodders/sourcemod -b 1.12-dev
    git clone https://github.com/F1F88/sm-ext-log4sp.git
    ```

2. 安装 Ambuild

    ```bash
    pip install ./ambuild
    ```

3. 编译构建

    ```bash
    mkdir sm-ext-log4sp/build && cd sm-ext-log4sp/build
    python3 ../configure.py --enable-optimize --sm-path ../../sourcemod --targets=x86,x64
    ambuild
    ```

    > [!TIP]
    > 本地构建能够启用[额外优化](./AMBuildScript#L233)

### Windows

1. 安装 Visual Studio、Python、Git（参考 [Building SourceMod](https://wiki.alliedmods.net/Building_sourcemod#Windows)）

2. 下载依赖与项目文件

    ```cmd
    mkdir alliedmodders && cd alliedmodders
    git clone https://github.com/alliedmodders/ambuild
    git clone --recursive https://github.com/alliedmodders/sourcemod -b 1.12-dev
    git clone https://github.com/F1F88/sm-ext-log4sp.git
    ```

3. 安装 AMBuild

    ```cmd
    pip install ./ambuild
    ```

4. 编译构建（在 Developer Command Prompt 环境中）

    ```cmd
    mkdir sm-ext-log4sp/build && cd sm-ext-log4sp/build
    python3 ../configure.py --enable-optimize --sm-path ../../sourcemod --targets=x86
    ambuild
    ```

## 常见问题

### AMBuild

error: externally-managed-environment

```shell
# ref: https://blog.csdn.net/2202_75762088/article/details/134625775
# Ubuntu 默认没有安装 pip
sudo apt install python3-pip
# 再次尝试安装 AMBuild
pip install ./ambuild --break-system-packages
```

### 扩展库

#### 编译问题

Linux 环境切换 clang/gcc 编译器

```shell
export CC=clang
export CXX=clang++
```

Unable to find a suitable CXX compiler

```shell
# ref: https://blog.csdn.net/weixin_38939826/article/details/105174347
sudo yum install gcc-c++ libstdc++-devel
```

gnu/stubs-32.h: No such file or directory

```shell
# ref: https://blog.csdn.net/wang_xijue/article/details/47128423
sudo yum install glibc-devel.i686
```

bits/c++config.h: No such file or directory

```shell
# ref: https://blog.csdn.net/Edidaughter/article/details/122627186
sudo apt-get install gcc-multilib g++-multilib
```

/usr/bin/ld: cannot find -lstdc++

> 删除 **`AMBuildScript`** 中的 [cxx.linkflags += ['-static-libstdc++']](./AMBuildScript#L306)

#### 运行问题

[SM] Unable to load extension "log4sp.ext": Could not find interface

> 检查 `扩展库` 版本是否与操作系统匹配
>
> 检查 `扩展库` 版本是否与 SourceMod 版本匹配

bin/libstdc++.so.6: version 'GLIBCXX_3.4.20' not found

> 详细方案请参考：[#6](https://github.com/F1F88/sm-ext-log4sp/issues/6)

### Plugins

#### 编译问题

error 139: could not find type "Logger"

> 检查是否缺少 `#include <log4sp>` 引入依赖
>
> 检查编译环境中是否缺少 Log4sp includes 文件
>
> 检查 Log4sp includes 文件版本是否过时

#### 运行问题

[SM] Unable to load plugin "....smx": Required extension "Log4sp" file("log4sp.ext") not running

> 检查 `"addons/sourcemod/extensions"` 文件夹内是否缺少 `log4sp.ext` 扩展库文件

执行 logger.Log(...) 后，日志文件没有数据

> 详细解答请参考：[\<spdlog wiki> FAQ](https://github.com/gabime/spdlog/wiki/FAQ#the-log-file-remains-empty)

## 特别感谢

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 项目实现了大部分功能，log4sp 将其包装到 sourceMod natives
- Fyren， nosoop， Deathreus 为管理 sink handle 提供了解决思路
- [blueblur0730](https://github.com/blueblur0730)， Digby 帮助改进了遍历操作所有 logger
- Bakugo， Anonymous Player， Fyren 帮助解决异步调用 sourcepawn 导致崩溃的问题
- [blueblur0730](https://github.com/blueblur0730) 添加了 log4sp_manager 插件

如有遗漏，请联系我
