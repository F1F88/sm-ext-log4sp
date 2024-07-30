# Logging for SourcePawn [EN](./readme.md) | [简中](./readme-chi.md)

This is a Sourcemod extension that wraps some of the spdlog API (a very fast C++ logging library) to enable more flexible logging in SourcePawn.

NOTE: This readme.md is derived from chatgpt's translation readme-chi.md

## Features

1. High Performance: Significantly faster than [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - [spdlog benchmarks](https://github.com/gabime/spdlog#benchmarks).
   - [log4sp benchmarks](./sourcemod/scripting/test-log4sp.sp).

2. Supports set different log level for each Logger

   - If the level of a log message is lower than the configured log level, no log message is displayed.
   - Log level can be dynamically adjusted.

3. Supports set different log format for each Logger

   - Log message composition independent of parameter formatting.
   - Log format can be dynamically adjusted.
   - [Custom formatting](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. Supports set multiple Sinks for each Logger

   - The Logger traverses all Sink output when logging.
   - For example, add a DailyFileSink and ServerConsoleSink to a Logger to simulate [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage).
   - Each Sink can have different level and format.

5. Supports multiple plugins reusing the same Logger

   - After creating a Logger in one plugin, another plugin can use `Logger.Get("name")` to retrieve it.

6. Supports format variable parameters when logging

   - For reduced learning and development costs, the parameter formatting scheme is similar to [Format()](https://sm.alliedmods.net/new-api/string/Format). Simply use the XXXAmxTpl method and use it like [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage).

   - [Format Class Functions (SourceMod Scripting)](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting))

   - Note: The maximum length for variable argument strings is 2048 characters. Strings longer than this will be truncated.

     To log longer messages, format the string in advance and use functions like void `Trace(const char[] msg)`, `void Info(const char[] msg)`. While these functions do not accept variable arguments or format parameters for you, the length of the `char[] msg` parameter is unrestricted.

7. Supports flush strategies

   - Typically, log messages are not immediately flushed but temporarily stored in a buffer.
   - Modify the Flush Level to determine when to flush.
   - [Flush policy](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)

8. Supports backtrace

   - When enabled, Trace and Debug level logs are stored in a circular buffer and only output explicitly after calling `DumpBacktrace()`.
   - [Backtrace support](https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support)

9. Supports various log targets:

   - ServerConsoleSink: Prints log messages to the server console.
     - Similar to [PrintToServer()](https://sm.alliedmods.net/new-api/console/PrintToServer).
     - Performance is comparable to [PrintToServer()](https://sm.alliedmods.net/new-api/console/PrintToServer).
   - ClientConsoleSink: Prints log messages to the client console.
     - Similar to [PrintToConsole()](https://sm.alliedmods.net/new-api/console/PrintToConsole).
   - BaseFileSink: Writes log messages to a file.
     - Similar to [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) but does not output log messages to the server console.
     - Much faster than [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage).
     - Even with a complete simulation of [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) (additional ServerConsoleSink), it is approximately 5 times faster than LogMessage.
   - RotatingFileSink: Distinguishes log messages into files based on rotation.
     - After the log file reaches a set size, it creates and writes to a new log file.
     - After creating a set number of files, it deletes old files, rotating their use.
   - DailyFileSink: Distinguishes log messages into files daily.
     - Defaults to switching to a new file at 00:00:00.
     - After creating a set number of files, it deletes old files, rotating their use.

## Usage Examples

### Simple Example

```
#include <sourcemod>
#include <log4sp>

Logger myLogger;
public void OnPluginStart()
{
    myLogger = Logger.CreateServerConsoleLogger("myLogger");
    myLogger.Debug("===== Example code initialization is complete! =====");
}

public void OnClientPutInServer(int client)
{
    myLogger.InfoAmxTpl("Client %L put in server.", client);
}
```

### More Detailed Example

```
#include <sourcemod>
#include <log4sp>

/**
 * ClientConsoleSink filter
 * Used to filter out dead players; we only output log messages to live players.
 */
Action FilterAlivePlayer(int client)
{
    /**
     * Extends the call before this filter has been judged the validity of the player
     *  if(IsClientInGame(client) == false || IsFakeClient(client) == true)
     *      return;
     *  call filter...
     */
    return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
}

Logger myLogger;

public void OnPluginStart()
{
    // Create a single-threaded Logger object named "myLogger".
    myLogger = Logger.CreateServerConsoleLogger("myLogger");

    // Configure myLogger output level. Default level: LogLevel_Info.
    myLogger.SetLevel(LogLevel_Debug);

    // Configure log format. Default format: [2014-10-31 23:46:59.678] [my_loggername] [info] Some message
    myLogger.SetPattern("[Date: %Y/%m/%d %H:%M:%S] [Name: %n] [LogLevel: %l] [message: %v]");

    // Create an output source mySink that outputs to the player console.
    // Every manually created sink is registered to the internal sink register of the extension, with a reference count of 1.
    ClientConsoleSinkST mySink = new ClientConsoleSinkST();

    // Set mySink level separately. Default is LogLevel_Trace.
    mySink.SetLevel(LogLevel_Info);

    // Set mySink filter rule: output only to the console of live players. Default output to all player consoles.
    mySink.SetFilter(FilterAlivePlayer);

    // Add mySink output source to Logger myLogger.
    myLogger.AddSink(mySink);

    // Now myLogger has 2 output sources:
    // 1. A sink that outputs to the server console added when CreateServerConsoleLogger was called.
    // 2. A sink that outputs to the console of live players added when AddSink was called.

    // Now mySink has 2 references:
    // 1. Registered to the sink register after new ClientConsoleSinkST() registration.
    // 2. Referenced by myLogger.AddSink() after myLogger's reference to mySink.

    // If you no longer need to modify mySink, you can "delete" it directly.
    // This will not cause an error, and myLogger can still output to the server console and client console.
    delete mySink;

    // delete mySink; effectively only unregisters mySink from the internal sink register of the extension
    // After unregistering, mySink's reference count decreases from 2 to 1.
    // Only when the reference count is 0, mySink object will be truly "deleted".

    // In this example: "delete mySink;" + "delete myLogger;"
    // Alternatively, you can use: "myLogger.DropSink(mySink);" + "delete mySink;"

    // Now our custom Logger configuration is complete, feel free to use it.
    myLogger.Info("===== Example code initialization is complete! =====");

    // Register a command for demonstration.
    RegConsoleCmd("sm_test_log4sp", CommandCallback);
}

Action CommandCallback(int client, int args)
{
    static int count = 0;
    // mySink level is LogLevel_Warn so this message won't be output to client console.
    myLogger.DebugAmxTpl("Command 'sm_test_log4sp' is invoked %d times.", ++count);

    // myLogger level is LogLevel_Debug so this message won't be output anywhere.
    myLogger.TraceAmxTpl("CommandCallback param client = %L param args = %d.", client, args);

    if (args >= 2)
    {
        LogLevel lvl = view_as<LogLevel>(GetCmdArgInt(1));
        char buffer[256];
        GetCmdArg(2, buffer, sizeof(buffer));

        switch (lvl)
        {
            LogLevel_Trace: myLogger.TraceAmxTpl("[%d] %s", count, buffer);
            LogLevel_Debug: myLogger.DebugAmxTpl("[%d] %s", count, buffer);
            LogLevel_Info: myLogger.InfoAmxTpl("[%d] %s", count, buffer);
            LogLevel_Warn: myLogger.WarnAmxTpl("[%d] %s", count, buffer);
            LogLevel_Error: myLogger.ErrorAmxTpl("[%d] %s", count, buffer);
            LogLevel_Fatal: myLogger.FatalAmxTpl("[%d] %s", count, buffer);
            default: LogLevel_Info: myLogger.WarnAmxTpl("[%d] The level cannot be resolved.", count);
        }
    }
    return Plugin_Handled;
}
```

### Additional Examples

- [./sourcemod/scripting/test-log4sp.sp](./sourcemod/scripting/test-log4sp.sp)

## Dependencies

- [Sourcemod](https://github.com/alliedmodders/sourcemod/tree/1.11-dev)
  - Version: 1.11-dev
- [spdlog](https://github.com/gabime/spdlog)
  - Version: v1.14.1
  - Only requires header files (included in [./extern](./extern) folder)

## Basic Build Steps

If you are new, I recommend reading these two articles first. This project is not very complex, but the articles are very helpful for beginners.

- [Building SourceMod](https://wiki.alliedmods.net/Building_SourceMod)
- [Writing Extensions](https://wiki.alliedmods.net/Writing_Extensions)

### Linux

```shell
cd log4sp
mkdir build && cd build
# Replace $SOURCEMOD_HOME111 with your Sourcemod environment variable or path
# e.g., "/home/nmr/sourcemod"
python3 ../configure.py --enable-optimize --sm-path $SOURCEMOD_HOME111
ambuild
```

### Windows

idk

## Q & A

1. Error on server startup: **bin/libstdc++.so.6: version `GLIBCXX_3.4.20' not found**

   Option 1

   ```shell
   # Reference link: https://stackoverflow.com/questions/44773296/libstdc-so-6-version-glibcxx%203-4-20-not-found
   sudo add-apt-repository ppa:ubuntu-toolchain-r/test
   sudo apt-get update
   sudo apt-get install gcc-4.9

   # If the issue is resolved, the following step is not needed
   # sudo apt-get upgrade libstdc++6
   ```

   Option 2

   ```shell
   # First, check if the operating system has the required GLIBCXX version
   strings /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep GLIBCXX
   # Then check if the server has the required GLIBCXX version
   strings ./server/bin/libstdc++.so | grep GLIBCXX
   # If the operating system has it but the server doesn't, try renaming the server's ./server/bin/libstdc++.so.6 file to use the operating system's version
   ```

2. Is the project code complex/hard to learn?

   - This is my first Sourcemod extension project and my first C++ project. As a beginner, I didn't use any particularly new features or advanced algorithms and data structures. So, I don't think it's difficult.
   - Thanks to the power of SM 1.11 and spdlog 1.14.1, most functionalities don't need to be implemented manually. The extension only needs to act as a wrapper for SourcePawn calls. Therefore, the code is relatively concise.

## Acknowledgements

First, thanks to **[gabime](https://github.com/gabime)** for providing the excellent library **[spdlog](https://github.com/gabime/spdlog)**, without which this project might not have been possible.

Next, thanks to the following projects for providing inspiration:

- Alienmario - [sm-logdebug](https://github.com/Alienmario/sm-logdebug/tree/main)
- Dr. McKay - [logdebug.inc](https://forums.alliedmods.net/showthread.php?t=258855)
- disawar1 - [SM-Logger](https://forums.alliedmods.net/showthread.php?t=317168)

Thanks to the following projects for providing Sourcemod extension code references:

- ProjectSky's https://github.com/ProjectSky/sm-ext-yyjson
- Code4Cookie's https://github.com/Code4Cookie/SM-Cereal

Thanks to the following users for answering some questions I had during extension development:

- Fyren
- nosoop
- Deathreus

If I missed anyone, please contact me.

