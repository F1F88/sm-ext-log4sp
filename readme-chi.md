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

### 文档

详细的使用文档可以在这里查看：[wiki](https://github.com/F1F88/sm-ext-log4sp/wiki)

### 支持的游戏

正常来说，`log4sp.ext` 应该适用于 Linux 和 Windows 上的所有游戏

但目前，在 Windows 系统使用前需要自行编译这个项目

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

### 制作人员

- **[gabime](https://github.com/gabime)** 的 **[spdlog](https://github.com/gabime/spdlog)** 库实现了绝大部分功能，此拓展将 spdlog API 包装后提供给 SourcePawn 使用

- Fyren, nosoop, Deathreus 为拓展管理 Sink Handle 提供了解决思路

如有遗漏，请联系我
