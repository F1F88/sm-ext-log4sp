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

#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/command/root_console_command_handler.h"
#include "log4sp/proxy/logger_proxy.h"


/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Log4sp g_Log4sp;    /**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Log4sp);


bool Log4sp::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
    // Init handle type
    {
        try
        {
            log4sp::logger_handler::initialize();
        }
        catch(const std::exception &ex)
        {
            snprintf(error, maxlen, "Could not create Logger handle type. (reason: %s)", ex.what());
            return false;
        }

        try
        {
            log4sp::sink_handler::initialize();
        }
        catch(const std::exception &ex)
        {
            snprintf(error, maxlen, "Could not create Sink handle type. (reason: %s)", ex.what());
            return false;
        }
    }

    // Init console command
    if (!rootconsole->AddRootConsoleCommand3(SMEXT_CONF_LOGTAG, "Logging For SourcePawn command menu", &log4sp::root_console_command_handler::instance()))
    {
        snprintf(error, maxlen, "Could not add root console commmand 'log4sp'.");
        return false;
    }

    // Init Global Thread Pool
    {
        const char *queueSizeStr = smutils->GetCoreConfigValue("Log4sp_ThreadPoolQueueSize");
        auto queueSize = queueSizeStr != nullptr ? static_cast<size_t>(atoi(queueSizeStr)) : static_cast<size_t>(8192);

        if (queueSize == 0 || queueSize > 1024 * 1024 * 10)
        {
            snprintf(error, maxlen, "Invalid configuration \"Log4sp_ThreadPoolQueueSize\" (%s), valid range is 1-10485760.", queueSizeStr);
            return false;
        }

        const char *threadCountStr = smutils->GetCoreConfigValue("Log4sp_ThreadPoolThreadCount");
        auto threadCount = threadCountStr != nullptr ? static_cast<size_t>(atoi(threadCountStr)) : static_cast<size_t>(1);

        if (threadCount == 0 || threadCount > 1000)
        {
            snprintf(error, maxlen, "Invalid configuration \"Log4sp_ThreadPoolThreadCount\" (%s), valid range is 1-1000.", threadCountStr);
            return false;
        }

        try
        {
            spdlog::init_thread_pool(queueSize, threadCount);
        }
        catch(const std::exception &ex)
        {
            snprintf(error, maxlen, "Could not create global thread pool, reason : %s", ex.what());
            return false;
        }
    }

    // Init Global Logger
    {
        spdlog::sink_ptr sink;
        try
        {
            sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
        }
        catch(const std::exception &ex)
        {
            snprintf(error, maxlen, "Could not create global logger handle, reason : %s", ex.what());
            return false;
        }

        auto logger = std::make_shared<log4sp::logger_proxy>(SMEXT_CONF_LOGTAG, sink);
        spdlog::set_default_logger(logger);

        // 默认 logger 属于拓展，不应该被任何插件释放
        HandleSecurity security{myself->GetIdentity(), myself->GetIdentity()};

        HandleAccess access;
        handlesys->InitAccessDefaults(nullptr, &access);
        access.access[HandleAccess_Delete] |= HANDLE_RESTRICT_IDENTITY;

        HandleError err;

        Handle_t handle = log4sp::logger_handler::instance().create_handle(logger, &security, &access, &err);
        if (handle == BAD_HANDLE)
        {
            snprintf(error, maxlen, "Could not create global logger handle. (err: %d)", err);
            return false;
        }
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
    rootconsole->RemoveRootConsoleCommand(SMEXT_CONF_LOGTAG, &log4sp::root_console_command_handler::instance());

    assert(log4sp::sink_handler::instance().handle_type() != NO_HANDLE_TYPE);
    assert(log4sp::logger_handler::instance().handle_type() != NO_HANDLE_TYPE);

    log4sp::sink_handler::destroy();
    log4sp::logger_handler::destroy();

    spdlog::shutdown();
}
