
#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_INL_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_INL_H_

#include "spdlog/logger.h"

#include "log4sp/utils.h"
#include "log4sp/adapter/logger_handler.h"

#include "log4sp/command/root_console_command.h"


namespace log4sp {

inline std::shared_ptr<spdlog::logger> command::arg_to_logger(const ICommandArgs *args, int num) {
    const char *logger_name = args->Arg(num);

    Handle_t handle = logger_handler::instance().find_handle(logger_name);
    if (handle == BAD_HANDLE) {
        rootconsole->ConsolePrint("[SM] Logger with name '%s' does not exists.", logger_name);
        return nullptr;
    }

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    std::shared_ptr<spdlog::logger> logger = logger_handler::instance().read_handle(handle, &security, &error);
    if (logger == nullptr) {
        SPDLOG_CRITICAL("[SM] Internal Error! Logger with name '{}' does not exists. (err: {})", logger_name, static_cast<int>(error));
        return nullptr;
    }
    return logger;
}

inline spdlog::level::level_enum command::arg_to_level(const ICommandArgs *args, int num) {
    const char *level_str = args->Arg(num);

    // 尝试按名字转换
    auto level = spdlog::level::from_str(level_str);

    // 尝试按数字转换
    if (level == spdlog::level::off) {
        try {
            int number = std::stoi(level_str);
            level = cell_to_level(static_cast<cell_t>(number));
        } catch (const std::exception &) {
            level = spdlog::level::off;
        }
    }

    return level;
}


inline void get_lvl_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 3) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp get_lvl <logger_name>");
        return;
    }

    std::shared_ptr<spdlog::logger> logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto level = logger->level();
    rootconsole->ConsolePrint("[SM] Logger '%s' log level is '%s'.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
}


inline void set_lvl_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 4) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp set_lvl <logger_name> <level>");
        return;
    }

    auto logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto level  = arg_to_level(args, 4);

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log level to '%s'", logger->name().c_str(), spdlog::level::to_string_view(level).data());
    logger->set_level(level);
}


inline void set_pattern_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 4) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp set_pattern <logger_name> <pattern>");
        return;
    }

    auto logger  = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto pattern = args->Arg(4);

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log pattern to '%s'", logger->name().c_str(), pattern);
    logger->set_pattern(pattern);
}


inline void should_log_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 4) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp should_log <logger_name> <level>");
        return;
    }

    auto logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto level  = arg_to_level(args, 4);

    bool result = logger->should_log(level);
    rootconsole->ConsolePrint("[SM] Logger '%s' has %s '%s' log level.", logger->name().c_str(), result ? "enabled" : "disabled", spdlog::level::to_string_view(level).data());
}


inline void log_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 5) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp log <logger_name> <level> <message>");
        return;
    }

    auto logger  = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto level   = arg_to_level(args, 4);
    auto message = args->Arg(5);

    rootconsole->ConsolePrint("[SM] Logger '%s' will log a message '%s' with log level '%s'.", logger->name().c_str(), message, spdlog::level::to_string_view(level).data());
    logger->log(level, message);
}


inline void flush_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 3) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp flush <logger_name>");
        return;
    }

    std::shared_ptr<spdlog::logger> logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will flush its contents.", logger->name().c_str());
    logger->flush();
}


inline void get_flush_lvl_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 3) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp get_flush_lvl <logger_name>");
        return;
    }

    std::shared_ptr<spdlog::logger> logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto level = logger->flush_level();
    rootconsole->ConsolePrint("[SM] Logger '%s' flush level is '%s'.", logger->name().c_str(), spdlog::level::to_string_view(level).data());
}


inline void set_flush_lvl_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 4) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp set_flush_lvl <logger_name> <level>");
        return;
    }

    auto logger  = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    auto level   = arg_to_level(args, 4);

    rootconsole->ConsolePrint("[SM] Logger '%s' will set flush level to '%s'", logger->name().c_str(), spdlog::level::to_string_view(level).data());
    logger->flush_on(level);
}


inline void should_bt_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 3) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp should_bt <logger_name>");
        return;
    }

    auto logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    bool result = logger->should_backtrace();
    rootconsole->ConsolePrint("[SM] Logger '%s' backtrace logging is %s.", logger->name().c_str(), result ? "enabled" : "disabled");
}


inline void enable_bt_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 4) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp enable_bt <logger_name> <number>");
        return;
    }

    auto logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    const char *number_str = args->Arg(4);
    size_t number;
    try {
        number = static_cast<size_t>(std::stoi(number_str));
    } catch(const std::exception &) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp enable_bt <logger_name> <number>");
        return;
    }

    if (logger->should_backtrace()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' has enabled backtrace logging.", logger->name().c_str());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will enable backtrace logging. (stored %d messages)", logger->name().c_str(), number);
    logger->enable_backtrace(number);
}


inline void disable_bt_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 3) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp disable_bt <logger_name>");
        return;
    }

    auto logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    if (!logger->should_backtrace()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' has disabled backtrace logging.", logger->name().c_str());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will disable backtrace logging.", logger->name().c_str());
    logger->disable_backtrace();
}


inline void dump_bt_command::execute(const ICommandArgs *args) {
    if (args->ArgC() <= 3) {
        rootconsole->ConsolePrint("[SM] Usage: sm log4sp dump_bt <logger_name>");
        return;
    }

    auto logger = arg_to_logger(args, 3);
    if (logger == nullptr) {
        return;
    }

    if (!logger->should_backtrace()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' has enabled backtrace logging.", logger->name().c_str());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will dump backtrace logging message.", logger->name().c_str());
    logger->dump_backtrace();
}


}       // namespace log4sp
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_INL_H_
