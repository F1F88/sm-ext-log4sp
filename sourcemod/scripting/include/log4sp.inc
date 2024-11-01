#if defined _logging_for_sourcepawn_included
 #endinput
#endif
#define _logging_for_sourcepawn_included

#pragma newdecls required
#pragma semicolon 1



/**
 * Define LOG4SP_NO_LOG before including this file to replace all log4sp apis with empty statements.
 * This can make the plugin no require log4sp extension
 */
// #define LOG4SP_NO_LOG



#if !defined LOG4SP_NO_LOG

#include <log4sp/common>
#include <log4sp/logger>
#include <log4sp/sinks/sink>
#include <log4sp/sinks/server_console_sink>
#include <log4sp/sinks/base_file_sink>
#include <log4sp/sinks/rotating_file_sink>
#include <log4sp/sinks/daily_file_sink>
#include <log4sp/sinks/client_console_sink>

#else

#include <log4sp_empty/common>
#include <log4sp_empty/logger>
#include <log4sp_empty/sinks/sink>
#include <log4sp_empty/sinks/server_console_sink>
#include <log4sp_empty/sinks/base_file_sink>
#include <log4sp_empty/sinks/rotating_file_sink>
#include <log4sp_empty/sinks/daily_file_sink>
#include <log4sp_empty/sinks/client_console_sink>

#endif








/**
 * Do not edit below this line!
 */
public Extension __ext_log4sp =
{
    name = "Logging for SourcePawn",
    file = "log4sp.ext",
#if defined AUTOLOAD_EXTENSIONS && !defined LOG4SP_NO_LOG
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS && !defined LOG4SP_NO_LOG
    required = 1,
#else
    required = 0,
#endif
};

#define LOG4SP_EXT_VERSION "v1.3.2"

