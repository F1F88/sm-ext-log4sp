# Logging for SourcePawn [EN](./readme.md) | [简中](./readme-chi.md)

这是一个 Sourcemod 拓展，封装了 spdlog（非常快的 C++ logging 库）部分 API，可以在 SourcePawn 中更灵活的记录日志。

## 特点

1. 非常快，比 [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) 快得多

   - [spdlog 性能测试](https://github.com/gabime/spdlog#benchmarks)
   - [log4sp 性能测试](./sourcemod/scripting/log4sp-benchmark.sp)

2. 支持每个 Logger(记录器) 和 Sink(输出源) 设定不同的日志级别

   - 低于设定日志级别的日志消息不会输出
   - 日志级别可以动态调整

3. 支持每个 Logger(记录器) 和 Sink(输出源) 设定不同的日志格式

   - 日志消息的组成，与参数格式化无关
   - 日志格式可以动态调整
   - 支持的格式：https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags

4. 支持每个 Logger 设定多个 Sink

   - Logger 输出时会遍历所有 Sink 输出
   - 例如模拟 [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) 可以为 Logger 添加 DailyFileSink 和 ServerConsoleSink
   - 每个 Sink 可以拥有不同的级别
   - 每个 Sink 可以拥有不同的格式

5. 支持多个插件复用同一个 Logger 对象

   - 在一个插件里创建 Logger 后，另一个插件使用 `Logger.Get("name")` 来获取 Logger

6. 支持 Log 时传递可变参数，并格式化为日志消息

   - 出于降低学习成本和开发成本考虑，目前参数格式化方案与 [Format()](https://sm.alliedmods.net/new-api/string/Format) 相同
   - 只需调用 XXXAmxTpl 方法，然后像使用 [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) 一样使用它即可

   - Format 格式化：https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting)

   - 注意：可变参数的字符串最大长度为 2048，超过这个长度的字符会被截断

     如需要记录更长的日志消息，可以自己先提前格式化好字符串，然后使用 `void Trace(const char[] msg)`、`void Debug(const char[] msg)` 这些函数虽然不能接受可变参数，也不能为你格式化参数，但参数 `char[] msg` 的长度不受限制

7. 支持刷新策略

   - 通常日志消息不会立即刷新，而是先暂存在缓冲区
   - 你可以修改 Flush Level 来决定什么时候刷新
   - 刷新策略：https://github.com/gabime/spdlog/wiki/7.-Flush-policy

8. 支持日志回溯

   - 开启后，Trace、Debug 级别的日志存储在环形缓冲区中，显式调用 `DumpBacktrace()` 后才会输出
   - 回溯策略：https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support

9. 支持多种 Sink

   - ServerConsoleSink
     - 类似于 [PrintToServer()](https://sm.alliedmods.net/new-api/console/PrintToServer)
   - ClientConsoleSink
     - 类似于 [PrintToConsole()](https://sm.alliedmods.net/new-api/console/PrintToConsole)
   - BaseFileSink
     - 类似于 [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) server console
     - 但比 [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) 快得多
     - 即使完全模拟 [LogMessage()](https://sm.alliedmods.net/new-api/logging/LogMessage) （额外添加 1 个 ServerConsoleSink），也比 LogMessage 快 5 倍左右
   - RotatingFileSink
   - DailyFileSink

## 使用示例

### 一个简单的示例

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
 * 获取玩家瞄准的实体信息
 */
Action CommandCallback(int client, int args)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client))
    {
        myLogger.Info("[SM] Command is in-game only.");
        return Plugin_Handled;
    }

    int entity = GetClientAimTarget(client, false);
    if (entity == -2)
    {
        myLogger.Fatal("[SM] The GetClientAimTarget() function is not supported.");
        return Plugin_Handled;
    }

    if (entity == -1)
    {
        myLogger.Warn("[SM] No entity is being aimed at.");
        return Plugin_Handled;
    }

    if (!IsValidEntity(entity))
    {
        myLogger.ErrorAmxTpl("[SM] entity %d is invalid.", entity);
        return Plugin_Handled;
    }

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));
    myLogger.InfoAmxTpl("[SM] The client %L is aiming a (%d) %s entity.", client, entity, classname);

    return Plugin_Handled;
}
```

### 更详细一点的示例

```sourcepawn
#include <sourcemod>
#include <log4sp>

