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

#include "extension.h"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_sinks.h"

#include <log4sp/common.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Log4sp g_Log4sp;    /**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Log4sp);

LoggerHandler                   g_LoggerHandler;
HandleType_t                    g_LoggerHandleType = 0;

SinkHandler                     g_SinkHandler;
HandleType_t                    g_ServerConsoleSinkSTHandleType = 0;
HandleType_t                    g_ServerConsoleSinkMTHandleType = 0;
HandleType_t                    g_BaseFileSinkSTHandleType = 0;
HandleType_t                    g_BaseFileSinkMTHandleType = 0;
HandleType_t                    g_RotatingFileSinkSTHandleType = 0;
HandleType_t                    g_RotatingFileSinkMTHandleType = 0;
HandleType_t                    g_DailyFileSinkSTHandleType = 0;
HandleType_t                    g_DailyFileSinkMTHandleType = 0;
HandleType_t                    g_ClientConsoleSinkSTHandleType = 0;
HandleType_t                    g_ClientConsoleSinkMTHandleType = 0;


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
    g_ServerConsoleSinkSTHandleType = handlesys->CreateType("ServerConsoleSinkST", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_ServerConsoleSinkSTHandleType)
    {
        snprintf(error, maxlen, "Could not create ServerConsoleSinkST handle type (err: %d)", err);
        return false;
    }

    g_ServerConsoleSinkMTHandleType = handlesys->CreateType("ServerConsoleSinkMT", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_ServerConsoleSinkMTHandleType)
    {
        snprintf(error, maxlen, "Could not create ServerConsoleSinkMT handle type (err: %d)", err);
        return false;
    }

    g_BaseFileSinkSTHandleType = handlesys->CreateType("BaseFileSinkST", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_BaseFileSinkSTHandleType)
    {
        snprintf(error, maxlen, "Could not create BaseFileSinkST handle type (err: %d)", err);
        return false;
    }

    g_BaseFileSinkMTHandleType = handlesys->CreateType("BaseFileSinkMT", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_BaseFileSinkMTHandleType)
    {
        snprintf(error, maxlen, "Could not create BaseFileSinkMT handle type (err: %d)", err);
        return false;
    }

    g_RotatingFileSinkSTHandleType = handlesys->CreateType("RotatingFileSinkST", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_RotatingFileSinkSTHandleType)
    {
        snprintf(error, maxlen, "Could not create RotatingFileSinkST handle type (err: %d)", err);
        return false;
    }

    g_RotatingFileSinkMTHandleType = handlesys->CreateType("RotatingFileSinkMT", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_RotatingFileSinkMTHandleType)
    {
        snprintf(error, maxlen, "Could not create RotatingFileSinkMT handle type (err: %d)", err);
        return false;
    }

    g_DailyFileSinkSTHandleType = handlesys->CreateType("DailyFileSinkST", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_DailyFileSinkSTHandleType)
    {
        snprintf(error, maxlen, "Could not create DailyFileSinkST handle type (err: %d)", err);
        return false;
    }

    g_DailyFileSinkMTHandleType = handlesys->CreateType("DailyFileSinkMT", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_DailyFileSinkMTHandleType)
    {
        snprintf(error, maxlen, "Could not create DailyFileSinkMT handle type (err: %d)", err);
        return false;
    }

    g_ClientConsoleSinkSTHandleType = handlesys->CreateType("ClientConsoleSinkST", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_ClientConsoleSinkSTHandleType)
    {
        snprintf(error, maxlen, "Could not create ClientConsoleSinkST handle type (err: %d)", err);
        return false;
    }

    g_ClientConsoleSinkMTHandleType = handlesys->CreateType("ClientConsoleSinkMT", &g_SinkHandler, 0, NULL, NULL, myself->GetIdentity(), &err);
    if (!g_ClientConsoleSinkMTHandleType)
    {
        snprintf(error, maxlen, "Could not create ClientConsoleSinkMT handle type (err: %d)", err);
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

    spdlog::init_thread_pool(8192, 1);
    spdlog::set_default_logger(spdlog::stdout_logger_mt("log4sp"));
    SPDLOG_INFO("****************** log4sp.ext initialize complete! ******************");
    return true;
}

void Log4sp::SDK_OnUnload()
{
    log4sp::SinkHandleRegistry::instance().dropAll();

    handlesys->RemoveType(g_LoggerHandleType, myself->GetIdentity());

    handlesys->RemoveType(g_ServerConsoleSinkSTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_ServerConsoleSinkMTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_BaseFileSinkSTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_BaseFileSinkMTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_RotatingFileSinkSTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_RotatingFileSinkMTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_DailyFileSinkSTHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_DailyFileSinkMTHandleType, myself->GetIdentity());

    rootconsole->RemoveRootConsoleCommand("log4sp", this);

    spdlog::shutdown();
}

