#pragma once

#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/sink.h"


namespace log4sp {
namespace sinks {

class base_sink : public spdlog::sinks::sink {
public:
    base_sink();
    explicit base_sink(std::unique_ptr<spdlog::formatter> formatter);
    ~base_sink() override = default;

    base_sink(const base_sink &) = delete;
    base_sink(base_sink &&) = delete;

    base_sink &operator=(const base_sink &) = delete;
    base_sink &operator=(base_sink &&) = delete;

    void log(const spdlog::details::log_msg &log_msg) final override;
    void flush() final override;
    void set_pattern(const std::string &pattern) final override;
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) final override;

    std::string to_pattern(const spdlog::details::log_msg &log_msg);

private:
    std::unique_ptr<spdlog::formatter> formatter_;

    virtual void sink_it_(const spdlog::details::log_msg &log_msg) = 0;
    virtual void flush_() = 0;
    virtual void set_pattern_(const std::string &pattern);
    virtual void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter);
};


}       // namespace sinks
}       // namespace log4sp
