
#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "log4sp/common.h"


namespace log4sp {

class logger;

class command {
public:
    virtual ~command() = default;

    /**
     * 命令模式抽象类
     *
     * 重复操作不算失败，不需要抛出异常，但可能响应一条消息
     *
     * @param args      参数列表
     * @exception       指令执行失败时抛出异常，消息为失败原因
     *                  例如：参数不匹配
     */
    virtual void execute(const std::vector<std::string> &args) = 0;

protected:
    [[nodiscard]] std::shared_ptr<logger> arg_to_logger(const std::string &arg);

    [[nodiscard]] level::level_enum arg_to_level(const std::string &arg);
};


class list_command final : public command {
public:
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
    void execute(const std::vector<std::string> &args) override;
};


class set_lvl_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class set_pattern_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class should_log_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class log_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class flush_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class get_flush_lvl_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class set_flush_lvl_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class version_command final : public command {
public:
    void execute(const std::vector<std::string> &) override;
};


}       // namespace log4sp
