#pragma once

#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/sink.h"

#include "log4sp/common.h"


namespace log4sp {
namespace sinks {
/**
 * 虽然原生 spdlog::sinks::sink 可以满足大部分需求
 * 但没有对外暴露 formatter 接口
 * 自己实现以方便更多自定义
 */
class base_sink : public spdlog::sinks::sink {
public:
    base_sink();
    explicit base_sink(std::unique_ptr<formatter> formatter);
    ~base_sink() override = default;

    base_sink(const base_sink &) = delete;
    base_sink(base_sink &&) = delete;

    base_sink &operator=(const base_sink &) = delete;
    base_sink &operator=(base_sink &&) = delete;

    void log(const details::log_msg &log_msg) final override;
    void flush() final override;
    void set_pattern(const std::string &pattern) final override;
    void set_formatter(std::unique_ptr<formatter> sink_formatter) final override;

    [[nodiscard]] std::string to_pattern(const details::log_msg &log_msg);

protected:
    std::unique_ptr<formatter> formatter_;

    virtual void sink_it_(const details::log_msg &log_msg) = 0;
    virtual void flush_() = 0;
    virtual void set_pattern_(const std::string &pattern);
    virtual void set_formatter_(std::unique_ptr<formatter> sink_formatter);
};


}       // namespace sinks
}       // namespace log4sp
