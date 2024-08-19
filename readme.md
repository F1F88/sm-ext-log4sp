**[English](./readme.md) | [Chinese](./readme-chi.md)**

## Logging for SourcePawn

This is a Sourcemod extension that wraps the [spdlog](https://github.com/gabime/spdlog) library to enhance SourcePawn logging and debugging.

### Features

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

### Installation

1. Download the latest Zip from [Github Action](https://github.com/F1F88/sm-ext-log4sp/actions) that matches your operating system and sourcemod version

2. Upload the `"addons/sourcemod/extensions/log4sp.ext.XXX"` in the ZIP to the `"game/addons/sourcemod/extensions"` folder

3. Use `#include <log4sp>` to import header files, and then you can use the `log4sp.ext` API (natives)

### Configuration

Usually `log4sp.ext` is initialized with the default value

You can add key-values in `"game/addons/sourcemod/config/core.cfg"` to change the default settings:

```
"Log4sp_ThreadPoolQueueSize"	"<Enter the thread pool queue size>"
"Log4sp_ThreadPoolThreadCount"	"<Enter the number of threads in the thread pool>"
```

So your `core.cfg` will end up looking like:

```
"Core"
{
	[...]

	"Log4sp_ThreadPoolQueueSize"	"8192"
	"Log4sp_ThreadPoolThreadCount"	"1"
}
```

The full set of available options with their default values and documentation are below, you should only put ones you intend to change into core.cfg:

```
"Core"
{
	/**
	 * The queue size of the thread pool
	 */
	"Log4sp_ThreadPoolQueueSize"	"8192"

	/**
	 * Number of worker threads in the thread pool
	 */
	"Log4sp_ThreadPoolThreadCount"	"1"

	/**
	 * Default Logger name
	 */
	"Log4sp_DefaultLoggerName"		"LOG4SP"

	/**
	 * Default Logger type
	 * 0 = Asynchronous, blocking when the queue is full
	 * 1 = Asynchronous, old messages are overrun when the queue is full
	 * 2 = Asynchronous, new messages are discard when the queue is full
	 * Orther = Synchronous
	 */
	"Log4sp_DefaultLoggerType"		"1"

	/**
	 * Default Logger level
	 * option: "trace", "debug", "info", "warn", "error", "fatal", "off"
	 */
	"Log4sp_DefaultLoggerLevel"		"info"

	/**
	 * Default Logger message pattern
     * option: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
	 */
	"Log4sp_DefaultLoggerPattern"	"[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v"

	/**
	 * Default Logger flush level
	 * option: "trace", "debug", "info", "warn", "error", "fatal", "off"
	 */
	"Log4sp_DefaultLoggerFlushOn"	"off"

	/**
	 * Default Logger backtrace message count
	 * 0 = Disable backtrace
	 */
	"Log4sp_DefaultLoggerBacktrace"	"0"
}
```

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

- [Test case](./sourcemod/scripting/log4sp-test.sp)

- [Benchmarks](./sourcemod/scripting/log4sp-benchmark.sp)

### Supported Games

The `Log4sp.ext` should work with all games, but currently only Linux binary files are provided

For the Windows system, what you need to do additionally is to compile this project upload the compiled `log4sp.ext.dll` to `"game/addons/sourcemod/extensions"`

### Benchmarks

Test platform: Windows 11 + VMware + Ubuntu 24.04 LTS + sourcemod 1.11

Host configuration: AMD Ryzen 7 6800H + 32 GB Memory

VMware configuration: 1 CPU  + 8 kernel  + 4 GB Memory

Test case 1: [benchmark-log4sp.sp](./sourcemod/scripting/benchmark-ext.sp)

Test case 2: [benchmark-sm-logging.sp](./sourcemod/scripting/benchmark-sm-logging.sp)

##### Single thread (Synchronous)

```
[benchmark-st]         Sink base file      | Iters 1000000 | Elapsed  0.268 secs   3719518/sec
[benchmark-st]         Sink daily file     | Iters 1000000 | Elapsed  0.278 secs   3589439/sec
[benchmark-st]         Sink rotating file  | Iters 1000000 | Elapsed  0.279 secs   3578598/sec
[benchmark-st]         Sink server console | Iters 1000000 | Elapsed  5.609 secs    178255/sec
```

##### Multi thread (Asynchronous)

```
# Queue size: 8192     Thread count: 1
[benchmark-mt-block]   Sink base file      | Iters 1000000 | Elapsed  0.479 secs   2084762/sec
[benchmark-mt-block]   Sink daily file     | Iters 1000000 | Elapsed  0.488 secs   2046592/sec
[benchmark-mt-block]   Sink rotating file  | Iters 1000000 | Elapsed  0.462 secs   2162868/sec
[benchmark-mt-block]   Sink server console | Iters 1000000 | Elapsed  8.422 secs    118725/sec

[benchmark-mt-overrun] Sink base file      | Iters 1000000 | Elapsed  0.442 secs   2259856/sec
[benchmark-mt-overrun] Sink daily file     | Iters 1000000 | Elapsed  0.438 secs   2280891/sec
[benchmark-mt-overrun] Sink rotating file  | Iters 1000000 | Elapsed  0.442 secs   2260684/sec
[benchmark-mt-overrun] Sink server console | Iters 1000000 | Elapsed  0.379 secs   2632167/sec


# Queue size: 8192     Thread count: 4
[benchmark-mt-block]   Sink base file      | Iters 1000000 | Elapsed  1.049 secs    952753/sec
[benchmark-mt-block]   Sink daily file     | Iters 1000000 | Elapsed  1.086 secs    920584/sec
[benchmark-mt-block]   Sink rotating file  | Iters 1000000 | Elapsed  1.034 secs    967049/sec
[benchmark-mt-block]   Sink server console | Iters 1000000 | Elapsed 15.784 secs     63354/sec

[benchmark-mt-overrun] Sink base file      | Iters 1000000 | Elapsed  0.439 secs   2273952/sec
[benchmark-mt-overrun] Sink daily file     | Iters 1000000 | Elapsed  0.451 secs   2212609/sec
[benchmark-mt-overrun] Sink rotating file  | Iters 1000000 | Elapsed  0.453 secs   2204658/sec
[benchmark-mt-overrun] Sink server console | Iters 1000000 | Elapsed  0.372 secs   2684282/sec


# Queue size: 131072     Thread count: 4
[benchmark-mt-block]   Sink base file      | Iters 1000000 | Elapsed  0.998 secs   1001216/sec
[benchmark-mt-block]   Sink daily file     | Iters 1000000 | Elapsed  0.973 secs   1027070/sec
[benchmark-mt-block]   Sink rotating file  | Iters 1000000 | Elapsed  0.956 secs   1045255/sec
[benchmark-mt-block]   Sink server console | Iters 1000000 | Elapsed 13.952 secs     71671/sec

[benchmark-mt-overrun] Sink base file      | Iters 1000000 | Elapsed  0.472 secs   2116635/sec
[benchmark-mt-overrun] Sink daily file     | Iters 1000000 | Elapsed  0.441 secs   2264892/sec
[benchmark-mt-overrun] Sink rotating file  | Iters 1000000 | Elapsed  0.478 secs   2091503/sec
[benchmark-mt-overrun] Sink server console | Iters 1000000 | Elapsed  0.385 secs   2592245/sec


# Queue size: 8192     Thread count: 8
[benchmark-mt-block]   Sink base file      | Iters 1000000 | Elapsed  1.135 secs    881010/sec
[benchmark-mt-block]   Sink daily file     | Iters 1000000 | Elapsed  1.183 secs    845069/sec
[benchmark-mt-block]   Sink rotating file  | Iters 1000000 | Elapsed  1.193 secs    838199/sec
[benchmark-mt-block]   Sink server console | Iters 1000000 | Elapsed 14.925 secs     67000/sec

[benchmark-mt-overrun] Sink base file      | Iters 1000000 | Elapsed  0.533 secs   1875363/sec
[benchmark-mt-overrun] Sink daily file     | Iters 1000000 | Elapsed  0.569 secs   1754767/sec
[benchmark-mt-overrun] Sink rotating file  | Iters 1000000 | Elapsed  0.508 secs   1967969/sec
[benchmark-mt-overrun] Sink server console | Iters 1000000 | Elapsed  0.394 secs   2532556/sec
```

##### Sourcemod logging

As a reference, [sourcemod logging API](https://sm.alliedmods.net/new-api/logging) was also tested


```
[benchmark] LogMessage    | Iters 1000000 | Elapsed 10.740 secs     93108/sec
[benchmark] LogToFile     | Iters 1000000 | Elapsed  9.091 secs    109989/sec
[benchmark] LogToFileEx   | Iters 1000000 | Elapsed  8.823 secs    113336/sec
[benchmark] PrintToServer | Iters 1000000 | Elapsed  5.779 secs    173024/sec
```

### Q & A

##### Loading the plugin failed

Error: `"[SM] Unable to load plugin "XXX.smx": Required extension "Logging for SourcePawn" file("log4sp.ext") not running"`

- Check if the `log4sp.ext.XXX` file has been uploaded to `"game/addons/sourcemod/extensions"`
- Check the log message, investigate the reason for the failed loading of `log4sp.ext.XXX`

##### Loading the log4sp extension failed

Error: `"[SM] Unable to load extension "log4sp.ext": Could not find interface: XXX"`

- Check if the `log4sp.ext.XXX` matches the operating system
- Check if the version of `log4sp.ext.XXX` matches the version of the sourcemod version

Error: `"bin/libstdc++.so.6: version 'GLIBCXX_3.4.20' not found"`

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
    # If the operating system has it but the server doesn't
    # try renaming the server's ./server/bin/libstdc++.so.6 file to use the operating system's version
    mv ./server/bin/libstdc++.so ./server/bin/libstdc++.so.bk
    ```

    Also see: [wiki](https://wiki.alliedmods.net/Installing_Metamod:Source#Normal_Installation)

### Build Dependencies

- [sourcemod](https://github.com/alliedmodders/sourcemod/tree/1.11-dev)
- [spdlog](https://github.com/gabime/spdlog) (Only requires [include files](https://github.com/gabime/spdlog/tree/v1.x/include/spdlog), already included in ["./extern/spdlog/include/spdlog"](./extern/spdlog/include/spdlog))

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

### Credits

- **[gabime](https://github.com/gabime) [spdlog](https://github.com/gabime/spdlog)** library implements most of the functionality, log4sp.ext wraps the spdlog API for SourcePawn to use

- Fyren, nosoop, Deathreus provides solution for managing the Sink Handle

If I missed anyone, please contact me.

