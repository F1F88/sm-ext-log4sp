#include <regex>
#include <cstdlib>

#include "spdlog/fmt/xchar.h"

#include "log4sp/adapter/logger_handler.h"

#include "log4sp/command/root_console_command.h"
#include "log4sp/command/root_console_command_handler.h"


namespace Log4sp {

std::shared_ptr<Logger> Command::ArgToLogger(const std::string &arg)
{
    // 尝试按名字查找 object
    auto logger = LoggerHandler::Instance().FindLogger(arg);
    if (!logger)
        ThrowLog4spEx("Logger with name \"" + arg + "\" not exists.");
    return logger;
}

spdlog::level::level_enum Command::ArgToLevel(const std::string &arg)
{
    // 尝试按名字转换
    LevelEnum level = StrToLvl(arg.c_str());

    // 尝试按数字转换
    if (level == LevelEnum::off)
    {
        try
        {
            int number = std::stoi(arg);
            level = NumToLvl(number);
        }
        catch (const std::exception &)
        {
            level = LevelEnum::off;
        }
    }
    return level;
}


void ListCommand::Execute(const std::vector<std::string> &args)
{
    using spdlog::fmt_lib::format;
    using spdlog::fmt_lib::join;

    std::vector<std::string> names;
    LoggerHandler::Instance().ApplyAll(
        [&names](std::shared_ptr<Logger> logger)
        {
            names.push_back(logger->Name());
        }
    );
    rootconsole->ConsolePrint("%s", format("[SM] List of all logger names: [{}].", join(names, ", ")).c_str());
}


void ApplyAllCommand::Execute(const std::vector<std::string> &args)
{
    using spdlog::fmt_lib::format;
    using spdlog::fmt_lib::join;

    if (args.empty())
        ThrowLog4spEx(format("Usage: sm " LOG4SP_ROOT_CMD " apply_all <function_name> [arguments]\nFunction names: [{}]", join(m_Functions, ", ")));

    auto funcName = args[0];
    if (m_Functions.find(funcName) == m_Functions.end())
        ThrowLog4spEx("Command function name \"" + funcName + "\" not exists.");

    std::vector<std::string> arguments = args;

    LoggerHandler::Instance().ApplyAll(
        [&funcName, &arguments](std::shared_ptr<Logger> logger)
        {
            arguments[0] = logger->Name();

            try
            {
                RootConsoleCommandHandler::Instance().Execute(funcName, arguments);
            }
            catch (const std::exception &ex)
            {
                // 如果是参数格式问题，将消息替换为 apply_all 格式
                static const std::regex match_usage_pattern("(Usage: sm " LOG4SP_ROOT_CMD " [a-z_]+ <logger_name>.*)");
                static const std::regex replace_logger_name_pattern(R"( <logger_name>)");
                static const std::regex replace_prefix_pattern("(Usage: sm " LOG4SP_ROOT_CMD " )");

                std::string msg = ex.what();
                if (std::regex_match(msg, match_usage_pattern))
                {
                    msg = std::regex_replace(msg, replace_logger_name_pattern, "");
                    msg = std::regex_replace(msg, replace_prefix_pattern, "Usage: sm " LOG4SP_ROOT_CMD " apply_all ");
                }
                ThrowLog4spEx(msg);
            }
        }
    );
}


void GetLvlCommand::Execute(const std::vector<std::string> &args)
{
    if (args.empty())
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " get_lvl <logger_name>");

    auto logger = ArgToLogger(args[0]);
    auto level  = logger->GetLevel();

    using spdlog::level::to_string_view;
    rootconsole->ConsolePrint("[SM] Logger '%s' log level is '%s'.", logger->Name().c_str(), to_string_view(level).data());
}


void SetLvlCommand::Execute(const std::vector<std::string> &args)
{
    if (args.size() < 2)
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " set_lvl <logger_name> <level>");

    auto logger = ArgToLogger(args[0]);
    auto level  = ArgToLevel(args[1]);

    using spdlog::level::to_string_view;
    if (level == logger->GetLevel())
    {
        rootconsole->ConsolePrint("[SM] Logger '%s' log level is already '%s' level.", logger->Name().c_str(), to_string_view(level).data());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log level to '%s'", logger->Name().c_str(), to_string_view(level).data());
    logger->SetLevel(level);
}


