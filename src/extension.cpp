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

#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/command/root_console_command_handler.h"


/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Log4sp g_Log4sp;    /**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Log4sp);


bool Log4sp::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
    static_assert(!NO_HANDLE_TYPE, "NO_HANDLE_TYPE has changed, conditional statement for handle needs to be modified!");
    static_assert(!BAD_HANDLE, "BAD_HANDLE has changed, conditional statement for handle needs to be modified!");

    try
    {
        log4sp::logger_handler::initialize();
        log4sp::sink_handler::initialize();
        log4sp::root_console_command_handler::initialize();
    }
    catch (const std::exception &ex)
    {
        smutils->Format(error, maxlen, "Initialization failure (reason: %s)", ex.what());
        return false;
    }

    sharesys->AddNatives(myself, CommonNatives);
    sharesys->AddNatives(myself, LoggerNatives);
    sharesys->AddNatives(myself, SinkNatives);
    sharesys->AddNatives(myself, BasicFileSinkNatives);
    sharesys->AddNatives(myself, CallbackSinkNatives);
    sharesys->AddNatives(myself, DailyFileSinkNatives);
    sharesys->AddNatives(myself, RingBufferSinkNatives);
    sharesys->AddNatives(myself, RotatingFileSinkNatives);
    sharesys->AddNatives(myself, ServerConsoleSinkNatives);
    sharesys->AddNatives(myself, TCPSinkNatives);
    sharesys->AddNatives(myself, UDPSinkNatives);

    sharesys->RegisterLibrary(myself, SMEXT_CONF_LOGTAG);

    rootconsole->ConsolePrint("****************** log4sp.ext initialize complete! ******************");
    return true;
}

void Log4sp::SDK_OnUnload()
{
    log4sp::root_console_command_handler::destroy();
    log4sp::sink_handler::destroy();
    log4sp::logger_handler::destroy();
}
