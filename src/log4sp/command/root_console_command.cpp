#include <regex>
#include <stdlib.h>

#include "spdlog/fmt/xchar.h"

#include "log4sp/utils.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/logger.h"

#include "log4sp/command/root_console_command.h"
#include "log4sp/command/root_console_command_handler.h"


namespace log4sp {

std::shared_ptr<logger> command::arg_to_logger(const std::string &arg) {
    // 尝试按名字查找 object
    auto logger = logger_handler::instance().find_logger(arg);
    if (logger == nullptr) {
        throw std::runtime_error{spdlog::fmt_lib::format("Logger with name \"{}\" not exists.", arg)};
    }
    return logger;
}

spdlog::level::level_enum command::arg_to_level(const std::string &arg) {
    // 尝试按名字转换
    auto level = spdlog::level::from_str(arg);

    // 尝试按数字转换
    if (level == spdlog::level::off) {
        try {
            int number = std::stoi(arg);
            level = cell_to_level(static_cast<cell_t>(number));
        } catch (const std::exception &) {
            level = spdlog::level::off;
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

    auto msg = spdlog::fmt_lib::format("[SM] List of all logger names: [{}].", spdlog::fmt_lib::join(names, ", "));
    rootconsole->ConsolePrint("%s", msg.c_str());
}


void apply_all_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error{
            spdlog::fmt_lib::format(
                "Usage: sm log4sp apply_all <function_name> [arguments]\nAvailable function names: [{}]",
                spdlog::fmt_lib::join(functions_, ", "))};
    }

    const std::string function_name = args[0];
    auto iter = functions_.find(function_name);
    if (iter == functions_.end()) {
        throw std::runtime_error{spdlog::fmt_lib::format("Command function \"{}\" not found.", function_name)};
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
                throw std::runtime_error{msg};
            }
        }
    );
}


void get_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error{"Usage: sm log4sp get_lvl <logger_name>"};
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = logger->level();

    rootconsole->ConsolePrint("[SM] Logger '%s' log level is '%s'.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
}


void set_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error{"Usage: sm log4sp set_lvl <logger_name> <level>"};
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);

    if (level == logger->level()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' log level is already '%s' level.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log level to '%s'", logger->name().c_str(), spdlog::level::to_string_view(level).data());
    logger->set_level(level);
}


void set_pattern_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error{"Usage: sm log4sp set_pattern <logger_name> <pattern>"};
    }

    auto logger  = arg_to_logger(args[0]);
    auto pattern = args[1];

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log pattern to '%s'", logger->name().c_str(), pattern.c_str());
    logger->set_pattern(pattern);
}


void should_log_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error{"Usage: sm log4sp should_log <logger_name> <level>"};
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);
    bool result = logger->should_log(level);

    rootconsole->ConsolePrint("[SM] Logger '%s' has %s '%s' log level.", logger->name().c_str(), result ? "enabled" : "disabled", spdlog::level::to_string_view(level).data());
}


void log_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 3) {
        throw std::runtime_error{"Usage: sm log4sp log <logger_name> <level> <message>"};
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);
    auto msg    = args[2];

    rootconsole->ConsolePrint("[SM] Logger '%s' will log a message '%s' with log level '%s'.", logger->name().c_str(), msg.c_str(), spdlog::level::to_string_view(level).data());
    logger->log({__FILE__, __LINE__, __FUNCTION__}, level, msg, nullptr);
}


void flush_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error{"Usage: sm log4sp flush <logger_name>"};
    }

    auto logger = arg_to_logger(args[0]);

    rootconsole->ConsolePrint("[SM] Logger '%s' will flush its contents.", logger->name().c_str());
    logger->flush({__FILE__, __LINE__, __FUNCTION__}, nullptr);
}


void get_flush_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error{"Usage: sm log4sp get_flush_lvl <logger_name>"};
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = logger->flush_level();

    rootconsole->ConsolePrint("[SM] Logger '%s' flush level is '%s'.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
}


void set_flush_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error{"Usage: sm log4sp set_flush_lvl <logger_name> <level>"};
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);

    if (level == logger->flush_level()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' flush level is already '%s' level.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set flush level to '%s'", logger->name().c_str(), spdlog::level::to_string_view(level).data());
    logger->flush_on(level);
}


}       // namespace log4sp
