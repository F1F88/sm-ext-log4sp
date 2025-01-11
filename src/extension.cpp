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
    try
    {
        log4sp::logger_handler::initialize();
        log4sp::sink_handler::initialize();
        log4sp::root_console_command_handler::initialize();
    }
    catch(const std::exception &ex)
    {
        snprintf(error, maxlen, "Initialization failure (reason: %s)", ex.what());
        return false;
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
            snprintf(error, maxlen, "Could not create global logger handle (reason: %s)", ex.what());
            return false;
        }

        auto logger = std::make_shared<log4sp::logger_proxy>(SMEXT_CONF_LOGTAG, sink);
        spdlog::set_default_logger(logger);

        // 全局 logger 属于拓展，不应该被任何插件释放
        HandleSecurity security{myself->GetIdentity(), myself->GetIdentity()};
        HandleAccess access;
        HandleError err;

        handlesys->InitAccessDefaults(nullptr, &access);
        access.access[HandleAccess_Delete] |= HANDLE_RESTRICT_IDENTITY;

        Handle_t handle = log4sp::logger_handler::instance().create_handle(logger, &security, &access, &err);
        if (handle == BAD_HANDLE)
        {
            snprintf(error, maxlen, "Could not create global logger handle (err: %d)", err);
            return false;
        }
    }

    sharesys->AddNatives(myself, CommonNatives);
    sharesys->AddNatives(myself, LoggerNatives);
    sharesys->AddNatives(myself, SinkNatives);
    sharesys->AddNatives(myself, BaseFileSinkNatives);
    sharesys->AddNatives(myself, CallbackSinkNatives);
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
    log4sp::root_console_command_handler::destroy();
    log4sp::sink_handler::destroy();
    log4sp::logger_handler::destroy();

    spdlog::shutdown();
}
