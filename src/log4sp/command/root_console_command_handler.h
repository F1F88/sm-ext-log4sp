#pragma once

#include "extension.h"

#include "log4sp/command/root_console_command.h"


namespace log4sp {

class root_console_command_handler final : public command,
                                           private SourceMod::IRootConsoleCommand {
public:
    /**
     * @brief 全局单例对象
     */
    [[nodiscard]] static root_console_command_handler &instance() {
        static root_console_command_handler singleInstance;
        return singleInstance;
    }

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
    static void destroy() noexcept;

    /**
     * @brief 绘制 log4sp 指令菜单
     */
    void draw_menu() noexcept;

    /**
     * @brief 执行 sm log4sp ... 指令
     *
     * @param args      命令所需参数
     * @return          命令执行结果
     */
    command::result execute(const std::vector<std::string> &args) noexcept override;

    /**
     * @brief Handles a root console menu action.
     */
    void OnRootConsoleCommand(const char *cmdname, const SourceMod::ICommandArgs *args) override;

    root_console_command_handler(const root_console_command_handler &) = delete;
    root_console_command_handler &operator=(const root_console_command_handler &) = delete;

private:
    root_console_command_handler() = default;
    ~root_console_command_handler() = default;

    void add_root_console_command_();
    void remove_root_console_command_() noexcept;

    static const std::unordered_map<std::string_view, std::unique_ptr<command>> commands_;
};

}       // namespace log4sp
