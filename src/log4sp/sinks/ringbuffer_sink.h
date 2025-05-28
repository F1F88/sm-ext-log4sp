// ref: https://github.com/gabime/spdlog/blob/v2.x/include/spdlog/sinks/ringbuffer_sink.h
#pragma once

#include "spdlog/details/circular_q.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/sinks/base_sink.h"

#include "extension.h"

namespace log4sp {
namespace sinks {

/*
 * Ring buffer sink. Holds fixed amount of log messages in memory. When the buffer is full, new
 * messages override the old ones. Useful for storing debug data in memory in case of error.
 * Example: auto rb_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128); spdlog::logger
 * logger("rb_logger", rb_sink); rb->drain([](const std::string_view msg) { process(msg);});
 */
template <typename Mutex>
class ringbuffer_sink final : public spdlog::sinks::base_sink<Mutex>  {
    using log_msg = spdlog::details::log_msg;
    using log_msg_buffer = spdlog::details::log_msg_buffer;

public:
    explicit ringbuffer_sink(size_t n_items)
        : q_{n_items} {}
    ~ringbuffer_sink() override = default;

    void drain(std::function<void(const log_msg_buffer &)> callback) {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        while (!q_.empty()) {
            callback(q_.front());
            q_.pop_front();
        }
    }

    void drain_formatted(std::function<void(std::string_view)> callback) {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        spdlog::memory_buf_t formatted;
        while (!q_.empty()) {
            formatted.clear();
            spdlog::sinks::base_sink<Mutex>::formatter_->format(q_.front(), formatted);
            callback(std::string_view{formatted.data(), formatted.size()});
            q_.pop_front();
        }
    }

private:
    spdlog::details::circular_q<log_msg_buffer> q_;

    void sink_it_(const log_msg &log_msg) override {
        q_.push_back(log_msg_buffer{log_msg});
    }

    void flush_() override {}
};

using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;
using ringbuffer_sink_st = ringbuffer_sink<spdlog::details::null_mutex>;


}       // namespace sinks
}       // namespace log4sp