/**
 * ClientConsoleSink 的过滤器
 * 用于过滤掉死亡的玩家, 我们只输出日志信息给活着的玩家
 */
Action FilterAlivePlater(int client)
{
    /**
     * 拓展在调用这个 filter 前已经判断了玩家的有效性
     *  if(IsClientInGame(client) == false || IsFakeClient(client) == true)
     *      return;
     *  call filter...
     */
    return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
}

Logger myLogger;

public void OnPluginStart()
{
    // 创建一个 单线程的, 名为 "myLogger" 的 Logger 对象
    myLogger = Logger.CreateServerConsoleLogger("logger-example-2");

    // 配置 myLogger 的输出级别. 默认级别: LogLevel_Info
    myLogger.SetLevel(LogLevel_Debug);

    // 配置日志格式. 默认格式: [2014-10-31 23:46:59.678] [my_loggername] [info] Some message
    myLogger.SetPattern("[Date: %Y/%m/%d %H:%M:%S] [Name: %n] [LogLevel: %l] [message: %v]");

    // 创建一个输出到玩家控制台的输出源 mySink
    // 每个手动创建的 sink 都会注册到拓展内部的 sink register, 即引用数为 1
    ClientConsoleSinkST mySink = new ClientConsoleSinkST();

    // 单独设置 mySink 的级别. 默认为 LogLevel_Trace
    mySink.SetLevel(LogLevel_Info);

    // 设置 mySink 过滤规则为：只输出到活着的玩家的控制台. 默认输出到所有玩家控制台
    mySink.SetFilter(FilterAlivePlater);

    // 将 mySink 输出源添加到记录器 myLogger
    myLogger.AddSink(mySink);

    // 现在 myLogger 有 2 个输出源
    // 1 个是调用 CreateServerConsoleLogger 时添加的输出到服务器控制台的 sink
    // 1 个是调用 AddSink 时添加的输出到活着的玩家控制台的 sink

    // 现在 mySink 有 2 个引用
    // 1 个是 new ClientConsoleSinkST() 后注册到 sink register 的引用
    // 1 个是 myLogger.AddSink() 后 myLogger 对 mySink 的引用

    // 如果你不在需要对 mySink 修改, 可以直接 "删除" 它
    // 这不会发生错误, myLogger 还是可以输出到 server console 以及 client console
    delete mySink;

    // delete mySink; 实际上做的只是从拓展内部的 sink register 里注销 mySink
    // 注销后, mySink 的引用从 2 个减少为 1 个
    // 只有引用归 0 后，mySink 对象才会真正的被 "删除"

    // 在本示例中即为: "delete mySink;"  +  "delete myLogger;"
    // 你也可以使用: "myLogger.DropSink(mySink);"  +  "delete mySink;"

    // 现在我们自定义的 Logger 配置完成了，尽情使用吧
    myLogger.Info("===== Example 2 code initialization is complete! =====");

    // 注册一个命令用于演示
    RegConsoleCmd("sm_log4sp_example2", CommandCallback);
}

