#include <cassert>
#include <exception>

#include "spdlog/fmt/xchar.h"

#include "log4sp/common.h"
#include "log4sp/command/root_console_command_handler.h"


namespace Log4sp {

RootConsoleCommandHandler &RootConsoleCommandHandler::Instance()
{
    static RootConsoleCommandHandler singleInstance;
    return singleInstance;
}

void RootConsoleCommandHandler::Initialize()
{
    Instance().Initialize_();
}

void RootConsoleCommandHandler::Destroy()
{
    Instance().Destroy_();
}


void RootConsoleCommandHandler::DrawMenu()
{
    using spdlog::fmt_lib::format;
    using spdlog::fmt_lib::join;
    using spdlog::level::level_string_views;

    rootconsole->ConsolePrint(SMEXT_CONF_NAME " Menu:");
    rootconsole->ConsolePrint("Usage: sm " LOG4SP_ROOT_CMD " <function_name> [arguments]");

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


void RootConsoleCommandHandler::Execute(const std::string &cmdname, const std::vector<std::string> &args)
{
    auto iter = m_Commands.find(cmdname);
    if (iter == m_Commands.end())
        ThrowLog4spEx("Command function \"" + cmdname + "\" not found.");

    iter->second->Execute(args);
}


void RootConsoleCommandHandler::OnRootConsoleCommand(const char *cmdname, const SourceMod::ICommandArgs *args)
{
    // 0-sm  |  1-log4sp  |  2-function name  |  3-logger name  |  x-params
    int argCnt = args->ArgC();
    if (argCnt <= 2)
    {
        DrawMenu();
        return;
    }

    std::string function_name = args->Arg(2);

    std::vector<std::string> arguments;
    for (int i = 3; i < argCnt; ++i)
    {
        arguments.push_back(args->Arg(i));
    }

    try
    {
        Execute(function_name, arguments);
    }
    catch (const std::exception &ex)
    {
        rootconsole->ConsolePrint("[SM] %s", ex.what());
    }
}

RootConsoleCommandHandler::RootConsoleCommandHandler()
{
    m_Commands["list"]           = std::make_unique<ListCommand>();
    m_Commands["apply_all"]      = std::make_unique<ApplyAllCommand>();
    m_Commands["get_lvl"]        = std::make_unique<GetLvlCommand>();
    m_Commands["set_lvl"]        = std::make_unique<SetLvlCommand>();
    m_Commands["set_pattern"]    = std::make_unique<SetPatternCommand>();
    m_Commands["should_log"]     = std::make_unique<ShouldLogCommand>();
    m_Commands["log"]            = std::make_unique<LogCommand>();
    m_Commands["flush"]          = std::make_unique<FlushCommand>();
    m_Commands["get_flush_lvl"]  = std::make_unique<GetFlushLvlCommand>();
    m_Commands["set_flush_lvl"]  = std::make_unique<SetFlushLvlCommand>();
    m_Commands["version"]        = std::make_unique<VersionCommand>();
}

void RootConsoleCommandHandler::Initialize_()
{
    if (!rootconsole->AddRootConsoleCommand3(SMEXT_CONF_LOGTAG, SMEXT_CONF_NAME " command menu", this))
        ThrowLog4spEx("SM error! Could not add root console commmand \"" SMEXT_CONF_LOGTAG "\".");
}

void RootConsoleCommandHandler::Destroy_()
{
    bool result = rootconsole->RemoveRootConsoleCommand(SMEXT_CONF_LOGTAG, this);
    assert(result);
}


}       // namespace Log4sp
