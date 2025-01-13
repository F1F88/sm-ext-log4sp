
#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_H_

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "extension.h"


namespace log4sp {

class logger;

class command {
public:
    virtual ~command() = default;

    /**
     * 命令模式抽象类
     *
     * 重复操作不算失败，不需要抛出异常，但可能响应一条消息
     * 例如：多次启用 backtrace
     *
     * @param args      参数列表
     * @exception       指令执行失败时抛出异常，消息为失败原因
     *                  例如：参数不匹配
     */
    virtual void execute(const std::vector<std::string> &args) = 0;

protected:
    [[nodiscard]] std::shared_ptr<logger> arg_to_logger(const std::string &arg);

    [[nodiscard]] spdlog::level::level_enum arg_to_level(const std::string &arg);
};


class list_command final : public command {
public:
    list_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class apply_all_command final : public command {
public:
    apply_all_command(std::unordered_set<std::string> functions) : functions_(functions) {}
    void execute(const std::vector<std::string> &args) override;

private:
    std::unordered_set<std::string> functions_;
};


class get_lvl_command final : public command {
public:
    get_lvl_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class set_lvl_command final : public command {
public:
    set_lvl_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class set_pattern_command final : public command {
public:
    set_pattern_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class should_log_command final : public command {
public:
    should_log_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class log_command final : public command {
public:
    log_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class flush_command final : public command {
public:
    flush_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class get_flush_lvl_command final : public command {
public:
    get_flush_lvl_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class set_flush_lvl_command final : public command {
public:
    set_flush_lvl_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class should_bt_command final : public command {
public:
    should_bt_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class enable_bt_command final : public command {
public:
    enable_bt_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class disable_bt_command final : public command {
public:
    disable_bt_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


class dump_bt_command final : public command {
public:
    dump_bt_command() = default;
    void execute(const std::vector<std::string> &args) override;
};


}       // namespace log4sp
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_H_
