#if defined _logging_for_sourcepawn_included
 #endinput
#endif
#define _logging_for_sourcepawn_included

#pragma newdecls required
#pragma semicolon 1


#include <log4sp/common>
#include <log4sp/logger>
#include <log4sp/sinks/sink>
#include <log4sp/sinks/server_console_sink>
#include <log4sp/sinks/base_file_sink>
#include <log4sp/sinks/rotating_file_sink>
#include <log4sp/sinks/daily_file_sink>
#include <log4sp/sinks/client_console_sink>









/**
 * Do not edit below this line!
 */
public Extension __ext_log4sp =
{
    name = "Logging for SourcePawn",
    file = "log4sp.ext",
#if defined AUTOLOAD_EXTENSIONS
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

