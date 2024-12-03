**[English](./readme.md) | [Chinese](./readme-chi.md)**

# Logging for SourcePawn

This is a Sourcemod extension that wraps the [spdlog](https://github.com/gabime/spdlog) library to enhance SourcePawn logging and debugging.

## Features

1. Very fast, much faster than [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - [spdlog benchmarks](https://github.com/gabime/spdlog#benchmarks)  |  [log4sp benchmarks](https://github.com/F1F88/sm-ext-log4sp#benchmarks)

2. Each `Logger` and `Sink` can customize the log level

3. Each `Logger` and `Sink` can customize the [log message pattern](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. Each `Logger` can customize the [flush policy](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)

5. Each `Logger` can have multiple `Sink`

   - For example: A `Logger` that has both `ServerConsoleSink` and `DailyFileSink` is similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

6. Each `Logger` can dynamic change the log level and pattern

   - see server command `"sm log4sp"`

7. Supports asynchronous `Logger`

8. Supports format parameters with variable numbers

   - [Parameter formatting](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) usage is consistent with [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - The maximum length of a variable parameter string is **2048** characters
     If characters exceeding this length will be truncated
     If longer message need to be log, non `AmxTpl` API can be used, e.g. `void Info(const char [] msg)`

9. Supports [backtrace](https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support)

   - When enabled, `Trace` and `Debug` level log message are stored in a circular buffer and only output explicitly after calling `DumpBacktrace()`

10. Supports various log targets

    - ServerConsoleSink (Similar to [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer))

    - ClientConsoleSink (Similar to [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole))

    - BaseFileSink  (Similar to [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile) when [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) is 0)

    - DailyFileSink (Similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) when [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) is 0)

    - RotatingFileSink

## Documentation

Documentation can be found in the [wiki](https://github.com/F1F88/sm-ext-log4sp/wiki) pages.

## Supported Games

`log4sp.ext` should work for all games on Linux and Windows.

## Benchmarks

Test platform: Windows 11 + VMware + Ubuntu 24.04 LTS + SourceMod 1.13.0.7178

Host configuration: AMD Ryzen 7 7840HS + 32 GB Memory

VM Ubuntu configuration: 1 CPU  + 8 kernel  + 8 GB Memory

Test case: [benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

#### Synchronous

```
[benchmark] base-file         | Iters 1000000 | Elapsed  0.582 secs   1717419/sec
[benchmark] daily-file        | Iters 1000000 | Elapsed  0.577 secs   1730828/sec
[benchmark] rotating-file     | Iters 1000000 | Elapsed  0.609 secs   1639532/sec
[benchmark] server-console    | Iters 1000000 | Elapsed  6.900 secs    144910/sec
```

#### Asynchronous

```
# Queue size: 8192      Thread count: 1
[benchmark] base-file-async-block          | Iters 1000000 | Elapsed  0.753 secs   1326782/sec
[benchmark] daily-file-async-block         | Iters 1000000 | Elapsed  0.736 secs   1357952/sec
[benchmark] rotating-file-async-block      | Iters 1000000 | Elapsed  0.750 secs   1333285/sec
[benchmark] server-console-async-block     | Iters 1000000 | Elapsed  7.445 secs    134304/sec

[benchmark] base-file-async-overrun        | Iters 1000000 | Elapsed  0.761 secs   1312366/sec
[benchmark] daily-file-async-overrun       | Iters 1000000 | Elapsed  0.752 secs   1328488/sec
[benchmark] rotating-file-async-overrun    | Iters 1000000 | Elapsed  0.751 secs   1331146/sec
[benchmark] server-console-async-overrun   | Iters 1000000 | Elapsed  0.629 secs   1588385/sec


# Queue size: 8192      Thread count: 8
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

As a reference, [sourcemod logging API](https://sm.alliedmods.net/new-api/logging) was also tested

Test case: [benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)


```
[benchmark] LogMessage    | Iters 1000000 | Elapsed 12.041 secs     83043/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed 10.271 secs     97355/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed 10.113 secs     98880/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  6.408 secs    156054/sec
```

## Credits

- **[gabime](https://github.com/gabime) [spdlog](https://github.com/gabime/spdlog)** library implements most of the functionality, log4sp.ext wraps the spdlog API for SourcePawn to use

- Fyren, nosoop, Deathreus provides solution for managing the Sink Handle

If I missed anyone, please contact me.

