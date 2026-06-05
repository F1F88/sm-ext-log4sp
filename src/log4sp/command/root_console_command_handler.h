#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "extension.h"

#include "log4sp/command/root_console_command.h"


namespace Log4sp {

class RootConsoleCommandHandler final : public SourceMod::IRootConsoleCommand
{
public:
    /**
     * @brief 全局单例对象
     */
    [[nodiscard]] static RootConsoleCommandHandler &Instance();

    /**
     * @brief 用于 SDK_OnLoad 时添加控制台指令。
     *
     * @exception       添加控制台指令失败。
     * @note            需要与 destroy 配对使用。
     */
    static void Initialize();

    /**
     * @brief 用于 SDK_OnUnload 时移除控制台指令。
     *
     * @note            需要与 initialize 配对使用。
     * @note            为了避免影响其他清理工作，此方法不抛出异常。
     */
    static void Destroy();

    /**
     * @brief 绘制 log4sp 指令菜单
     */
    void DrawMenu();

    /**
     * @brief 执行命令
     *
     * @param cmdname   命令名称
     * @param args      命令所需参数
     * @exception       指令执行失败时抛出异常，消息为失败原因
     *                  例如：指令不存在，或参数不匹配
     */
    void Execute(const std::string &cmdname, const std::vector<std::string> &args);

    /**
     * @brief Handles a root console menu action.
     */
    void OnRootConsoleCommand(const char *cmdname, const SourceMod::ICommandArgs *args) override;

    RootConsoleCommandHandler(const RootConsoleCommandHandler &) = delete;
    RootConsoleCommandHandler &operator=(const RootConsoleCommandHandler &) = delete;

private:
    RootConsoleCommandHandler();

    void Initialize_();
    void Destroy_();

    std::unordered_map<std::string, std::unique_ptr<Command>> m_Commands;
};

}       // namespace Log4sp
