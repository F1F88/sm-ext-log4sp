**[English](./readme.md) | [中文](./readme-chi.md)**

# Logging for SourcePawn

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

5. 支持 "回溯" 日志消息

   - 将 `trace` 和 `debug` 日志消息存储在环形缓冲区中，后续按需输出

6. 支持服务器控制台菜单

   - 服务器控制台指令 **"sm log4sp"** 可以动态的修改 日志级别、刷新级别、日志模板、"回溯" 等

7. 支持异步记录日志消息

   - 不会阻塞服务器的主线程

8. 支持 "无限长度" 的日志消息

   - 对于 `Logger.LogAmxTpl()` 方法，日志消息的最大长度为 2048 字符，超出的部分会被截断

   - 对于 `Logger.Log()` 和 `Logger.LogEx()`方法，日志消息的长度不受限制（理论上取决于可用内存）

9. 支持一次日志操作写入多个输出源

   - 每一个记录器 (Logger) 都可以拥有多个输出源 (Sink)

   - 每一个输出源 (Sink) 都可以自定义不同的日志级别、日志模板

      例如：当 Logger 拥有 `ServerConsoleSink` 和 `DailyFileSink` 时，相当于 `sv_logecho 1` 时的 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

10. 支持多种输出源

    - BaseFileSink （类似于 [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile)）

    - ClientChatSink（类似于 [PrintToChat](https://sm.alliedmods.net/new-api/halflife/PrintToChat)）

    - ClientConsoleSink（类似于 [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole)）

    - DailyFileSink（类似于 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)，基于日期更替日志文件）

    - RotatingFileSink（类似于 [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)，基于文件大小更替日志文件）

    - ServerConsoleSink（类似于 [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer)）

## 文档

详细的使用文档可以在这里查看：[wiki pages](https://github.com/F1F88/sm-ext-log4sp/wiki)

## 支持的游戏

`log4sp` 拓展适用于 Linux 和 Windows 上的所有游戏

## 性能测试

测试平台: Windows 11 + VMware + Ubuntu 24.04 LTS + SourceMod 1.13.0.7178

宿主机配置: AMD Ryzen 7 7840HS + 32 GB 内存

VM Ubuntu 配置: 1 CPU + 8 核心 + 8 GB 内存

测试用例：[benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

#### 同步

```
[benchmark] base-file         | Iters 1000000 | Elapsed  0.465 secs   2150463/sec
[benchmark] daily-file        | Iters 1000000 | Elapsed  0.471 secs   2118729/sec
[benchmark] rotating-file     | Iters 1000000 | Elapsed  0.482 secs   2073553/sec
[benchmark] server-console    | Iters 1000000 | Elapsed  4.847 secs    206288/sec
```

#### 异步 Logger

```
# 队列大小：8192      线程数：1
[benchmark] base-file-async-block          | Iters 1000000 | Elapsed  0.532 secs   1878922/sec
[benchmark] daily-file-async-block         | Iters 1000000 | Elapsed  0.530 secs   1883991/sec
[benchmark] rotating-file-async-block      | Iters 1000000 | Elapsed  0.527 secs   1895788/sec
[benchmark] server-console-async-block     | Iters 1000000 | Elapsed  6.091 secs    164162/sec

[benchmark] base-file-async-overrun        | Iters 1000000 | Elapsed  0.530 secs   1883977/sec
[benchmark] daily-file-async-overrun       | Iters 1000000 | Elapsed  0.528 secs   1893666/sec
[benchmark] rotating-file-async-overrun    | Iters 1000000 | Elapsed  0.511 secs   1956709/sec
[benchmark] server-console-async-overrun   | Iters 1000000 | Elapsed  0.407 secs   2455367/sec


# 队列大小：8192      线程数：8
[benchmark] base-file-async-block          | Iters 1000000 | Elapsed  0.688 secs   1452901/sec
[benchmark] daily-file-async-block         | Iters 1000000 | Elapsed  0.671 secs   1488398/sec
[benchmark] rotating-file-async-block      | Iters 1000000 | Elapsed  0.715 secs   1397846/sec
[benchmark] server-console-async-block     | Iters 1000000 | Elapsed 11.645 secs     85873/sec

[benchmark] base-file-async-overrun        | Iters 1000000 | Elapsed  0.524 secs   1905625/sec
[benchmark] daily-file-async-overrun       | Iters 1000000 | Elapsed  0.494 secs   2022167/sec
[benchmark] rotating-file-async-overrun    | Iters 1000000 | Elapsed  0.533 secs   1872676/sec
[benchmark] server-console-async-overrun   | Iters 1000000 | Elapsed  0.509 secs   1964281/sec
```

#### Sourcemod logging

作为参考, 还测试了 sourcemod 的 [logging API](https://sm.alliedmods.net/new-api/logging)

测试用例：[benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)

```
[benchmark] LogMessage    | Iters 1000000 | Elapsed  9.657 secs    103548/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed  8.070 secs    123903/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed  7.959 secs    125637/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  4.718 secs    211920/sec
```

## 制作人员

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 库实现了绝大部分功能，此拓展将 spdlog API 包装后提供给 SourcePawn 使用

- Fyren, nosoop, Deathreus 为拓展管理 Sink Handle 提供了解决思路

如有遗漏，请联系我