void LoggerHandler::OnHandleDestroy(HandleType_t type, void *object)
{
    spdlog::logger *logger = static_cast<spdlog::logger *>(object);
    SPDLOG_TRACE("Destroy a Logger type({}) object({}) name({}).", type, fmt::ptr(object), logger->name());
    spdlog::drop(logger->name());
}

void SinkHandler::OnHandleDestroy(HandleType_t type, void *object)
{
    spdlog::sinks::sink *sink = static_cast<spdlog::sinks::sink *>(object);

    // 所以为什么 OnHandleDestroy 只有 HandleType_t 而没有 Handle？
    if (!log4sp::SinkHandleRegistry::instance().drop(sink)) // O(n)
    {
        SPDLOG_CRITICAL("Destroy a Sink handle of unknown type (type={}, obj={}).", type, fmt::ptr(object));
    }
    else
    {
        SPDLOG_TRACE("Destroy a Sink handle of a known type (type={}, obj={}).", type, fmt::ptr(object));
    }
}

void Log4sp::OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args)
{
    // 0-sm  |  1-log4sp  |  2-func  |  3-logger name  |  x-params
    int argCnt = args->ArgC();
    if (argCnt < 3)
    {
        rootconsole->ConsolePrint("Logging for SourcePawn Menu:");

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

    if (argCnt < 4)
    {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp <function> <logger_name>");
        return;
    }

    const char *name = args->Arg(3);
    std::shared_ptr<spdlog::logger> logger = spdlog::get(name);
    if (logger == nullptr)
    {
        rootconsole->ConsolePrint("[SM] The logger named '%s' does not exist.", name);
        return;
    }

    const char *func = args->Arg(2);
    if (!strcmp(func, "get_lvl"))
    {
        spdlog::level::level_enum lvl = logger->level();
        rootconsole->ConsolePrint("[SM] The level of the logger '%s' is '%s'.", name, spdlog::level::to_string_view(lvl));
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

        rootconsole->ConsolePrint("[SM] Set the logger '%s' level to '%s'", name, spdlog::level::to_string_view(lvl));
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

        rootconsole->ConsolePrint("[SM] Set the logger '%s' pattern to '%s'.", name, pattern);
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
        rootconsole->ConsolePrint("[SM] The logger '%s' has %s at '%s'.", name, result ? "enabled" : "disabled", spdlog::level::to_string_view(lvl));
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

        rootconsole->ConsolePrint("[SM] Use logger '%s' log a '%s' level message.", name, spdlog::level::to_string_view(lvl));
        logger->log(lvl, msg);
        return;
    }

    if (!strcmp(func, "flush"))
    {
        rootconsole->ConsolePrint("[SM] Logger '%s' starts flush.", name);
        logger->flush();
        return;
    }

    if (!strcmp(func, "get_flush_lvl"))
    {
        spdlog::level::level_enum lvl = logger->flush_level();
        rootconsole->ConsolePrint("[SM] The flush level of the logger '%s' is '%s'.", name, spdlog::level::to_string_view(lvl));
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

        rootconsole->ConsolePrint("[SM] Set the logger '%s' flush level to '%s'", name, spdlog::level::to_string_view(lvl));
        logger->flush_on(lvl);
        return;
    }

    if (!strcmp(func, "should_bt"))
    {
        bool result = logger->should_backtrace();
        rootconsole->ConsolePrint("[SM] The logger '%s' has %s backtrace.", name, result ? "enabled" : "disabled");
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
        catch(const std::exception &ex)
        {
            rootconsole->ConsolePrint("[SM] Usage: sm log4sp enable_bt <logger_name> <num>");
            return;
        }

        rootconsole->ConsolePrint("[SM] Enable logger '%s' backtrace. (stored %d messages)", name, num);
        logger->enable_backtrace(num);
        return;
    }

    if (!strcmp(func, "disable_bt"))
    {
        rootconsole->ConsolePrint("[SM] Disable logger '%s' backtrace.", name);
        logger->disable_backtrace();
        return;
    }

    if (!strcmp(func, "dump_bt"))
    {
        rootconsole->ConsolePrint("[SM] Dump logger '%s' backtrace messages.", name);
        logger->dump_backtrace();
        return;
    }

    // 所有的 function 都不匹配
    rootconsole->ConsolePrint("[SM] The function '%s' does not exist.", func);
}
