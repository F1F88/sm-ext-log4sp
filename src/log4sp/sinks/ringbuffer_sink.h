// ref: https://github.com/gabime/spdlog/blob/v2.x/include/spdlog/sinks/ringbuffer_sink.h
#pragma once

#include <deque>

#include "spdlog/details/circular_q.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

namespace Log4sp {
namespace Sinks {

class RingBufferSink final : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
    using LogMsg        = spdlog::details::log_msg;
    using LogMsgBuffer  = spdlog::details::log_msg_buffer;

public:
    explicit RingBufferSink(std::size_t maxSize) noexcept
        : m_MaxSize{maxSize} {}
    ~RingBufferSink() override = default;

    // Returns true if a message was drained, false if the buffer is empty.
    [[nodiscard]]
    bool DrainLatest(std::function<void(const LogMsgBuffer &)> callback) noexcept {
        if (!m_Buffer.empty()) {
            callback(m_Buffer.back());
            m_Buffer.pop_back();
            return true;
        }
        return false;
    }

    // Returns true if a message was drained, false if the buffer is empty.
    [[nodiscard]]
    bool DrainOldest(std::function<void(const LogMsgBuffer &)> callback) noexcept {
        if (!m_Buffer.empty()) {
            callback(m_Buffer.front());
            m_Buffer.pop_front();
            return true;
        }
        return false;
    }

    [[nodiscard]]
    std::size_t GetSize() const noexcept {
        return m_Buffer.size();
    }

    [[nodiscard]]
    std::size_t GetMaxSize() const noexcept {
        return m_MaxSize;
    }

private:
    std::deque<LogMsgBuffer> m_Buffer;
    const std::size_t m_MaxSize;

    void sink_it_(const LogMsg &logMsg) noexcept override {
        if (m_MaxSize) {
            if (m_Buffer.size() >= m_MaxSize)
                m_Buffer.pop_front();
            m_Buffer.push_back(LogMsgBuffer(logMsg));
        }
    }

    void flush_() noexcept override {}
};


}       // namespace Sinks
}       // namespace Log4sp

