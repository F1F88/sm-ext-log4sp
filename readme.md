## Logging for SourcePawn

**[English](./readme.md) | [Chinese](./readme-chi.md)**

This is a Sourcemod extension that wraps the [spdlog](https://github.com/gabime/spdlog) library, allowing SourcePawn to record logs more flexibly.

### Useage

1. Download the latest Zip from [Github Action](https://github.com/F1F88/sm-ext-log4sp/actions) that matches your operating system and sourcemod version
1. Upload the `addons/sourcemod/extension/log4sp.ext.XXX` in the ZIP to the `game/addons/sourcemod/extension` folder on the server

### Features

1. Very fast, much faster than [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - [spdlog benchmarks](https://github.com/gabime/spdlog#benchmarks)  |  [log4sp benchmarks](./sourcemod/scripting/log4sp-benchmark.sp)

2. Each `Logger` and `Sink` can customize the log level

3. Each `Logger` and `Sink` can customize the [log pattern](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)

4. Each `Logger` and `Sink` can customize the [flush policy](https://github.com/gabime/spdlog/wiki/7.-Flush-policy)

5. Each `Logger` can have multiple `Sink`

   - For example: A `Logger` that has both `ServerConsoleSink` and `DailyFileSink` is similar to [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

6. Each `Logger` can dynamic change the log level and pattern
   - see server command `"sm log4sp"`

7. Supports format parameters with variable numbers

   - [Parameter formatting](https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)) usage is consistent with [LogMessage](https://sm.alliedmods.net/new-api/logging/LogMessage)

   - The maximum length of a variable parameter string is **2048** characters

     If characters exceeding this length will be truncated
     If longer log messages need to be log, non `AmxTpl` methods can be used, e.g. `void Info(const char [] msg)`

8. Supports [backtrace](https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support)

   - When enabled, Trace and Debug level log message are stored in a circular buffer and only output explicitly after calling `DumpBacktrace()`

9. Supports various log targets

   - ServerConsoleSink  (Similar to [PrintToServer](https://sm.alliedmods.net/new-api/console/PrintToServer))
   - ClientConsoleSink  (Similar to [PrintToConsole](https://sm.alliedmods.net/new-api/console/PrintToConsole))
   - BaseFileSink
   - RotatingFileSink
   - DailyFileSink

### Usage Examples

##### Simple Example

```sourcepawn
#include <sdktools>
#include <log4sp>

Logger myLogger;
public void OnPluginStart()
{
    myLogger = Logger.CreateServerConsoleLogger("logger-example-1");

    RegConsoleCmd("sm_log4sp_example1", CommandCallback);

    myLogger.Debug("===== Example 1 code initialization is complete! =====");
}

/**
 * Get client aiming entity info.
 */
Action CommandCallback(int client, int args)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client))
    {
        myLogger.Info("Command is in-game only.");
        return Plugin_Handled;
    }

    int entity = GetClientAimTarget(client, false);
    if (entity == -2)
    {
        myLogger.Fatal("The GetClientAimTarget() function is not supported.");
        return Plugin_Handled;
    }

    if (entity == -1)
    {
        myLogger.Warn("No entity is being aimed at.");
        return Plugin_Handled;
    }

    if (!IsValidEntity(entity))
    {
        myLogger.ErrorAmxTpl("entity %d is invalid.", entity);
        return Plugin_Handled;
    }

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));
    myLogger.InfoAmxTpl("The client %L is aiming a (%d) %s entity.", client, entity, classname);

    return Plugin_Handled;
}
```

##### More Detailed Example

```sourcepawn
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
    myLogger = Logger.CreateServerConsoleLogger("logger-example-2");

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
    myLogger.Info("===== Example 2 code initialization is complete! =====");

    // Register a command for demonstration.
    RegConsoleCmd("sm_log4sp_example2", CommandCallback);
}

Action CommandCallback(int client, int args)
{
    static int count = 0;
    // mySink level is LogLevel_Warn so this message won't be output to client console.
    myLogger.DebugAmxTpl("Command 'sm_log4sp_example2' is invoked %d times.", ++count);

    // myLogger level is LogLevel_Debug so this message won't be output anywhere.
    myLogger.TraceAmxTpl("CommandCallback param client = %L param args = %d.", client, args);

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

- [./sourcemod/scripting/log4sp-benchmark.sp](./sourcemod/scripting/log4sp-benchmark.sp)
- [./sourcemod/scripting/log4sp-test.sp](./sourcemod/scripting/log4sp-test.sp)

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

