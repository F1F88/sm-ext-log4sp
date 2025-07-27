#include <regex>

#include "spdlog/fmt/xchar.h"

#include "log4sp/adapter/logger_handler.h"
#include "log4sp/command/root_console_command.h"


namespace log4sp {

[[nodiscard]] static
std::pair<std::shared_ptr<logger>, std::optional<command::result>>
arg_to_logger(const std::string &arg) noexcept {
    auto logger = logger_handler::instance().find_object(arg);
    if (!logger)
        return {nullptr, command::param_error("Logger with name \"" + arg + "\" not exists.")};
    return {logger, std::nullopt};
}

[[nodiscard]] static
spdlog::level::level_enum
arg_to_level(const std::string &arg) noexcept {
    using spdlog::level::level_enum;

    // 尝试按名字转换
    level_enum level = str_to_lvl(arg.c_str());

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


command::result list_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::fmt_lib::format;
    using spdlog::fmt_lib::join;

    std::vector<std::string> names;
    log4sp::logger_handler::instance().apply_all(
        [&names](const auto &value) {
            const auto &[handle, logger] = value;
            names.push_back(logger->name());
        }
    );

    rootconsole->ConsolePrint("%s", format("[SM] List of all logger names: [{}].", join(names, ", ")).c_str());
    return ok();
}


const auto apply_all_command::functions_ = []{
    std::unordered_map<std::string_view, std::unique_ptr<command>> map;
    map.emplace("get_lvl",         std::make_unique<get_lvl_command>());
    map.emplace("set_lvl",         std::make_unique<set_lvl_command>());
    map.emplace("set_pattern",     std::make_unique<set_pattern_command>());
    map.emplace("should_log",      std::make_unique<should_log_command>());
    map.emplace("log",             std::make_unique<log_command>());
    map.emplace("flush",           std::make_unique<flush_command>());
    map.emplace("get_flush_lvl",   std::make_unique<get_flush_lvl_command>());
    map.emplace("set_flush_lvl",   std::make_unique<set_flush_lvl_command>());
    return map;
}();

command::result apply_all_command::execute(const std::vector<std::string> &args) noexcept {
    if (args.empty()) {
        std::vector<std::string> names;
        names.reserve(functions_.size());
        for (const auto &[name, cmd] : functions_) {
            names.emplace_back(name);
        }
        return usage_error(spdlog::fmt_lib::format("Usage: sm log4sp apply_all <function_name> [arguments]\nFunction names: [{}]",
                           spdlog::fmt_lib::join(names, ", ")));
    }

    auto func_name = args[0];
    auto func = functions_.find(func_name);
    if (func == functions_.end()) {
        return param_error(spdlog::fmt_lib::format("Command function name \"{}\" not exists.", func_name));
    }

    std::vector<std::shared_ptr<logger>> loggers;
    logger_handler::instance().apply_all(
        [&loggers](const auto &value) {
            const auto &[handle, logger] = value;
            loggers.push_back(logger);
        }
    );

    auto arguments = args;
    for (auto logger : loggers) {
        arguments[0] = logger->name();

        auto [s, err] = func->second->execute(arguments);

        // 将消息替换为 apply_all 格式, 并返回 usage_error 结束命令
        if (s == status::usage_error) {
            static const std::regex match_usage_pattern("(Usage: sm log4sp [a-z_]+ <logger_name>.*)");
            static const std::regex replace_logger_name_pattern(R"( <logger_name>)");
            static const std::regex replace_prefix_pattern("(Usage: sm log4sp )");

            std::string error = err.value();
            if (std::regex_match(error, match_usage_pattern)) {
                error = std::regex_replace(error, replace_logger_name_pattern, "");
                error = std::regex_replace(error, replace_prefix_pattern, "Usage: sm log4sp apply_all ");
            }
            return usage_error(error);
        }
    }

    return ok();
}


command::result get_lvl_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::level::to_string_view;

