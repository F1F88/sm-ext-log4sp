#include "spdlog/fmt/xchar.h"

#include "log4sp/common.h"
#include "log4sp/command/root_console_command_handler.h"


namespace log4sp {

void root_console_command_handler::initialize() {
    instance().add_root_console_command_();
}

void root_console_command_handler::destroy() noexcept {
    instance().remove_root_console_command_();
}


void root_console_command_handler::draw_menu() noexcept {
    using spdlog::fmt_lib::format;
    using spdlog::fmt_lib::join;
    using spdlog::level::level_string_views;

    rootconsole->ConsolePrint(SMEXT_CONF_NAME " Menu:");
    rootconsole->ConsolePrint("Usage: sm log4sp <function_name> [arguments]");

    rootconsole->DrawGenericOption("list",          "List all logger names.");
    rootconsole->DrawGenericOption("apply_all",     "Apply a command function on all loggers.");
    rootconsole->DrawGenericOption("get_lvl",       format("Gets a logger log level. [{}]", join(level_string_views, " < ")).c_str());
    rootconsole->DrawGenericOption("set_lvl",       format("Sets a logger log level. [{}]", join(level_string_views, " < ")).c_str());
    rootconsole->DrawGenericOption("set_pattern",   "Sets a logger log pattern.");
    rootconsole->DrawGenericOption("should_log",    "Gets a logger whether logging is enabled for the given log level.");
    rootconsole->DrawGenericOption("log",           "Use a logger to log a message.");
    rootconsole->DrawGenericOption("flush",         "Manual flush a logger contents.");
    rootconsole->DrawGenericOption("get_flush_lvl", "Gets the minimum log level that will trigger automatic flush.");
    rootconsole->DrawGenericOption("set_flush_lvl", "Sets the minimum log level that will trigger automatic flush.");
    rootconsole->DrawGenericOption("version",       "Display version information");
}


command::result root_console_command_handler::execute(const std::vector<std::string> &args) noexcept {
    auto cmdname = args.at(0);
    auto iter = commands_.find(cmdname);
    if (iter == commands_.end()) {
        return command::param_error("Command function \"" + cmdname + "\" not found.");
    }

    auto arguments = std::vector<std::string>{args.begin() + 1, args.end()};
    return iter->second->execute(arguments);
}


void root_console_command_handler::OnRootConsoleCommand(const char *cmdname, const SourceMod::ICommandArgs *args) {
    // 0-sm  |  1-log4sp  |  2-function name  |  3-logger name  |  x-params
    int argCnt = args->ArgC();
    if (argCnt <= 2) {
        draw_menu();
        return;
    }

    std::vector<std::string> arguments;
    for (int i = 2; i < argCnt; ++i) {
        arguments.emplace_back(args->Arg(i));
    }

    auto [s, err] = execute(arguments);
    if (s != command::status::ok) {
        rootconsole->ConsolePrint("[SM] %s", err.value().c_str());
    }
}


void root_console_command_handler::add_root_console_command_() {
    if (!rootconsole->AddRootConsoleCommand3(SMEXT_CONF_LOGTAG, SMEXT_CONF_NAME " command menu", this)) {
        throw std::runtime_error("SM error! Could not add root console commmand \"" SMEXT_CONF_LOGTAG "\".");
    }
}


void root_console_command_handler::remove_root_console_command_() noexcept {
#ifdef DEBUG
    assert(rootconsole->RemoveRootConsoleCommand(SMEXT_CONF_LOGTAG, this));
#else
    rootconsole->RemoveRootConsoleCommand(SMEXT_CONF_LOGTAG, this);
#endif
}


inline const auto root_console_command_handler::commands_ = []{
    std::unordered_map<std::string_view, std::unique_ptr<command>> map;
    map.emplace("list",            std::make_unique<list_command>());
    map.emplace("apply_all",       std::make_unique<apply_all_command>());
    map.emplace("get_lvl",         std::make_unique<get_lvl_command>());
    map.emplace("set_lvl",         std::make_unique<set_lvl_command>());
    map.emplace("set_pattern",     std::make_unique<set_pattern_command>());
    map.emplace("should_log",      std::make_unique<should_log_command>());
    map.emplace("log",             std::make_unique<log_command>());
    map.emplace("flush",           std::make_unique<flush_command>());
    map.emplace("get_flush_lvl",   std::make_unique<get_flush_lvl_command>());
    map.emplace("set_flush_lvl",   std::make_unique<set_flush_lvl_command>());
    map.emplace("version",         std::make_unique<version_command>());
    return map;
}();


}       // namespace log4sp
