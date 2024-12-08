**[English](./readme.md) | [中文](./readme-chi.md)**

# Logging for SourcePawn

This is a Sourcemod extension that wraps the [spdlog](https://github.com/gabime/spdlog) library to enhance SourcePawn logging and debugging.

## Features

1. Very fast, much faster than [SourceMod API - Logging](https://sm.alliedmods.net/new-api/logging)

   - spdlog [benchmarks](https://github.com/gabime/spdlog#benchmarks)

   - log4sp [benchmarks](https://github.com/F1F88/sm-ext-log4sp#benchmarks)

   - SourceMod API - Logging [benchmarks](https://github.com/F1F88/sm-ext-log4sp#sourcemod-logging)

2. Support custom log filtering.

   - For test environments, you can use low log level (such as `trace`, `debug`) to increase log message and find bugs.

   - For online environments, you can use high log level (such as: `warn`, `error`) to reduce log message and improve performance.

3. Support custom log message pattern.

   - Custom information can be appended to log messages. (such as time, log level, source code location, etc.)

   - Default log message pattern is:

      > [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#] %v

      > [2024-08-01 12:34:56:789] [log4sp] [info] [example.sp:123] message

   - All pattern flags see: [spdlog wiki](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. Support custom log message flush level.

   - By default spdlog lets the underlying libc flush whenever it sees fit in order to [achieve good performance](https://github.com/gabime/spdlog/wiki/7.-Flush-policy).

   - You can use `Logger.Flush()` to manually flush, or use `Logger.FlushOn()` set the minimum log level that will trigger automatic flush.

5. Support "backtrace" log messages.

   - store debug messages in a ring buffer and display them later on demand.

6. Support server console commands.

   - The server console command "sm log4sp" can dynamically modify the log level, flush level, log pattern, backtrace, etc.

7. Support for asynchronous logging messages.

   - Does not block the server's main thread.

8. Support for "infinite length" logging messages.

   - For logging methods called `Logger.***AmxTpl()`, the maximum length of the log message is 2048 characters, and any excess will be truncated.

   - For logging methods other than `Logger.***AmxTpl()`, the maximum length of the log message is unlimited. (theoretically, it depends on available memory)

9. Supports logging to multiple sinks at once.

   - Each Logger can have multiple Sinks.

   - Each Sink can customize different log level and log pattern.

      For example: When the Logger has `ServerConsoleSink` and `DailyFileSink`, it is equivalent to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) when `sv_Logecho 1`.

10. Various log targets

    - BaseFileSink （Similar to [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile)）

    - ClientChatSink（Similar to [PrintToChat](https://sm.alliedmods.net/new-api/halflife/PrintToChat)）

    - ClientConsoleSink（Similar to [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole)）

    - DailyFileSink（Similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)）

      Rotate log files based on date.

    - RotatingFileSink（Similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)）

      Rotate log files based on file size.

    - ServerConsoleSink（Similar to [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer)）

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

