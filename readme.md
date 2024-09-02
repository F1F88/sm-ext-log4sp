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

In general, `Log4sp.ext` should work with all games on both Linux and Windows.

But for now, you need to compile the project yourself before using it on Windows.

## Benchmarks

Test platform: Windows 11 + VMware + Ubuntu 24.04 LTS + sourcemod 1.11

Host configuration: AMD Ryzen 7 6800H + 32 GB Memory

VMware configuration: 1 CPU  + 8 kernel  + 4 GB Memory

Test case 1: [benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

Test case 2: [benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)

#### Single thread (Synchronous)

```
[benchmark] base-file-st      | Iters 1000000 | Elapsed  0.268 secs   3719518/sec
[benchmark] daily-file-st     | Iters 1000000 | Elapsed  0.278 secs   3589439/sec
[benchmark] rotating-file-st  | Iters 1000000 | Elapsed  0.279 secs   3578598/sec
[benchmark] server-console-st | Iters 1000000 | Elapsed  5.609 secs    178255/sec
```

#### Multi thread (Asynchronous)

```
# Queue size: 8192      Thread count: 1
[benchmark] base-file-block        | Iters 1000000 | Elapsed  0.479 secs   2084762/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed  0.488 secs   2046592/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed  0.462 secs   2162868/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed  8.422 secs    118725/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed  0.442 secs   2259856/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed  0.438 secs   2280891/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed  0.442 secs   2260684/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed  0.379 secs   2632167/sec


# Queue size: 8192      Thread count: 4
[benchmark] base-file-block        | Iters 1000000 | Elapsed  1.049 secs    952753/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed  1.086 secs    920584/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed  1.034 secs    967049/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed 15.784 secs     63354/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed  0.439 secs   2273952/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed  0.451 secs   2212609/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed  0.453 secs   2204658/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed  0.372 secs   2684282/sec


# Queue size: 131072    Thread count: 4
[benchmark] base-file-block        | Iters 1000000 | Elapsed   0.998 secs   1001216/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed   0.973 secs   1027070/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed   0.956 secs   1045255/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed  13.952 secs     71671/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed   0.472 secs   2116635/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed   0.441 secs   2264892/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed   0.478 secs   2091503/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed   0.385 secs   2592245/sec


# Queue size: 8192      Thread count: 8
[benchmark] base-file-block        | Iters 1000000 | Elapsed  1.135 secs    881010/sec
[benchmark] daily-file-block       | Iters 1000000 | Elapsed  1.183 secs    845069/sec
[benchmark] rotating-file-block    | Iters 1000000 | Elapsed  1.193 secs    838199/sec
[benchmark] server-console-block   | Iters 1000000 | Elapsed 14.925 secs     67000/sec

[benchmark] base-file-overrun      | Iters 1000000 | Elapsed  0.533 secs   1875363/sec
[benchmark] daily-file-overrun     | Iters 1000000 | Elapsed  0.569 secs   1754767/sec
[benchmark] rotating-file-overrun  | Iters 1000000 | Elapsed  0.508 secs   1967969/sec
[benchmark] server-console-overrun | Iters 1000000 | Elapsed  0.394 secs   2532556/sec
```

#### Sourcemod logging

As a reference, [sourcemod logging API](https://sm.alliedmods.net/new-api/logging) was also tested


```
[benchmark] LogMessage    | Iters 1000000 | Elapsed 10.740 secs     93108/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed  9.091 secs    109989/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed  8.823 secs    113336/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  5.779 secs    173024/sec
```

## Credits

- **[gabime](https://github.com/gabime) [spdlog](https://github.com/gabime/spdlog)** library implements most of the functionality, log4sp.ext wraps the spdlog API for SourcePawn to use

- Fyren, nosoop, Deathreus provides solution for managing the Sink Handle

If I missed anyone, please contact me.

