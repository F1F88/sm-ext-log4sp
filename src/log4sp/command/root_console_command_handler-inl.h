#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_INL_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_INL_H_

#include "spdlog/fmt/xchar.h"

#include "log4sp/command/root_console_command.h"

#include "log4sp/command/root_console_command_handler.h"

namespace log4sp {

inline root_console_command_handler &root_console_command_handler::instance() {
    static root_console_command_handler singleInstance;

    singleInstance.commands_["list"]            = std::make_unique<list_command>();
    singleInstance.commands_["get_lvl"]         = std::make_unique<get_lvl_command>();
    singleInstance.commands_["set_lvl"]         = std::make_unique<set_lvl_command>();
    singleInstance.commands_["set_pattern"]     = std::make_unique<set_pattern_command>();
    singleInstance.commands_["should_log"]      = std::make_unique<should_log_command>();
    singleInstance.commands_["log"]             = std::make_unique<log_command>();
    singleInstance.commands_["flush"]           = std::make_unique<flush_command>();
    singleInstance.commands_["get_flush_lvl"]   = std::make_unique<get_flush_lvl_command>();
    singleInstance.commands_["set_flush_lvl"]   = std::make_unique<set_flush_lvl_command>();
    singleInstance.commands_["should_bt"]       = std::make_unique<should_bt_command>();
    singleInstance.commands_["enable_bt"]       = std::make_unique<enable_bt_command>();
    singleInstance.commands_["disable_bt"]      = std::make_unique<disable_bt_command>();
    singleInstance.commands_["dump_bt"]         = std::make_unique<dump_bt_command>();

    return singleInstance;
}


inline void root_console_command_handler::draw_menu() {
    rootconsole->ConsolePrint("Logging for SourcePawn Menu:");
    rootconsole->ConsolePrint("Usage: sm log4sp <function_name> <logger_name> [arguments]");

    rootconsole->DrawGenericOption("list",          "List all logger names.");
    rootconsole->DrawGenericOption("get_lvl",       fmt::format("Gets a logger log level. [{}]", fmt::join(spdlog::level::level_string_views, " < ")).c_str());
    rootconsole->DrawGenericOption("set_lvl",       fmt::format("Sets a logger log level. [{}]", fmt::join(spdlog::level::level_string_views, " < ")).c_str());
    rootconsole->DrawGenericOption("set_pattern",   "Sets a logger log pattern.");
    rootconsole->DrawGenericOption("should_log",    "Gets a logger whether logging is enabled for the given log level.");
    rootconsole->DrawGenericOption("log",           "Use a logger to log a message.");
    rootconsole->DrawGenericOption("flush",         "Manual flush a logger contents.");
    rootconsole->DrawGenericOption("get_flush_lvl", "Gets the minimum log level that will trigger automatic flush.");
    rootconsole->DrawGenericOption("set_flush_lvl", "Sets the minimum log level that will trigger automatic flush.");
    rootconsole->DrawGenericOption("should_bt",     "Gets a logger whether backtrace logging is enabled.");
    rootconsole->DrawGenericOption("enable_bt",     "Enable a logger backtrace logging.");
    rootconsole->DrawGenericOption("disable_bt",    "Disaable a logger backtrace logging.");
    rootconsole->DrawGenericOption("dump_bt",       "Dump a logger backtrace logging messages.");
}


inline void root_console_command_handler::OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args) {
    // 0-sm  |  1-log4sp  |  2-function name  |  3-logger name  |  x-params
    int argCnt = args->ArgC();
    if (argCnt <= 2) {
        draw_menu();
        return;
    }

    std::string function_name {args->Arg(2)};

    auto iter = commands_.find(function_name);
    if (iter != commands_.end()) {
        iter->second->execute(args);
    } else {
        rootconsole->ConsolePrint("[SM] The function name '%s' does not exist.", function_name.c_str());
        rootconsole->ConsolePrint("-------------------------------------------");
        draw_menu();
    }
}


}       // namespace log4sp
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_INL_H_
