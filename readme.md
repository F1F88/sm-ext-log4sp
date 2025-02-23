**[English](./readme.md) | [中文](./readme-chi.md)**

# Log for SourcePawn

Log4sp is a powerful [SourceMod](https://www.sourcemod.net/about.php) extension that provides SourceMod plugin developers with a high-performance, feature-rich, and easy-to-use [logging API](./sourcemod/scripting/include/).

With log4sp, plugin developers no longer need to write complex logging code, so they can focus more on the core function development of the plugin.

## Features

1. Very fast, much faster than [SourceMod API - Logging](https://sm.alliedmods.net/new-api/logging)

   - spdlog [benchmarks](https://github.com/gabime/spdlog#benchmarks)

   - log4sp [benchmarks](https://github.com/F1F88/sm-ext-log4sp#benchmarks)

   - SourceMod API - Logging [benchmarks](https://github.com/F1F88/sm-ext-log4sp#sourcemod-logging)

2. Support custom log filtering.

   - For test, using lower log level (e.g., `trace`, `debug`) can increase log messages and help find problems.

   - For release, using higher log level (e.g., `warn`, `error`) can reduce log messages and thus improve performance.

3. Support custom log message pattern.

   - Custom information can be appended to log messages. (such as time, log level, source code location, etc.)

   - Specify the style of the log message and append more information (such as time, log level, source code location, etc.)

   - Default log message pattern:

      > [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#] %v

      > [2024-08-01 12:34:56:789] [log4sp] [info] [example.sp:123] message

   - All log message pattern flags: [spdlog wiki](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. Support custom log message flush level.

   - By default, the log4sp extension flush the log buffer when [it sees fit in order](https://github.com/gabime/spdlog/wiki/7.-Flush-policy) to achieve good performance.

   - `Logger.Flush()` can be manually flushed.

   - `Logger.FlushOn()` can set the minimum log level that will trigger automatic flush.

5. Support server console commands.

   - The server console command **"sm log4sp"** can dynamically modify the log level, flush level, log pattern, etc.

6. Support for "unlimited size" logging messages.

   - For `Logger.LogAmxTpl()` method, the maximum length of the log message is 2048 characters, and the excess will be truncated.

   - For `Logger.Log()` and `Logger.LogEx()` methods, the length of the log message is not limited (theoretically subject to available memory)

7. Supports logging to multiple sinks at once.

   - Each Logger can have multiple Sinks.

   - Each Sink can customize different log level and log pattern.

      For example: When the Logger has `ServerConsoleSink` and `DailyFileSink`, it is equivalent to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) when `sv_logecho 1`.

8. Various log targets

    - BasicFileSink  (Similar to [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile))

    - ClientChatAllSink (Similar to [PrintToChat](https://sm.alliedmods.net/new-api/halflife/PrintToChatAll))

    - ClientConsoleAllSink (Similar to [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsoleAll))

    - DailyFileSink (Similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage), Rotate log files based on date.)

    - RotatingFileSink (Similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage), Rotate log files based on file size.)

    - ServerConsoleSink (Similar to [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer))

    - For the full list of sinks please visit the [sinks folder](./sourcemod/scripting/include/log4sp/sinks/).

## Documentation

[Native API Reference](./sourcemod/scripting/include/log4sp/)

Documentation can be found in the [wiki pages](https://github.com/F1F88/sm-ext-log4sp/wiki).

## Supported Games

The `log4sp` extension should work for all games on Linux and Windows.

## Benchmarks

Test platform: Windows 11 + VMware + Ubuntu 24.04 LTS + SourceMod 1.13.0.7178

Host configuration: AMD Ryzen 7 7840HS + 32 GB Memory

VM Ubuntu configuration: 1 CPU  + 8 kernel  + 8 GB Memory

Test case: [benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

```
[benchmark] base-file         | Iters 1000000 | Elapsed  0.385 secs   2592735/sec
[benchmark] daily-file        | Iters 1000000 | Elapsed  0.393 secs   2541238/sec
[benchmark] rotating-file     | Iters 1000000 | Elapsed  0.406 secs   2462884/sec
[benchmark] server-console    | Iters 1000000 | Elapsed  5.224 secs    191411/sec
```

#### Sourcemod logging

As a reference, [sourcemod logging API](https://sm.alliedmods.net/new-api/logging) was also tested

Test case: [benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)


```
[benchmark] LogMessage    | Iters 1000000 | Elapsed  8.862 secs    112829/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed  7.392 secs    135267/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed  7.284 secs    137272/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  5.419 secs    184534/sec
```

## Credits

- **[gabime](https://github.com/gabime) [spdlog](https://github.com/gabime/spdlog)** library implements most of the functionality, log4sp.ext wraps the spdlog API for SourcePawn to use

- Fyren, nosoop, Deathreus provides solution for managing the Sink Handle

- [blueblur0730](https://github.com/blueblur0730), Digby helped improve the traversal operation of all loggers

- Bakugo, Anonymous Player, Fyren Help Fix Crash with Asynchronous Calls to SourcePawn

If I missed anyone, please contact me.

