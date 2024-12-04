#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_H_

#include "extension.h"

#include "log4sp/command/root_console_command.h"

namespace log4sp {
namespace command {

class root_console_command_handler final : public IRootConsoleCommand {
public:
    static root_console_command_handler &instance();

    void list_menu();

    /**
     * @brief Handles a root console menu action.
     */
    void OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args);

private:
    root_console_command_handler() {}
    ~root_console_command_handler() {}

    std::unordered_map<std::string, std::unique_ptr<command>> commands_;
};

}       // namespace command
}       // namespace log4sp
#include "log4sp/command/root_console_command_handler-inl.h"
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_HANDLER_H_
