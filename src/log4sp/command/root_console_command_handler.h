#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "extension.h"

#include "log4sp/command/root_console_command.h"


namespace log4sp {

class root_console_command_handler final : public IRootConsoleCommand {
public:
    /**
     * @brief 全局单例对象
     */
    [[nodiscard]] static root_console_command_handler &instance();

    /**
     * @brief 用于 SDK_OnLoad 时添加控制台指令。
     *
     * @exception       添加控制台指令失败。
     * @note            需要与 destroy 配对使用。
     */
    static void initialize();

    /**
     * @brief 用于 SDK_OnUnload 时移除控制台指令。
     *
     * @note            需要与 initialize 配对使用。
     * @note            为了避免影响其他清理工作，此方法不抛出异常。
     */
    static void destroy();

    /**
     * @brief 绘制 log4sp 指令菜单
     */
    void draw_menu();

    /**
     * @brief 执行命令
     *
     * @param cmdname   命令名称
     * @param args      命令所需参数
     * @exception       指令执行失败时抛出异常，消息为失败原因
     *                  例如：指令不存在，或参数不匹配
     */
    void execute(const std::string &cmdname, const std::vector<std::string> &args);

    /**
     * @brief Handles a root console menu action.
     */
    void OnRootConsoleCommand(const char *cmdname, const ICommandArgs *args);

    root_console_command_handler(const root_console_command_handler &) = delete;
    root_console_command_handler &operator=(const root_console_command_handler &) = delete;

private:
    root_console_command_handler();

    void initialize_();
    void destroy_();

    std::unordered_map<std::string, std::unique_ptr<command>> commands_;
};

}       // namespace log4sp
