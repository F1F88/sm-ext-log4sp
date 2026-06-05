// ref: https://github.com/gabime/spdlog/blob/v2.x/include/spdlog/sinks/ringbuffer_sink.h
#pragma once

#include "spdlog/details/circular_q.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/sinks/base_sink.h"

#include "extension.h"


namespace Log4sp {
namespace Sinks {

/*
 * Ring buffer sink. Holds fixed amount of log messages in memory. When the buffer is full, new
 * messages override the old ones. Useful for storing debug data in memory in case of error.
 * Example: auto rb_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(128); spdlog::logger
 * logger("rb_logger", rb_sink); rb->drain([](const std::string_view msg) { process(msg);});
 */
template <typename Mutex>
class RingBufferSink final : public spdlog::sinks::base_sink<Mutex>
{
    using LogMsg        = spdlog::details::log_msg;
    using LogMsgBuffer  = spdlog::details::log_msg_buffer;

public:
    explicit RingBufferSink(std::size_t maxSize) noexcept
        : m_Buffer{maxSize} {}
    ~RingBufferSink() override = default;

    void Drain(std::function<void(const LogMsgBuffer &)> func) noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        while (!m_Buffer.empty()) {
            func(m_Buffer.front());
            m_Buffer.pop_front();
        }
    }

    void DrainFormatted(std::function<void(std::string_view)> func) noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        spdlog::memory_buf_t formatted;
        while (!m_Buffer.empty()) {
            formatted.clear();
            spdlog::sinks::base_sink<Mutex>::formatter_->format(m_Buffer.front(), formatted);
            func(std::string_view(formatted.data(), formatted.size()));
            m_Buffer.pop_front();
        }
    }

private:
    spdlog::details::circular_q<LogMsgBuffer> m_Buffer;

    void sink_it_(const LogMsg &logMsg) noexcept override {
        m_Buffer.push_back(LogMsgBuffer(logMsg));
    }

    void flush_() noexcept override {}
};

using RingBufferSinkMT = RingBufferSink<std::mutex>;
using RingBufferSinkST = RingBufferSink<spdlog::details::null_mutex>;


}       // namespace Sinks
}       // namespace Log4sp

