/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Logging for SourcePawn Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <extension.h>

#include <log4sp/logger_handle_manager.h>
#include <log4sp/sink_handle_manager.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Log4sp g_Log4sp;    /**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Log4sp);

LoggerHandler       g_LoggerHandler;
HandleType_t        g_LoggerHandleType = 0;

SinkHandler         g_SinkHandler;
HandleType_t        g_ServerConsoleSinkHandleType   = 0;
HandleType_t        g_BaseFileSinkHandleType        = 0;
HandleType_t        g_RotatingFileSinkHandleType    = 0;
HandleType_t        g_DailyFileSinkHandleType       = 0;
HandleType_t        g_ClientConsoleSinkHandleType   = 0;


bool Log4sp::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
    HandleError err;
    g_LoggerHandleType = handlesys->CreateType("Logger", &g_LoggerHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_LoggerHandleType)
    {
        snprintf(error, maxlen, "Could not create Logger handle type (err: %d)", err);
        return false;
    }

    // We don't use inheritance because types only can have up to 15 sub-types.
    g_ServerConsoleSinkHandleType = handlesys->CreateType("ServerConsoleSink", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_ServerConsoleSinkHandleType)
    {
        snprintf(error, maxlen, "Could not create ServerConsoleSink handle type (err: %d)", err);
        return false;
    }

    g_BaseFileSinkHandleType = handlesys->CreateType("BaseFileSink", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_BaseFileSinkHandleType)
    {
        snprintf(error, maxlen, "Could not create BaseFileSink handle type (err: %d)", err);
        return false;
    }

    g_RotatingFileSinkHandleType = handlesys->CreateType("RotatingFileSink", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_RotatingFileSinkHandleType)
    {
        snprintf(error, maxlen, "Could not create RotatingFileSink handle type (err: %d)", err);
        return false;
    }

    g_DailyFileSinkHandleType = handlesys->CreateType("DailyFileSink", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_DailyFileSinkHandleType)
    {
        snprintf(error, maxlen, "Could not create DailyFileSink handle type (err: %d)", err);
        return false;
    }

    g_ClientConsoleSinkHandleType = handlesys->CreateType("ClientConsoleSink", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_ClientConsoleSinkHandleType)
    {
        snprintf(error, maxlen, "Could not create ClientConsoleSink handle type (err: %d)", err);
        return false;
    }

    sharesys->AddNatives(myself, CommonNatives);
    sharesys->AddNatives(myself, LoggerNatives);
    sharesys->AddNatives(myself, SinkNatives);
    sharesys->AddNatives(myself, ServerConsoleSinkNatives);
    sharesys->AddNatives(myself, BaseFileSinkNatives);
    sharesys->AddNatives(myself, RotatingFileSinkNatives);
    sharesys->AddNatives(myself, DailyFileSinkNatives);
    sharesys->AddNatives(myself, ClientConsoleSinkNatives);

    if (!rootconsole->AddRootConsoleCommand3("log4sp", "Manager Logging For SourcePawn", this))
    {
        snprintf(error, maxlen, "Could not add root console commmand 'log4sp'.");
        return false;
    }

    sharesys->RegisterLibrary(myself, "log4sp");

    // Init Global Thread Pool
    {
        const char *queueSizeStr = smutils->GetCoreConfigValue("Log4sp_ThreadPoolQueueSize");
        auto queueSize = queueSizeStr != nullptr ? static_cast<size_t>(atoi(queueSizeStr)) : static_cast<size_t>(8192);

        const char *threadCountStr = smutils->GetCoreConfigValue("Log4sp_ThreadPoolThreadCount");
        auto threadCount = threadCountStr != nullptr ? static_cast<size_t>(atoi(threadCountStr)) : static_cast<size_t>(1);

        // rootconsole->ConsolePrint("Thread Pool: queue size = %d.", queueSize);
        // rootconsole->ConsolePrint("Thread Pool: thread count = %d.", threadCount);

        spdlog::init_thread_pool(queueSize, threadCount);
    }

    // Init Default Logger
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_sink_st>();
        consoleSink->set_pattern("[%H:%M:%S.%e] [%n] [%l] %v");

        char filepath[PLATFORM_MAX_PATH];
        smutils->BuildPath(Path_SM, filepath, sizeof(filepath), "logs/log4sp.log");

        auto dailyFileSink = std::make_shared<spdlog::sinks::daily_file_sink_st>(filepath, 0, 0);

        auto sinks = spdlog::sinks_init_list{consoleSink, dailyFileSink};
        auto logger = std::make_shared<spdlog::logger>(SMEXT_CONF_LOGTAG, sinks);

        spdlog::set_default_logger(logger);
    }

    rootconsole->ConsolePrint("****************** log4sp.ext initialize complete! ******************");
    return true;
}

