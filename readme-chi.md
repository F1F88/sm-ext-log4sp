**[英语](./readme.md) | [中文](./readme-chi.md)**

# Logging for SourcePawn

这是一个包装了 [spdlog](https://github.com/gabime/spdlog) 库的 Sourcemod 拓展，用于增强 SourcePawn 记录日志和调试功能。

## 特点

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

## 文档

详细的使用文档可以在这里查看：[wiki](https://github.com/F1F88/sm-ext-log4sp/wiki)

## 支持的游戏

`log4sp.ext` 应该适用于 Linux 和 Windows 上的所有游戏

## 性能测试

测试平台: Windows 11 + VMware + Ubuntu 24.04 LTS + SourceMod 1.13.0.7178

宿主机配置: AMD Ryzen 7 7840HS + 32 GB 内存

VM Ubuntu 配置: 1 CPU + 8 核心 + 8 GB 内存

测试用例：[benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

#### 同步

```
[benchmark] base-file         | Iters 1000000 | Elapsed  0.582 secs   1717419/sec
[benchmark] daily-file        | Iters 1000000 | Elapsed  0.577 secs   1730828/sec
[benchmark] rotating-file     | Iters 1000000 | Elapsed  0.609 secs   1639532/sec
[benchmark] server-console    | Iters 1000000 | Elapsed  6.900 secs    144910/sec
```

#### 异步 Logger

```
# 队列大小：8192      线程数：1
[benchmark] base-file-async-block          | Iters 1000000 | Elapsed  0.753 secs   1326782/sec
[benchmark] daily-file-async-block         | Iters 1000000 | Elapsed  0.736 secs   1357952/sec
[benchmark] rotating-file-async-block      | Iters 1000000 | Elapsed  0.750 secs   1333285/sec
[benchmark] server-console-async-block     | Iters 1000000 | Elapsed  7.445 secs    134304/sec

[benchmark] base-file-async-overrun        | Iters 1000000 | Elapsed  0.761 secs   1312366/sec
[benchmark] daily-file-async-overrun       | Iters 1000000 | Elapsed  0.752 secs   1328488/sec
[benchmark] rotating-file-async-overrun    | Iters 1000000 | Elapsed  0.751 secs   1331146/sec
[benchmark] server-console-async-overrun   | Iters 1000000 | Elapsed  0.629 secs   1588385/sec


# 队列大小：8192      线程数：8
[benchmark] base-file-async-block          | Iters 1000000 | Elapsed  0.809 secs   1235689/sec
[benchmark] daily-file-async-block         | Iters 1000000 | Elapsed  0.849 secs   1176801/sec
[benchmark] rotating-file-async-block      | Iters 1000000 | Elapsed  0.898 secs   1113002/sec
[benchmark] server-console-async-block     | Iters 1000000 | Elapsed 12.028 secs     83135/sec

[benchmark] base-file-async-overrun        | Iters 1000000 | Elapsed  0.603 secs   1657736/sec
[benchmark] daily-file-async-overrun       | Iters 1000000 | Elapsed  0.843 secs   1185798/sec
[benchmark] rotating-file-async-overrun    | Iters 1000000 | Elapsed  0.594 secs   1681998/sec
[benchmark] server-console-async-overrun   | Iters 1000000 | Elapsed  0.662 secs   1508803/sec
```

#### Sourcemod logging

作为参考, 还测试了 sourcemod 的 [logging API](https://sm.alliedmods.net/new-api/logging)

测试用例：[benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)

```
[benchmark] LogMessage    | Iters 1000000 | Elapsed 12.041 secs     83043/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed 10.271 secs     97355/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed 10.113 secs     98880/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  6.408 secs    156054/sec
```

## 制作人员

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 库实现了绝大部分功能，此拓展将 spdlog API 包装后提供给 SourcePawn 使用

- Fyren, nosoop, Deathreus 为拓展管理 Sink Handle 提供了解决思路

如有遗漏，请联系我
