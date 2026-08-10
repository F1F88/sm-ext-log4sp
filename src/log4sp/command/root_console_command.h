
#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "log4sp/common.h"


namespace Log4sp {

#define LOG4SP_ROOT_CMD     "log4sp"

class Logger;

class Command
{
public:
    using LevelEnum = spdlog::level::level_enum;

    virtual ~Command() = default;

    /**
     * 命令模式抽象类
     *
     * 重复操作不算失败，不需要抛出异常，但可能响应一条消息
     *
     * @param args      参数列表
     * @exception       指令执行失败时抛出异常，消息为失败原因
     *                  例如：参数不匹配
     */
    virtual void Execute(const std::vector<std::string> &args) = 0;

protected:
    [[nodiscard]] std::shared_ptr<Logger> ArgToLogger(const std::string &arg);

    [[nodiscard]] LevelEnum ArgToLevel(const std::string &arg);
};


class ListCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class ApplyAllCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;

private:
    inline static const std::unordered_set<std::string> m_Functions{
        "get_lvl", "set_lvl", "set_pattern", "should_log", "log",
        "flush", "get_flush_lvl", "set_flush_lvl"};
};


class GetLvlCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class SetLvlCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class SetPatternCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class ShouldLogCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class LogCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class FlushCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class GetFlushLvlCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class SetFlushLvlCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &args) override;
};


class VersionCommand final : public Command
{
public:
    void Execute(const std::vector<std::string> &) override;
};


}       // namespace Log4sp
