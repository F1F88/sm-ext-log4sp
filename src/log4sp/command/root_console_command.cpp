#include <regex>
#include <stdlib.h>

#include "spdlog/fmt/xchar.h"

#include "log4sp/adapter/logger_handler.h"

#include "log4sp/command/root_console_command.h"
#include "log4sp/command/root_console_command_handler.h"


namespace log4sp {

using spdlog::fmt_lib::format;
using spdlog::fmt_lib::join;
using spdlog::level::level_enum;
using spdlog::level::to_string_view;

std::shared_ptr<logger> command::arg_to_logger(const std::string &arg) {
    // 尝试按名字查找 object
    auto logger = logger_handler::instance().find_logger(arg);
    if (logger == nullptr) {
        throw_log4sp_ex("Logger with name \"" + arg + "\" not exists.");
    }
    return logger;
}

level_enum command::arg_to_level(const std::string &arg) {
    // 尝试按名字转换
    level_enum level{str_to_lvl(arg.c_str())};

    // 尝试按数字转换
    if (level == level_enum::off) {
        try {
            int number = std::stoi(arg);
            level = num_to_lvl(number);
        } catch (const std::exception &) {
            level = level_enum::off;
        }
    }

    return level;
}


void list_command::execute(const std::vector<std::string> &args) {
    std::vector<std::string> names;
    log4sp::logger_handler::instance().apply_all(
        [&names](std::shared_ptr<logger> logger) {
            names.push_back(logger->name());
        }
    );

    rootconsole->ConsolePrint("%s", format("[SM] List of all logger names: [{}].", join(names, ", ")).c_str());
}


void apply_all_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw_log4sp_ex(format("Usage: sm log4sp apply_all <function_name> [arguments]\nFunction names: [{}]", join(functions_, ", ")));
    }

    auto function_name = args[0];
    auto iter = functions_.find(function_name);
    if (iter == functions_.end()) {
        throw_log4sp_ex("Command function name \"" + function_name + "\" not exists.");
    }

    std::vector<std::string> arguments{args};

    logger_handler::instance().apply_all(
        [&function_name, &arguments](std::shared_ptr<logger> logger) {
            arguments[0] = logger->name();

            try {
                root_console_command_handler::instance().execute(function_name, arguments);
            } catch (const std::exception &ex) {
                // 如果是参数格式问题，将消息替换为 apply_all 格式
                static const std::regex match_usage_pattern{R"(Usage: sm log4sp [a-z_]+ <logger_name>.*)"};
                static const std::regex replace_logger_name_pattern{R"( <logger_name>)"};
                static const std::regex replace_prefix_pattern{R"(Usage: sm log4sp )"};

                std::string msg{ex.what()};
                if (std::regex_match(msg, match_usage_pattern)) {
                    msg = std::regex_replace(msg, replace_logger_name_pattern, "");
                    msg = std::regex_replace(msg, replace_prefix_pattern, "Usage: sm log4sp apply_all ");
                }
                throw_log4sp_ex(msg);
            }
        }
    );
}


void get_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw_log4sp_ex("Usage: sm log4sp get_lvl <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = logger->level();

    rootconsole->ConsolePrint("[SM] Logger '%s' log level is '%s'.", logger->name().c_str(), to_string_view(level).data());
}


void set_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw_log4sp_ex("Usage: sm log4sp set_lvl <logger_name> <level>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);

    if (level == logger->level()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' log level is already '%s' level.", logger->name().c_str(), to_string_view(level).data());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log level to '%s'", logger->name().c_str(), to_string_view(level).data());
    logger->set_level(level);
}


void set_pattern_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw_log4sp_ex("Usage: sm log4sp set_pattern <logger_name> <pattern>");
    }

    auto logger  = arg_to_logger(args[0]);
    auto pattern = args[1];

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log pattern to '%s'", logger->name().c_str(), pattern.c_str());
    logger->set_pattern(pattern);
}


void should_log_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw_log4sp_ex("Usage: sm log4sp should_log <logger_name> <level>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);
    bool result = logger->should_log(level);

    rootconsole->ConsolePrint("[SM] Logger '%s' has %s '%s' log level.", logger->name().c_str(), result ? "enabled" : "disabled", to_string_view(level).data());
}


void log_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 3) {
        throw_log4sp_ex("Usage: sm log4sp log <logger_name> <level> <message>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);
    auto msg    = args[2];

    rootconsole->ConsolePrint("[SM] Logger '%s' will log a message '%s' with log level '%s'.", logger->name().c_str(), msg.c_str(), to_string_view(level).data());
    logger->log({__FILE__, __LINE__, __FUNCTION__}, level, msg, nullptr);
}


void flush_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw_log4sp_ex("Usage: sm log4sp flush <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);

    rootconsole->ConsolePrint("[SM] Logger '%s' will flush its contents.", logger->name().c_str());
    logger->flush({__FILE__, __LINE__, __FUNCTION__}, nullptr);
}


void get_flush_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw_log4sp_ex("Usage: sm log4sp get_flush_lvl <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = logger->flush_level();

    rootconsole->ConsolePrint("[SM] Logger '%s' flush level is '%s'.", logger->name().c_str(), to_string_view(level).data());
}


void set_flush_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw_log4sp_ex("Usage: sm log4sp set_flush_lvl <logger_name> <level>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);

    if (level == logger->flush_level()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' flush level is already '%s' level.", logger->name().c_str(), to_string_view(level).data());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set flush level to '%s'", logger->name().c_str(), to_string_view(level).data());
    logger->flush_on(level);
}


void version_command::execute(const std::vector<std::string> &) {
    rootconsole->ConsolePrint("SourceMod extension Log4sp version information:");
    rootconsole->ConsolePrint("    Version         " SMEXT_CONF_VERSION);
    rootconsole->ConsolePrint("    Compiled on     " SMEXT_CONF_DATESTRING " - " SMEXT_CONF_TIMESTRING);
    rootconsole->ConsolePrint("    Built from      https://github.com/F1F88/sm-ext-log4sp/commit/" SMEXT_CONF_SHA_SHORT);
}


}       // namespace log4sp
