#ifndef _LOG4SP_SINKS_CALLBACK_SINK_H_
#define _LOG4SP_SINKS_CALLBACK_SINK_H_

#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/sink.h"

#include "extension.h"


namespace log4sp {
namespace sinks {

class callback_sink final : public spdlog::sinks::sink {
public:
    callback_sink(IPluginFunction *log_function = nullptr,
                  IPluginFunction *log_post_function = nullptr,
                  IPluginFunction *flush_function = nullptr);
    ~callback_sink() override;

    callback_sink(const callback_sink &) = delete;
    callback_sink(callback_sink &&) = delete;

    callback_sink &operator=(const callback_sink &) = delete;
    callback_sink &operator=(callback_sink &&) = delete;

    void set_log_callback(IPluginFunction *log_function);
    void set_log_post_callback(IPluginFunction *log_post_function);
    void set_flush_callback(IPluginFunction *flush_function);

    void log(const spdlog::details::log_msg &msg) override;
    void flush() override;

    void set_pattern(const std::string &pattern) override {
        set_formatter(spdlog::details::make_unique<spdlog::pattern_formatter>(pattern));
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override {
        formatter_ = std::move(sink_formatter);
    }

    std::string format_pattern(const spdlog::details::log_msg &msg) {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        return spdlog::fmt_lib::to_string(formatted);
    }

private:
    std::unique_ptr<spdlog::formatter> formatter_{nullptr};
    IChangeableForward *log_callback_{nullptr};
    IChangeableForward *log_post_callback_{nullptr};
    IChangeableForward *flush_callback_{nullptr};

    void release_forwards();
};


}       // namespace sinks
}       // namespace log4sp
#endif  // _LOG4SP_SINKS_CALLBACK_SINK_H_