    if (args.empty()) {
        return usage_error("Usage: sm log4sp get_lvl <logger_name>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger = result.first;
    auto level  = logger->level();

    rootconsole->ConsolePrint("[SM] Logger '%s' log level is '%s'.", logger->name().c_str(), to_string_view(level).data());
    return ok();
}


command::result set_lvl_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::level::to_string_view;

    if (args.size() < 2) {
        return usage_error("Usage: sm log4sp set_lvl <logger_name> <level>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger  = result.first;
    auto level   = arg_to_level(args[1]);
    auto new_lvl = to_string_view(level).data();
    auto old_lvl = to_string_view(logger->level()).data();

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log level '%s' to '%s'", logger->name().c_str(), old_lvl, new_lvl);
    logger->set_level(level);
    return ok();
}


command::result set_pattern_command::execute(const std::vector<std::string> &args) noexcept {
    if (args.size() < 2) {
        return usage_error("Usage: sm log4sp set_pattern <logger_name> <pattern>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger  = result.first;
    auto pattern = args[1];

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log pattern to '%s'", logger->name().c_str(), pattern.c_str());
    logger->set_pattern(pattern);
    return ok();
}


command::result should_log_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::level::to_string_view;

    if (args.size() < 2) {
        return usage_error("Usage: sm log4sp should_log <logger_name> <level>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger = result.first;
    auto level  = arg_to_level(args[1]);
    bool should = logger->should_log(level);

    rootconsole->ConsolePrint("[SM] Logger '%s' has %s '%s' log level.", logger->name().c_str(), should ? "enabled" : "disabled", to_string_view(level).data());
    return ok();
}


command::result log_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::source_loc;
    using spdlog::level::to_string_view;

    if (args.size() < 3) {
        return usage_error("Usage: sm log4sp log <logger_name> <level> <message>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger = result.first;
    auto level  = arg_to_level(args[1]);
    auto msg    = args[2];

    rootconsole->ConsolePrint("[SM] Logger '%s' will log a message '%s' with log level '%s'.", logger->name().c_str(), msg.c_str(), to_string_view(level).data());
    logger->log(source_loc(__FILE__, __LINE__, __FUNCTION__), level, msg);
    return ok();
}


command::result flush_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::source_loc;

    if (args.empty()) {
        return usage_error("Usage: sm log4sp flush <logger_name>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger = result.first;

    rootconsole->ConsolePrint("[SM] Logger '%s' will flush its contents.", logger->name().c_str());
    logger->flush(source_loc(__FILE__, __LINE__, __FUNCTION__));
    return ok();
}


command::result get_flush_lvl_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::level::to_string_view;

    if (args.empty()) {
        return usage_error("Usage: sm log4sp get_flush_lvl <logger_name>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger = result.first;
    auto level  = logger->flush_level();

    rootconsole->ConsolePrint("[SM] Logger '%s' flush level is '%s'.", logger->name().c_str(), to_string_view(level).data());
    return ok();
}


command::result set_flush_lvl_command::execute(const std::vector<std::string> &args) noexcept {
    using spdlog::level::to_string_view;

    if (args.size() < 2) {
        return usage_error("Usage: sm log4sp set_flush_lvl <logger_name> <level>");
    }

    auto result = arg_to_logger(args[0]);
    if (result.second.has_value()) {
        return result.second.value();
    }

    auto logger = result.first;
    auto level  = arg_to_level(args[1]);

    if (level == logger->flush_level()) {
        rootconsole->ConsolePrint("[SM] Logger '%s' flush level is already '%s' level.", logger->name().c_str(), to_string_view(level).data());
        return ok();
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set flush level to '%s'", logger->name().c_str(), to_string_view(level).data());
    logger->flush_on(level);
    return ok();
}


command::result version_command::execute(const std::vector<std::string> &) noexcept {
    rootconsole->ConsolePrint("SourceMod extension " SMEXT_CONF_LOGTAG " version information:");
    rootconsole->ConsolePrint("    Version         " SMEXT_CONF_VERSION);
    rootconsole->ConsolePrint("    Compiled on     " SMEXT_CONF_DATESTRING " - " SMEXT_CONF_TIMESTRING);
    rootconsole->ConsolePrint("    Built from      https://github.com/F1F88/sm-ext-log4sp/commit/" SMEXT_CONF_SHA_SHORT);
    return ok();
}


}       // namespace log4sp