Action CommandCallback(int client, int args)
{
    static int count = 0;
    // mySink 的级别是 LogLevel_Warn 所以这条消息不会输出到 client console
    myLogger.DebugAmxTpl("Command 'sm_log4sp_example2' is invoked %d times.", ++count);

    // myLogger 的级别是 LogLevel_Debug 所以这条消息不会输出到任何地方
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

### 更多示例

- [./sourcemod/scripting/log4sp-test.sp](./sourcemod/scripting/log4sp-test.sp)

## 依赖

- [sourcemod](https://github.com/alliedmodders/sourcemod/tree/1.11-dev)
  - 版本：1.11-dev

- [spdlog](https://github.com/gabime/spdlog)
  - 版本：v1.14.1
  - 仅需头文件（已包含在 [./extern](./extern) 文件夹内）


## 基本构建步骤

如果你是一个小白，建议先阅读这两篇文章。这个项目没有那么复杂，但是文章内容对入门有很大的帮助。

- https://wiki.alliedmods.net/Building_SourceMod
- https://wiki.alliedmods.net/Writing_Extensions

### Linux

```shell
cd log4sp
mkdir build && cd build
# 把 $SOURCEMOD_HOME111 替换为你的 sourcemod 环境变量或路径
# 如："/home/nmr/sourcemod"
python3 ../configure.py --enable-optimize --sm-path $SOURCEMOD_HOME111
ambuild
```

### Windows

我不知道

## 疑难解答

1. 服务器启动时报错：**bin/libstdc++.so.6: version `GLIBCXX_3.4.20' not found**

   方案一：

   ```
   # 参考链接：https://stackoverflow.com/questions/44773296/libstdc-so-6-version-glibcxx%203-4-20-not-found
   sudo add-apt-repository ppa:ubuntu-toolchain-r/test
   sudo apt-get update
   sudo apt-get install gcc-4.9

   # 如果问题解决，则不需要下面这一步
   # sudo apt-get upgrade libstdc++6
   ```

   方案二：

   ```
   # 先查看操作系统是否有需要的 GLIBCXX 版本
   strings /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep GLIBCXX
   # 再查看服务器是否有需要的 GLIBCXX 版本
   strings ./server/bin/libstdc++.so | grep GLIBCXX
   # 如果操作系统有，但服务器没有，可以尝试重命名服务器的 ./server/bin/libstdc++.so.6 文件，从而使用操作系统的版本
   mv ./server/bin/libstdc++.so ./server/bin/libstdc++.so.bk
   ```

2. 这个项目代码复杂/学习难度大吗？

   - 这是我制作的的第一个 sourcemod extension 项目，也是我第一个 cpp 项目，作为初学者我并没有使用到什么特别新的特性或者高深的算法与数据结构。所以我认为不难
   - 得益于 sm 1.11 和 spdlog 1.14.1 的强大，大部分功能不需要手动实现，拓展只需要做一层包装，以供 sourcepawn 调用。所以代码相对来说比较简洁

## 致谢

首先感谢 **[gabime](https://github.com/gabime)** 提供了 **[spdlog](https://github.com/gabime/spdlog)** 这么好的一个库，否则这个项目可能不会诞生

感谢以下用户为我解答了拓展开发时的一些问题

- Fyren
- nosoop
- Deathreus

如有遗漏，请联系我

## 待办

1. 完善英语版 readme.md
2. 在 Windows 系统中测试 log4sp.ext.dll
3. 支持 XXXAmxTpl 格式化的字符串无限长度
4. 支持参数格式不匹配时不抛出异常
    - 没记错的话 spdlog 也是这么做的
    - 目前 XXXAmlTpl 调用 sm 内部的 API，如果格式与参数不匹配会直接抛出异常
    - 值得考虑：其他API是否也可需要减少抛出异常？
5. 支持 fmt 风格的格式化方案
    - 可能需要自定义一个 cell_t 类型来识别参数
    - 如何处理参数个数的问题？也许可以参考 flink tuple
6. 实现更多不同类型的 sink
    - 如果有时间的话
7. 支持纯 sourcepawn 自定义 sink
    - 使用一个代理 sink，通过 forward 来调用 sp 自定义的 sink_it\_() 和 flush_()
    - 自定义构造器、以及如何识别自定义的 sink 类型还需要进一步考虑
8. 实现更多的 SPDLOG_API
    - 使用 forward 实现 logger、sinks 的 file_event_handlers
