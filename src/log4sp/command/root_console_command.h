
#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "log4sp/common.h"

#define LOG4SP_ROOT_CMD     "log4sp"


namespace log4sp {

class logger;

class command {
public:
    using level_enum = spdlog::level::level_enum;

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

    [[nodiscard]] level_enum arg_to_level(const std::string &arg);
};


class list_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;
};


class apply_all_command final : public command {
public:
    void execute(const std::vector<std::string> &args) override;

private:
    inline static const std::unordered_set<std::string> functions_{
        "get_lvl", "set_lvl", "set_pattern", "should_log", "log",
        "flush", "get_flush_lvl", "set_flush_lvl"};
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