void SetPatternCommand::Execute(const std::vector<std::string> &args)
{
    if (args.size() < 2)
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " set_pattern <logger_name> <pattern>");

    auto logger  = ArgToLogger(args[0]);
    auto pattern = args[1];

    rootconsole->ConsolePrint("[SM] Logger '%s' will set log pattern to '%s'", logger->Name().c_str(), pattern.c_str());
    logger->SetPattern(pattern);
}


void ShouldLogCommand::Execute(const std::vector<std::string> &args)
{
    if (args.size() < 2)
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " should_log <logger_name> <level>");

    auto logger = ArgToLogger(args[0]);
    auto level  = ArgToLevel(args[1]);
    bool result = logger->ShouldLog(level);

    using spdlog::level::to_string_view;
    rootconsole->ConsolePrint("[SM] Logger '%s' has %s '%s' log level.", logger->Name().c_str(), result ? "enabled" : "disabled", to_string_view(level).data());
}


void LogCommand::Execute(const std::vector<std::string> &args)
{
    if (args.size() < 3)
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " log <logger_name> <level> <message>");

    auto logger = ArgToLogger(args[0]);
    auto level  = ArgToLevel(args[1]);
    auto msg    = args[2];

    using spdlog::level::to_string_view;
    rootconsole->ConsolePrint("[SM] Logger '%s' will log a message '%s' with log level '%s'.", logger->Name().c_str(), msg.c_str(), to_string_view(level).data());

    using spdlog::source_loc;
    logger->Log(source_loc(__FILE__, __LINE__, __FUNCTION__), level, msg);
}


void FlushCommand::Execute(const std::vector<std::string> &args)
{

    if (args.empty())
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " flush <logger_name>");

    auto logger = ArgToLogger(args[0]);

    rootconsole->ConsolePrint("[SM] Logger '%s' will flush its contents.", logger->Name().c_str());

    using spdlog::source_loc;
    logger->Flush(source_loc(__FILE__, __LINE__, __FUNCTION__));
}


void GetFlushLvlCommand::Execute(const std::vector<std::string> &args)
{
    if (args.empty())
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " get_flush_lvl <logger_name>");

    auto logger = ArgToLogger(args[0]);
    auto level  = logger->GetFlushLevel();

    using spdlog::level::to_string_view;
    rootconsole->ConsolePrint("[SM] Logger '%s' flush level is '%s'.", logger->Name().c_str(), to_string_view(level).data());
}


void SetFlushLvlCommand::Execute(const std::vector<std::string> &args)
{
    if (args.size() < 2)
        ThrowLog4spEx("Usage: sm " LOG4SP_ROOT_CMD " set_flush_lvl <logger_name> <level>");

    auto logger = ArgToLogger(args[0]);
    auto level  = ArgToLevel(args[1]);

    using spdlog::level::to_string_view;
    if (level == logger->GetFlushLevel())
    {
        rootconsole->ConsolePrint("[SM] Logger '%s' flush level is already '%s' level.", logger->Name().c_str(), to_string_view(level).data());
        return;
    }

    rootconsole->ConsolePrint("[SM] Logger '%s' will set flush level to '%s'", logger->Name().c_str(), to_string_view(level).data());
    logger->SetFlushLevel(level);
}


void VersionCommand::Execute(const std::vector<std::string> &)
{
    rootconsole->ConsolePrint("SourceMod extension " SMEXT_CONF_LOGTAG " version information:");
    rootconsole->ConsolePrint("    Version         " SMEXT_CONF_VERSION);
    rootconsole->ConsolePrint("    Compiled on     " SMEXT_CONF_DATESTRING " - " SMEXT_CONF_TIMESTRING);
    rootconsole->ConsolePrint("    Built from      https://github.com/F1F88/sm-ext-log4sp/commit/" SMEXT_CONF_SHA_SHORT);
}


}       // namespace Log4sp