void Log4sp::SDK_OnUnload()
{
    log4sp::logger_handle_manager::instance().shutdown();
    log4sp::sink_handle_manager::instance().shutdown();

    handlesys->RemoveType(g_LoggerHandleType, myself->GetIdentity());

    handlesys->RemoveType(g_ServerConsoleSinkHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_BaseFileSinkHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_RotatingFileSinkHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_DailyFileSinkHandleType, myself->GetIdentity());

    rootconsole->RemoveRootConsoleCommand("log4sp", this);

    spdlog::shutdown();
}

void LoggerHandler::OnHandleDestroy(HandleType_t type, void *object)
{
    auto logger = static_cast<spdlog::logger *>(object);

    if (spdlog::should_log(spdlog::level::trace))
    {
        auto data = log4sp::logger_handle_manager::instance().get_data(logger->name());
        SPDLOG_TRACE("Destroy a logger handle. (name='{}', hdl={:X}, ptr={})", logger->name(), static_cast<int>(data->handle()), fmt::ptr(object));
    }

    log4sp::logger_handle_manager::instance().drop(logger->name());
}

void SinkHandler::OnHandleDestroy(HandleType_t type, void *object)
{
    spdlog::sinks::sink *sink = static_cast<spdlog::sinks::sink *>(object);

    if (spdlog::should_log(spdlog::level::trace))
    {
        auto data = log4sp::sink_handle_manager::instance().get_data(sink);
        SPDLOG_TRACE("Destroy a sink handle. (type={}, hdl={:X}, ptr={})", data->handle_type(), static_cast<int>(data->handle()), fmt::ptr(object));
    }

    log4sp::sink_handle_manager::instance().drop(sink);
}

