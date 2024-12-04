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

#include "spdlog/async.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

#include "extension.h"

#include "log4sp/adapter/sync_logger.h"
#include "log4sp/logger_register.h"
#include "log4sp/sink_register.h"


/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Log4sp g_Log4sp;    /**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Log4sp);

HandleType_t        g_LoggerHandleType = 0;
HandleType_t        g_SinkHandleType   = 0;


bool Log4sp::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
    // Init handle type
    {
        HandleAccess access;
        handlesys->InitAccessDefaults(nullptr, &access);
        access.access[HandleAccess_Delete] = 0;

        HandleError err;

        g_LoggerHandleType = handlesys->CreateType("Logger", this, 0, nullptr, &access, myself->GetIdentity(), &err);
        if (!g_LoggerHandleType)
        {
            snprintf(error, maxlen, "Could not create Logger handle type (err: %d)", err);
            return false;
        }

        g_SinkHandleType = handlesys->CreateType("Sink", this, 0, nullptr, &access, myself->GetIdentity(), &err);
        if (!g_SinkHandleType)
        {
            snprintf(error, maxlen, "Could not create Sink handle type (err: %d)", err);
            return false;
        }
    }

    // Init console command
    if (!rootconsole->AddRootConsoleCommand3(SMEXT_CONF_LOGTAG, "Logging For SourcePawn command menu", this))
    {
        snprintf(error, maxlen, "Could not add root console commmand 'log4sp'.");
        return false;
    }

    // Init Global Thread Pool
    {
        const char *queueSizeStr = smutils->GetCoreConfigValue("Log4sp_ThreadPoolQueueSize");
        auto queueSize = queueSizeStr != nullptr ? static_cast<size_t>(atoi(queueSizeStr)) : static_cast<size_t>(8192);

        const char *threadCountStr = smutils->GetCoreConfigValue("Log4sp_ThreadPoolThreadCount");
        auto threadCount = threadCountStr != nullptr ? static_cast<size_t>(atoi(threadCountStr)) : static_cast<size_t>(1);

        spdlog::init_thread_pool(queueSize, threadCount);
    }

    // Init Default Logger
    {
        auto sink   = std::make_shared<spdlog::sinks::stdout_sink_st>();
        auto logger = std::make_shared<spdlog::logger>(SMEXT_CONF_LOGTAG, sink);

        sink->set_pattern("[%H:%M:%S.%e] [%n] [%l] %v");
        spdlog::set_default_logger(logger);

        HandleSecurity sec = {nullptr, myself->GetIdentity()};

        HandleAccess access;
        handlesys->InitAccessDefaults(nullptr, &access);
        access.access[HandleAccess_Delete] |= HANDLE_RESTRICT_IDENTITY;

        HandleError err;

        auto loggerAdapter = log4sp::sync_logger::create(logger, &sec, &access, &err);
        if (loggerAdapter == nullptr)
        {
            snprintf(error, maxlen, "Could not create default logger handle. (err=%d)", err);
            return false;
        }

        logger->set_level(spdlog::level::trace);
    }

    sharesys->AddNatives(myself, CommonNatives);
    sharesys->AddNatives(myself, LoggerNatives);
    sharesys->AddNatives(myself, SinkNatives);
    sharesys->AddNatives(myself, BaseFileSinkNatives);
    sharesys->AddNatives(myself, ClientChatSinkNatives);
    sharesys->AddNatives(myself, ClientConsoleSinkNatives);
    sharesys->AddNatives(myself, DailyFileSinkNatives);
    sharesys->AddNatives(myself, RotatingFileSinkNatives);
    sharesys->AddNatives(myself, ServerConsoleSinkNatives);

    sharesys->RegisterLibrary(myself, SMEXT_CONF_LOGTAG);

    rootconsole->ConsolePrint("****************** log4sp.ext initialize complete! ******************");
    return true;
}

void Log4sp::SDK_OnUnload()
{
    log4sp::logger_register::instance().shutdown();
    log4sp::sink_register::instance().shutdown();

    handlesys->RemoveType(g_LoggerHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_SinkHandleType, myself->GetIdentity());

    rootconsole->RemoveRootConsoleCommand(SMEXT_CONF_LOGTAG, this);

    spdlog::shutdown();
}

void Log4sp::OnHandleDestroy(HandleType_t type, void *object)
{
    if (type == g_LoggerHandleType)
    {
        auto loggerAdapterPtr = static_cast<log4sp::base_logger *>(object);

        if (spdlog::should_log(spdlog::level::trace))
        {
            auto logger = loggerAdapterPtr->raw();
            SPDLOG_TRACE("Destroy a logger handle. (name='{}', hdl={:X}, ptr={})", logger->name(), static_cast<int>(loggerAdapterPtr->handle()), fmt::ptr(object));
        }

        log4sp::logger_register::instance().drop(loggerAdapterPtr->raw()->name());
        return;
    }

    if (type == g_SinkHandleType)
    {
        auto sinkAdapterPtr = static_cast<log4sp::base_sink *>(object);

        if (spdlog::should_log(spdlog::level::trace))
        {
            SPDLOG_TRACE("Destroy a sink handle. (hdl={:X}, ptr={})", static_cast<int>(sinkAdapterPtr->handle()), fmt::ptr(object));
        }

        log4sp::sink_register::instance().drop(sinkAdapterPtr);
        return;
    }
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
        rootconsole->DrawGenericOption("get_lvl",       "Gets a logger log level.");
        rootconsole->DrawGenericOption("set_lvl",       "Sets a logger log level. [trace, debug, info, warn, error, fatal, off]");
        rootconsole->DrawGenericOption("set_pattern",   "Sets formatting for the sinks in a logger.");
        rootconsole->DrawGenericOption("should_log",    "Gets a logger whether logging is enabled for the given log level.");
        rootconsole->DrawGenericOption("log",           "Use a logger to log a message.");
        rootconsole->DrawGenericOption("flush",         "Manual flush a logger contents.");
        rootconsole->DrawGenericOption("get_flush_lvl", "Gets the minimum log level that will trigger automatic flush.");
        rootconsole->DrawGenericOption("set_flush_lvl", "Sets the minimum log level that will trigger automatic flush. [trace, debug, info, warn, error, fatal, off]");
        rootconsole->DrawGenericOption("should_bt",     "Gets whether backtrace logging is enabled.");
        rootconsole->DrawGenericOption("enable_bt",     "Create new backtrace sink and move to it all our child sinks.");
        rootconsole->DrawGenericOption("disable_bt",    "Restore original sinks and level and delete the backtrace sink.");
        rootconsole->DrawGenericOption("dump_bt",       "Dump log messages stored in the backtrace ring buffer.");
        return;
    }

    const char *name = args->Arg(3);
    auto loggerAdapter = log4sp::logger_register::instance().get(name);
    if (loggerAdapter == nullptr)
    {
        rootconsole->ConsolePrint("[SM] Logger with name '%s' does not exists.", name);
        return;
    }

    auto logger = loggerAdapter->raw();

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
