**[English](./readme.md) | [Chinese](./readme-chi.md)**

## Logging for SourcePawn

This is a Sourcemod extension that wraps the [spdlog](https://github.com/gabime/spdlog) library to enhance SourcePawn logging and debugging.

### Useage

1. Download the latest Zip from [Github Action](https://github.com/F1F88/sm-ext-log4sp/actions) that matches your operating system and sourcemod version
1. Upload the `addons/sourcemod/extension/log4sp.ext.XXX` in the ZIP to the `game/addons/sourcemod/extension` folder on the server

### Features

1. Very fast, much faster than [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - [spdlog benchmarks](https://github.com/gabime/spdlog#benchmarks)  |  [log4sp benchmarks](https://github.com/F1F88/sm-ext-log4sp#benchmarks)

2. Each `Logger` and `Sink` can customize the log level

3. Each `Logger` and `Sink` can customize the [log pattern](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. Each `Logger` and `Sink` can customize the [flush policy](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)

5. Each `Logger` can have multiple `Sink`

   - For example: A `Logger` that has both `ServerConsoleSink` and `DailyFileSink` is similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

6. Each `Logger` can dynamic change the log level and pattern

   - see server command `"sm log4sp"`

7. Supports asynchronous `Logger`

8. Supports format parameters with variable numbers

   - [Parameter formatting](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) usage is consistent with [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - The maximum length of a variable parameter string is **2048** characters

     If characters exceeding this length will be truncated
     If longer log messages need to be log, non `AmxTpl` methods can be used, e.g. `void Info(const char [] msg)`

9. Supports [backtrace](https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support)

   - When enabled, Trace and Debug level log message are stored in a circular buffer and only output explicitly after calling `DumpBacktrace()`

10. Supports various log targets

   - ServerConsoleSink  (Similar to [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer))

   - ClientConsoleSink  (Similar to [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole))

   - BaseFileSink  (Similar to [LogToFile](https://sm.alliedmods.net/new-api/logging/LogToFile) when [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) is 0)

   - DailyFileSink  (Similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage) when [sv_logecho](https://forums.alliedmods.net/showthread.php?t=170556#sv_logecho) is 0)

   - RotatingFileSink

### Usage Examples

##### Simple Example

```sourcepawn
#include <sourcemod>
#include <sdktools>
#include <log4sp>

Logger myLogger;

public void OnPluginStart()
{
    // Default LogLevel: LogLevel_Info
    // Default Pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
    myLogger = Logger.CreateServerConsoleLogger("logger-example-1");

    RegConsoleCmd("sm_log4sp_example1", CommandCallback);

    // Debug is lower than the default LogLevel, so this line of code won't output log message
    myLogger.Debug("===== Example 1 code initialization is complete! =====");
}

/**
 * Get client aiming entity info.
 */
Action CommandCallback(int client, int args)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client))
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [info] Command is in-game only.
        myLogger.Info("Command is in-game only.");
        return Plugin_Handled;
    }

    int entity = GetClientAimTarget(client, false);
    if (entity == -2)
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [fatal] The GetClientAimTarget() function is not supported.
        myLogger.Fatal("The GetClientAimTarget() function is not supported.");
        return Plugin_Handled;
    }

    if (entity == -1)
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [warn] client (1) is not aiming at entity.
        myLogger.WarnAmxTpl("client (%d) is not aiming at entity.", client);
        return Plugin_Handled;
    }

    if (!IsValidEntity(entity))
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [error] entity (444) is invalid.
        myLogger.ErrorAmxTpl("entity (%d) is invalid.", entity);
        return Plugin_Handled;
    }

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));

    // [2024-08-01 12:34:56.789] [logger-example-1] [info] client (1) is aiming a (403 - prop_door_breakable) entity.
    myLogger.InfoAmxTpl("client (%d) is aiming a (%d - %s) entity.", client, entity, classname);

    return Plugin_Handled;
}
```

##### More Detailed Example

```sourcepawn
#include <sourcemod>
#include <log4sp>

Logger myLogger;

public void OnPluginStart()
{
    // Create a single-threaded Logger object named "myLogger"
    myLogger = Logger.CreateServerConsoleLogger("logger-example-2");

    // Change the level of myLogger. Default is LogLevel_Info
    myLogger.SetLevel(LogLevel_Debug);

    // Change the pattern of myLogger. Default is "[2014-10-31 23:46:59.678] [name] [info] message"
    myLogger.SetPattern("[Date: %Y/%m/%d %H:%M:%S] [Name: %n] [LogLevel: %l] [message: %v]");

    /**
     * Each manually created sink is registered to the sink register within the extension
     * meaning that the number of references must be at least 1
     */
    // Create a Sink object for output to the client console
    ClientConsoleSinkST mySink = new ClientConsoleSinkST();

    // Change the level of mySink. Default is LogLevel_Trace
    mySink.SetLevel(LogLevel_Info);

    // Set mySink filter. Default output to all client in the game
    mySink.SetFilter(FilterAlivePlayer);

    // Add mySink to Logger myLogger
    myLogger.AddSink(mySink);

    /**
     * Now myLogger has 2 Sinks
     * One is ServerConsoleSink added when "Logger.CreateServerConsoleLogger()"
     * One is ClientConsoleSink added when "myLogger.AddSink()"
     *
     * Now mySink has 2 references
     * One is the reference registered in SinkRegister when using new "ClientConsoleSinkST()"
     * One is myLogger references mySink when using "myLogger.AddSink()"
     */

    // If you do not need to modify mySink any more, you can 'delete' it
    delete mySink;

    /**
     * This will not cause INVALID_HANDLE, as myLogger also references this ClientConsoleSink
     *
     * "delete mySink;" actually just removes the reference to mySink from the SinkRegister
     * So the number of references to mySink object will be decreases 1
     * But only when the number of references decreases to 0, the mySink object will be truly 'delete'
     *
     * In this example is   "delete mySink;"    +   "myLogger.DropSink(mySink);"
     * or                   "delete mySink;"    +   "delete myLogger;"
     */

    RegConsoleCmd("sm_log4sp_example2", CommandCallback);

    myLogger.Info("===== Example 2 code initialization is complete! =====");
}

/**
 * ClientConsoleSink filter
 * Used to filter out dead players, we only output log messages to live players
 *
 * Only when (IsClientInGame(client) && !IsFakeClient(client)) == true, the extension will call this filter
 * So there is no need to repeat IsClientInGame(client) or IsFakeClient(client) within this filter
 */
Action FilterAlivePlayer(int client)
{
    return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
}

Action CommandCallback(int client, int args)
{
    static int count = 0;
    ++count;

    /**
     * Due to the modification of the level in "mySink.SetLevel(LogLevel_Info);"
     * So this message will only be output to the server console, not to the client console
     */
    myLogger.DebugAmxTpl("Command 'sm_log4sp_example2' was called (%d) times.", count);

    /**
     * Due to the modified level of "myLogger.SetLevel(LogLevel_Debug);" being greater than LogLevel_Trace
     * So this message will not be output anywhere
     */
    myLogger.TraceAmxTpl("CommandCallback params: client (%d), args (%d)", client, args);

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

##### Additional Examples

- [./sourcemod/scripting/log4sp-test.sp](./sourcemod/scripting/log4sp-test.sp)

- [./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)

### Benchmarks

Test platform: Windows 11 23H2 + VMware + Ubuntu 24.04 LTS + NMRIH Dedicated Server v1.13.6 + SM 1.11

Host configuration: AMD Ryzen 7 6800H + 32GB Memory

VMware configuration: 1 CPU  + 8 kernel  + 4GB Memory

#### Single thread (Synchronous)

[./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)

```
sm_log4sp_bench_files_st
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext API Single thread, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] base-file-st             Elapsed:  0.28 secs      3524341 /sec
[log4sp-benchmark] rotating-file-st         Elapsed:  0.29 secs      3390347 /sec
[log4sp-benchmark] daily-file-st            Elapsed:  0.29 secs      3385034 /sec

sm_log4sp_bench_server_console_st
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext Server Console Single thread, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] server-console-st        Elapsed:  5.60 secs       178455 /sec
```

#### Multi thread (Asynchronous)

[./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)

```
sm_log4sp_bench_files_async
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext API Asynchronous mode, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: block
[log4sp-benchmark] *********************************
[log4sp-benchmark] base-file-block          Elapsed:  0.46 secs      2164179 /sec
[log4sp-benchmark] rotating-file-block      Elapsed:  0.48 secs      2071195 /sec
[log4sp-benchmark] daily-file-block         Elapsed:  0.46 secs      2131532 /sec
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: overrun
[log4sp-benchmark] *********************************
[log4sp-benchmark] base-file-overrun        Elapsed:  0.43 secs      2288062 /sec
[log4sp-benchmark] rotating-file-overrun    Elapsed:  0.43 secs      2310306 /sec
[log4sp-benchmark] daily-file-overrun       Elapsed:  0.34 secs      2876704 /sec

sm_log4sp_bench_server_console_async
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Log4sp.ext Server Console Asynchronous mode, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: block
[log4sp-benchmark] *********************************
[log4sp-benchmark] server-console-block     Elapsed:  8.05 secs       124192 /sec
[log4sp-benchmark]
[log4sp-benchmark] *********************************
[log4sp-benchmark] Queue Overflow Policy: overrun
[log4sp-benchmark] *********************************
[log4sp-benchmark] server-console-overrun   Elapsed:  8.19 secs       121953 /sec
```

#### Sourcemod logging API

As a reference, it is also used [./sourcemod/scripting/sm-logging-benchmark.sp](./sourcemod/scripting/sm-logging-benchmark.sp) tested the Sourcemod [logging API](https://sm.alliedmods.net/new-api/logging)


```
sm_log4sp_bench_sm_logging
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Sourcemod Logging API, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] LogMessage               Elapsed: 10.99 secs        90979 /sec
[log4sp-benchmark] LogToFile                Elapsed:  8.91 secs       112111 /sec
[log4sp-benchmark] LogToFileEx              Elapsed:  9.07 secs       110141 /sec

sm_log4sp_bench_sm_console
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] Sourcemod Console API, 1000000 iterations
[log4sp-benchmark] **************************************************************
[log4sp-benchmark] PrintToServer            Elapsed:  5.86 secs       170446 /sec
```

### Dependencies

- [Sourcemod](https://github.com/alliedmodders/sourcemod/tree/1.11-dev)
- [spdlog](https://github.com/gabime/spdlog)

  Only requires [include files](https://github.com/gabime/spdlog/tree/v1.x/include/spdlog), already included in [./extern/spdlog/include/spdlog](./extern/spdlog/include/spdlog)

### Basic Build Steps

##### Linux

```shell
cd log4sp
mkdir build && cd build
# Replace $SOURCEMOD_HOME with your Sourcemod environment variable or path
# e.g. "~/sourcemod"
python3 ../configure.py --enable-optimize --sm-path $SOURCEMOD_HOME
ambuild
```

##### Windows

idk

### Q & A

#### Loading the plugin failed

##### [SM] Unable to load plugin "XXX.smx": Required extension "Logging for SourcePawn" file("log4sp.ext") not running

1. Check if the `log4sp.ext.XXX` file has been uploaded to `addons/sourcemod/extensions`
2. Check the log message, investigate the reason for the failed loading of `log4sp.ext.XXX`

#### Loading the log4sp extension failed

##### [SM] Unable to load extension "log4sp.ext": Could not find interface: XXX

1. Check if the `log4sp.ext.XXX` matches the operating system
2. Check if the version of `log4sp.ext.XXX` matches the version of the sourcemod version

#### bin/libstdc++.so.6: version `GLIBCXX_3.4.20' not found

- Option 1

    ```shell
    # Reference link: https://stackoverflow.com/questions/44773296/libstdc-so-6-version-glibcxx%203-4-20-not-found
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install gcc-4.9

    # If the issue is resolved, the following step is not needed
    # sudo apt-get upgrade libstdc++6
    ```

- Option 2

    ```shell
    # First, check if the operating system has the required GLIBCXX version
    strings /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep GLIBCXX
    # Then check if the server has the required GLIBCXX version
    strings ./server/bin/libstdc++.so | grep GLIBCXX
    # If the operating system has it but the server doesn't, try renaming the server's ./server/bin/libstdc++.so.6 file to use the operating system's version
    mv ./server/bin/libstdc++.so ./server/bin/libstdc++.so.bk
    ```

## Credits

- **[gabime](https://github.com/gabime) [spdlog](https://github.com/gabime/spdlog)** library implements most of the functionality, log4sp.ext wraps the spdlog API for SourcePawn to use

- Fyren, nosoop, Deathreus provides solution for expanding the Sink Handle of management

If I missed anyone, please contact me.