void Log4sp::OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args)
{
    // 0-sm  |  1-log4sp  |  2-func  |  3-logger name  |  x-params
    int argCnt = args->ArgC();
    if (argCnt <= 3) // 注意是个数, 不是索引
    {
        rootconsole->ConsolePrint("Logging for SourcePawn Menu:");
        rootconsole->ConsolePrint("Usage: sm log4sp <function> <logger_name> [arguments]");

        // rootconsole->DrawGenericOption("list", "Show all loggers name."); // ref: https://github.com/gabime/spdlog/issues/180
        rootconsole->DrawGenericOption("get_lvl", "Get a logger logging level.");
        rootconsole->DrawGenericOption("set_lvl", "Set a logger logging level. [trace, debug, info, warn, error, fatal, off]");
        rootconsole->DrawGenericOption("set_pattern", "Change a logger log pattern.");
        rootconsole->DrawGenericOption("should_log", "Get whether logger is enabled at the given level.");
        rootconsole->DrawGenericOption("log", "Logging a Message.");
        rootconsole->DrawGenericOption("flush", "Manual flush logger contents.");
        rootconsole->DrawGenericOption("get_flush_lvl", "Get the minimum log level that will trigger automatic flush.");
        rootconsole->DrawGenericOption("set_flush_lvl", "Set the minimum log level that will trigger automatic flush. [trace, debug, info, warn, error, fatal, off]");
        rootconsole->DrawGenericOption("should_bt", "Create new backtrace sink and move to it all our child sinks.");
        rootconsole->DrawGenericOption("enable_bt", "Create new backtrace sink and move to it all our child sinks.");
        rootconsole->DrawGenericOption("disable_bt", "Restore orig sinks and level and delete the backtrace sink.");
        rootconsole->DrawGenericOption("dump_bt", "Dump the backtrace of logged messages in the logger.");
        return;
    }

    const char *name = args->Arg(3);
    auto data = log4sp::logger_handle_manager::instance().get_data(name);
    if (data == nullptr)
    {
        rootconsole->ConsolePrint("[SM] Logger with name '%s' does not exists.", name);
        return;
    }

    auto logger = data->logger();

    const char *func = args->Arg(2);
    if (!strcmp(func, "get_lvl"))
    {
        spdlog::level::level_enum lvl = logger->level();
        rootconsole->ConsolePrint("[SM] The level of logger '%s' is '%s'.", name, spdlog::level::to_string_view(lvl).data());
        return;
    }

    if (!strcmp(func, "set_lvl"))
    {
        if (argCnt < 5)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp set_lvl <logger_name> <level>");
            return;
        }

        spdlog::level::level_enum lvl;
        {
            const char *str = args->Arg(4);
            if (strlen(str) == 1 && str[0] < '0' + spdlog::level::n_levels && str[0] >= '0')
            {
                lvl = static_cast<spdlog::level::level_enum>(str[0] - '0');
            }
            else
            {
                lvl = spdlog::level::from_str(str); // If name does not exist, return LogLevel_Off
            }
        }

        rootconsole->ConsolePrint("[SM] Setting logger '%s' level to '%s'", name, spdlog::level::to_string_view(lvl).data());
        logger->set_level(lvl);
        return;
    }

    if (!strcmp(func, "set_pattern"))
    {
        if (argCnt < 5)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp set_pattern <logger_name> <pattern>");
            return;
        }

        const char *pattern = args->Arg(4);

        rootconsole->ConsolePrint("[SM] Setting logger '%s' pattern to '%s'.", name, pattern);
        logger->set_pattern(pattern);
        return;
    }

    if (!strcmp(func, "should_log"))
    {
        if (argCnt < 5)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp should_log <logger_name> <level>");
            return;
        }

        spdlog::level::level_enum lvl;
        {
            const char *str = args->Arg(4);
            if (strlen(str) == 1 && str[0] < '0' + spdlog::level::n_levels && str[0] >= '0')
            {
                lvl = static_cast<spdlog::level::level_enum>(str[0] - '0');
            }
            else
            {
                lvl = spdlog::level::from_str(str); // If name does not exist, return LogLevel_Off
            }
        }

        bool result = logger->should_log(lvl);
        rootconsole->ConsolePrint("[SM] The logger '%s' has %s at '%s'.", name, result ? "enabled" : "disabled", spdlog::level::to_string_view(lvl).data());
        return;
    }

    if (!strcmp(func, "log"))
    {
        if (argCnt < 6)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp log <logger_name> <level> <message>");
            return;
        }

        spdlog::level::level_enum lvl;
        {
            const char *str = args->Arg(4);
            if (strlen(str) == 1 && str[0] < '0' + spdlog::level::n_levels && str[0] >= '0')
            {
                lvl = static_cast<spdlog::level::level_enum>(str[0] - '0');
            }
            else
            {
                lvl = spdlog::level::from_str(str); // If name does not exist, return LogLevel_Off
            }
        }

        const char *msg = args->Arg(5);

        rootconsole->ConsolePrint("[SM] Logger '%s' will log message with level '%s': '%s'.", name, spdlog::level::to_string_view(lvl).data(), msg);
        logger->log(lvl, msg);
        return;
    }

    if (!strcmp(func, "flush"))
    {
        rootconsole->ConsolePrint("[SM] Logger '%s' will perform a flush operation.", name);
        logger->flush();
        return;
    }

    if (!strcmp(func, "get_flush_lvl"))
    {
        spdlog::level::level_enum lvl = logger->flush_level();
        rootconsole->ConsolePrint("[SM] The flush level of logger '%s' is '%s'.", name, spdlog::level::to_string_view(lvl).data());
        return;
    }

    if (!strcmp(func, "set_flush_lvl"))
    {
        if (argCnt < 5)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp set_flush_lvl <logger_name> <level>");
            return;
        }

        spdlog::level::level_enum lvl;
        {
            const char *str = args->Arg(4);
            if (strlen(str) == 1 && str[0] < '0' + spdlog::level::n_levels && str[0] >= '0')
            {
                lvl = static_cast<spdlog::level::level_enum>(str[0] - '0');
            }
            else
            {
                lvl = spdlog::level::from_str(str); // If name does not exist, return LogLevel_Off
            }
        }

        rootconsole->ConsolePrint("[SM] Setting logger '%s' flush level to '%s'", name, spdlog::level::to_string_view(lvl).data());
        logger->flush_on(lvl);
        return;
    }

    if (!strcmp(func, "should_bt"))
    {
        rootconsole->ConsolePrint("[SM] Backtrace for logger '%s' is %s.", name, logger->should_backtrace() ? "enabled" : "disabled");
        return;
    }

    if (!strcmp(func, "enable_bt"))
    {
        if (argCnt < 5)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp enable_bt <logger_name> <num>");
            return;
        }

        int num;
        try
        {
            num = std::stoi(args->Arg(4)); // actually enable_backtrace param type is size_t - unsigned long long
        }
        catch(const std::exception &)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp enable_bt <logger_name> <num>");
            return;
        }

        rootconsole->ConsolePrint("[SM] Enable backtrace for logger '%s'. (stored %d messages)", name, num);
        logger->enable_backtrace(num);
        return;
    }

    if (!strcmp(func, "disable_bt"))
    {
        rootconsole->ConsolePrint("[SM] Disable backtrace for logger '%s'.", name);
        logger->disable_backtrace();
        return;
    }

    if (!strcmp(func, "dump_bt"))
    {
        rootconsole->ConsolePrint("[SM] Dump backtrace messages for logger '%s'.", name);
        logger->dump_backtrace();
        return;
    }

    // 所有的 function 都不匹配
    rootconsole->ConsolePrint("[SM] The function '%s' does not exist.", func);
}
