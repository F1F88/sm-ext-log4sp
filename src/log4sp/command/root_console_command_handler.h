#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_H_

#include "extension.h"


namespace log4sp {

class command;

class root_console_command_handler final : public IRootConsoleCommand {
public:
    /**
     * @brief 全局单例对象
     */
    static root_console_command_handler &instance();

    /**
     * @brief 绘制 log4sp 指令菜单
     */
    void draw_menu();

    /**
     * @brief Handles a root console menu action.
     */
    void OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args);

private:
    root_console_command_handler();
    ~root_console_command_handler() {}

    std::unordered_map<std::string, std::unique_ptr<command>> commands_;
};

}       // namespace log4sp
#include "log4sp/command/root_console_command_handler-inl.h"
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_H_
