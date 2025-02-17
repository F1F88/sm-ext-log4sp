**[English](./readme.md) | [中文](./readme-chi.md)**

# Log for SourcePawn

Log4sp 是一个强大的 [SourceMod](https://www.sourcemod.net/about.php) 拓展，为 SourceMod 插件开发者提供了一套高性能、功能丰富、简单易用的[记录日志 API](./sourcemod/scripting/include/)。

借助 log4sp，插件开发者无需再编写复杂的日志记录代码，从而更专注于插件的核心功能开发。

## 特点

1. 非常快，比 [SourceMod API - Logging](https://sm.alliedmods.net/new-api/logging) 快得多

   - spdlog [性能测试](https://github.com/gabime/spdlog#benchmarks)

   - log4sp [性能测试](https://github.com/F1F88/sm-ext-log4sp/blob/main/readme-chi.md#%E6%80%A7%E8%83%BD%E6%B5%8B%E8%AF%95)

   - SourceMod API - Logging [性能测试](https://github.com/F1F88/sm-ext-log4sp/blob/main/readme-chi.md#sourcemod-logging)

2. 支持自定义日志过滤

   - 对于测试环境，使用较低的日志级别（如：`trace`、`debug`）可以增加日志输出，从而排查问题

   - 对于发布环境，使用较高的日志级别（如：`warn`、`error`）可以减少日志输出，从而提高性能

3. 支持自定义日志消息模板

   - 指定日志消息的样式，附加其他信息（如：时间、日志级别、源代码位置 等）

   - 默认日志模板：

      > [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#] %v
      >
      > [2024-08-01 12:34:56:789] [log4sp] [info] [example.sp:123] message

   - 日志模板占位符：[spdlog wiki](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. 支持自定义日志消息刷新级别

   - 默认情况下，log4sp 拓展会在[认为合适时](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)刷新日志缓冲区，以实现更高的性能

   - `Logger.Flush()` 可以手动刷新

   - `Logger.FlushOn()` 可以自定义触发自动刷新的最低日志级别

5. 支持服务器控制台菜单

   - 控制台指令 **"sm log4sp"** 能够动态修改：日志级别、刷新级别、日志模板、等

6. 支持 "无限长度" 的日志消息

   - 对于 `Logger.LogAmxTpl()` 方法，日志消息的最大长度为 2048 字符，超出的部分会被截断

   - 对于 `Logger.Log()` 和 `Logger.LogEx()`方法，日志消息的长度不受限制（理论上取决于可用内存）

7. 支持一次日志操作写入多个输出源

   - 每一个记录器 (Logger) 都可以拥有多个输出源 (Sink)

   - 每一个输出源 (Sink) 都可以自定义不同的日志级别、日志模板

      例如：当 Logger 拥有 `ServerConsoleSink` 和 `DailyFileSink` 时，相当于 `sv_logecho 1` 时的 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

8. 支持多种输出源

    - BasicFileSink （类似于 [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile)）

    - ClientChatAllSink（类似于 [PrintToChatAll](https://sm.alliedmods.net/new-api/halflife/PrintToChatAll)）

    - ClientConsoleAllSink（类似于 [PrintToConsoleAll](https://sm.alliedmods.net/new-api/console/PrintToConsoleAll)）

    - DailyFileSink（类似于 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)，基于日期更替日志文件）

    - RotatingFileSink（类似于 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)，基于文件大小更替日志文件）

    - ServerConsoleSink（类似于 [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer)）

    - 完整的 Sink 列表请查看：[sinks 文件夹](./sourcemod/scripting/include/log4sp/sinks/)

## 文档

API 文档：[Native API Reference](./sourcemod/scripting/include/log4sp/)

使用文档：[Wiki Pages](https://github.com/F1F88/sm-ext-log4sp/wiki)

## 支持的游戏

`log4sp` 拓展适用于 Linux 和 Windows 上的所有游戏

## 性能测试

测试平台: Windows 11 + VMware + Ubuntu 24.04 LTS + SourceMod 1.13.0.7178

宿主机配置: AMD Ryzen 7 7840HS + 32 GB 内存

VM Ubuntu 配置: 1 CPU + 8 核心 + 8 GB 内存

测试用例：[benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

```
[benchmark] base-file         | Iters 1000000 | Elapsed  0.385 secs   2592735/sec
[benchmark] daily-file        | Iters 1000000 | Elapsed  0.393 secs   2541238/sec
[benchmark] rotating-file     | Iters 1000000 | Elapsed  0.406 secs   2462884/sec
[benchmark] server-console    | Iters 1000000 | Elapsed  5.224 secs    191411/sec
```

#### Sourcemod logging

作为参考, 还测试了 sourcemod 的 [logging API](https://sm.alliedmods.net/new-api/logging)

测试用例：[benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)

```
[benchmark] LogMessage    | Iters 1000000 | Elapsed  8.862 secs    112829/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed  7.392 secs    135267/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed  7.284 secs    137272/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  5.419 secs    184534/sec
```

## 制作人员

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 库实现了绝大部分功能，此拓展将 spdlog API 包装后提供给 SourcePawn 使用

- Fyren, nosoop, Deathreus 为拓展管理 Sink Handle 提供了解决思路

- [blueblur0730](https://github.com/blueblur0730), Digby 帮助改进了遍历操作所有 logger

- Bakugo, Anonymous Player, Fyren 帮助解决异步调用 SourcePawn 导致崩溃的问题

如有遗漏，请联系我
