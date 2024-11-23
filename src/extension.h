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

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Logging for sourcepawn extension code header.
 */
#include "smsdk_ext.h"

/**
 * @brief Implementation of the logger in SourcePawn Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class Log4sp : public SDKExtension, public IRootConsoleCommand
{
public:
    /**
     * @brief This is called after the initial loading sequence has been processed.
     *
     * @param error     Error message buffer.
     * @param maxlen    Size of error message buffer.
     * @param late      Whether or not the module was loaded after map load.
     * @return          True to succeed loading, false to fail.
     */
    virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);

    /**
     * @brief This is called right before the extension is unloaded.
     */
    virtual void SDK_OnUnload();

    /**
     * @brief This is called once all known extensions have been loaded.
     * Note: It is is a good idea to add natives here, if any are provided.
     */
    // virtual void SDK_OnAllLoaded();

    /**
     * @brief Called when the pause state is changed.
     */
    //virtual void SDK_OnPauseChange(bool paused);

    /**
     * @brief this is called when Core wants to know if your extension is working.
     *
     * @param error     Error message buffer.
     * @param maxlen    Size of error message buffer.
     * @return          True if working, false otherwise.
     */
    //virtual bool QueryRunning(char *error, size_t maxlen);
public:
    #if defined SMEXT_CONF_METAMOD
    /**
     * @brief Called when Metamod is attached, before the extension version is called.
     *
     * @param error         Error buffer.
     * @param maxlen        Maximum size of error buffer.
     * @param late          Whether or not Metamod considers this a late load.
     * @return              True to succeed, false to fail.
     */
    //virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);

    /**
     * @brief Called when Metamod is detaching, after the extension version is called.
     * NOTE: By default this is blocked unless sent from SourceMod.
     *
     * @param error         Error buffer.
     * @param maxlen        Maximum size of error buffer.
     * @return              True to succeed, false to fail.
     */
    //virtual bool SDK_OnMetamodUnload(char *error, size_t maxlen);

    /**
     * @brief Called when Metamod's pause state is changing.
     * NOTE: By default this is blocked unless sent from SourceMod.
     *
     * @param paused        Pause state being set.
     * @param error         Error buffer.
     * @param maxlen        Maximum size of error buffer.
     * @return              True to succeed, false to fail.
     */
    //virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlen);
#endif

public:
    /**
     * @brief Handles a root console menu action.
     */
    void OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args);
};


class LoggerHandler : public IHandleTypeDispatch
{
public:
    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(HandleType_t type, void *object);
};

class SinkHandler : public IHandleTypeDispatch
{
public:
    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(HandleType_t type, void *object);
};

extern Log4sp                   g_Log4sp;

extern LoggerHandler            g_LoggerHandler;
extern HandleType_t             g_LoggerHandleType;

extern SinkHandler              g_SinkHandler;
extern HandleType_t             g_ServerConsoleSinkSTHandleType;
extern HandleType_t             g_ServerConsoleSinkMTHandleType;
extern HandleType_t             g_BaseFileSinkSTHandleType;
extern HandleType_t             g_BaseFileSinkMTHandleType;
extern HandleType_t             g_RotatingFileSinkSTHandleType;
extern HandleType_t             g_RotatingFileSinkMTHandleType;
extern HandleType_t             g_DailyFileSinkSTHandleType;
extern HandleType_t             g_DailyFileSinkMTHandleType;
extern HandleType_t             g_ClientConsoleSinkSTHandleType;
extern HandleType_t             g_ClientConsoleSinkMTHandleType;

extern const sp_nativeinfo_t    CommonNatives[];
extern const sp_nativeinfo_t    LoggerNatives[];
extern const sp_nativeinfo_t    SinkNatives[];
extern const sp_nativeinfo_t    ServerConsoleSinkNatives[];
extern const sp_nativeinfo_t    BaseFileSinkNatives[];
extern const sp_nativeinfo_t    RotatingFileSinkNatives[];
extern const sp_nativeinfo_t    DailyFileSinkNatives[];
extern const sp_nativeinfo_t    ClientConsoleSinkNatives[];

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
