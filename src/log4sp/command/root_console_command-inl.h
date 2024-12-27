
#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_INL_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_INL_H_

#include "spdlog/fmt/xchar.h"

#include "log4sp/utils.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/proxy/logger_proxy.h"

#include "log4sp/command/root_console_command.h"


namespace log4sp {

inline std::shared_ptr<logger_proxy> command::arg_to_logger(const std::string &arg) {
    // 尝试按名字查找 handle
    Handle_t handle = logger_handler::instance().find_handle(arg);
    if (handle == BAD_HANDLE) {
        throw std::runtime_error("Logger with name '"+ arg + "' does not exists.");
    }

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    // 尝试按 handle 查找 object
    std::shared_ptr<logger_proxy> logger = logger_handler::instance().read_handle(handle, &security, &error);
    if (logger == nullptr) {
        throw std::runtime_error(spdlog::fmt_lib::format("Internal Error! Logger with name '{}' does not exists. (err: {})", arg, static_cast<int>(error)));
    }
    return logger;
}

inline spdlog::level::level_enum command::arg_to_level(const std::string &arg) {
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


inline void list_command::execute(const std::vector<std::string> &args) {
    std::vector<std::string> names = log4sp::logger_handler::instance().get_all_logger_names();

    auto msg = spdlog::fmt_lib::format("[SM] List of all logger names: [{}].", spdlog::fmt_lib::join(names, ", "));
    rootconsole->ConsolePrint("%s", msg.c_str());
}


inline void get_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Usage: sm log4sp get_lvl <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = logger->level();

    rootconsole->ConsolePrint("[SM] Logger '%s' log level is '%s'.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
}


inline void set_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("Usage: sm log4sp set_lvl <logger_name> <level>");
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


inline void set_pattern_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("Usage: sm log4sp set_pattern <logger_name> <pattern>");
    }

    auto logger  = arg_to_logger(args[0]);
    auto pattern = args[1];

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log pattern to '%s'", logger->name().c_str(), pattern.c_str());
    logger->set_pattern(pattern);
}


inline void should_log_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("Usage: sm log4sp should_log <logger_name> <level>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);
    bool result = logger->should_log(level);

    rootconsole->ConsolePrint("[SM] Logger '%s' has %s '%s' log level.", logger->name().c_str(), result ? "enabled" : "disabled", spdlog::level::to_string_view(level).data());
}


inline void log_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 3) {
        throw std::runtime_error("Usage: sm log4sp log <logger_name> <level> <message>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = arg_to_level(args[1]);
    auto msg    = args[2];

    rootconsole->ConsolePrint("[SM] Logger '%s' will log a message '%s' with log level '%s'.", logger->name().c_str(), msg.c_str(), spdlog::level::to_string_view(level).data());
    logger->log(level, msg);
}


inline void flush_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Usage: sm log4sp flush <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);

    rootconsole->ConsolePrint("[SM] Logger '%s' will flush its contents.", logger->name().c_str());
    logger->flush();
}


inline void get_flush_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Usage: sm log4sp get_flush_lvl <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);
    auto level  = logger->flush_level();

    rootconsole->ConsolePrint("[SM] Logger '%s' flush level is '%s'.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
}


inline void set_flush_lvl_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("Usage: sm log4sp set_flush_lvl <logger_name> <level>");
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


inline void should_bt_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Usage: sm log4sp should_bt <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);
    bool result = logger->should_backtrace();

    rootconsole->ConsolePrint("[SM] Logger '%s' backtrace logging is %s.", logger->name().c_str(), result ? "enabled" : "disabled");
}


inline void enable_bt_command::execute(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("Usage: sm log4sp enable_bt <logger_name> <number>");
    }

    auto logger = arg_to_logger(args[0]);
    auto number = static_cast<size_t>(atoll(args[1].c_str()));

    if (logger->should_backtrace()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' has enabled backtrace logging.", logger->name().c_str());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will enable backtrace logging. (stored %d messages)", logger->name().c_str(), number);
    logger->enable_backtrace(number);
}


inline void disable_bt_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Usage: sm log4sp disable_bt <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);

    if (!logger->should_backtrace()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' has disabled backtrace logging.", logger->name().c_str());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will disable backtrace logging.", logger->name().c_str());
    logger->disable_backtrace();
}


inline void dump_bt_command::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Usage: sm log4sp dump_bt <logger_name>");
    }

    auto logger = arg_to_logger(args[0]);

    if (!logger->should_backtrace()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' has enabled backtrace logging.", logger->name().c_str());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will dump backtrace logging message.", logger->name().c_str());
    logger->dump_backtrace();
}


}       // namespace log4sp
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_INL_H_
