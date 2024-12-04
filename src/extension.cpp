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
#include "log4sp/command/root_console_command_handler.h"


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
    if (!rootconsole->AddRootConsoleCommand3(SMEXT_CONF_LOGTAG, "Logging For SourcePawn command menu", &log4sp::command::root_console_command_handler::instance()))
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

    rootconsole->RemoveRootConsoleCommand(SMEXT_CONF_LOGTAG, &log4sp::command::root_console_command_handler::instance());

    handlesys->RemoveType(g_LoggerHandleType, myself->GetIdentity());
    handlesys->RemoveType(g_SinkHandleType, myself->GetIdentity());

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
