// ref: https://github.com/gabime/spdlog/blob/v2.x/tests/test_sink.h
#pragma once

#include <chrono>
#include <exception>
#include <mutex>
#include <thread>

#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/os.h"
#include "spdlog/sinks/base_sink.h"


namespace Log4sp {
namespace Sinks {

template <typename Mutex>
class TestSink final : public spdlog::sinks::base_sink<Mutex>
{
    using LogMsg        = spdlog::details::log_msg;
    using LogMsgBuffer  = spdlog::details::log_msg_buffer;
    using DrainFunc     = std::function<void(const LogMsgBuffer &)>;
    using DrainLineFunc = std::function<void(std::string_view)>;

public:
    [[nodiscard]]
    unsigned int GetLogCounter() noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        return m_LogCounter;
    }
    [[nodiscard]]
    unsigned int GetFlushCounter() noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        return m_FlushCounter;
    }

    void DrainMsgs(DrainFunc func) noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        for (auto msg : m_Msgs) {
            func(msg);
        }
        m_Msgs.clear();
    }
    void DrainLastMsgs(DrainFunc func) noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        if (!m_Msgs.empty()) {
            func(m_Msgs.back());
            m_Msgs.pop_back();
        }
    }

    void DrainLines(DrainLineFunc func) noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        for (auto line : m_Lines) {
            func(line);
        }
        m_Lines.clear();
    }
    void DrainLastLines(DrainLineFunc func) noexcept {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        if (!m_Lines.empty()) {
            func(m_Lines.back());
            m_Lines.pop_back();
        }
    }

    void SetLogDelay(std::chrono::milliseconds delay) noexcept { m_LogDelay = delay; }
    void SetFlushDelay(std::chrono::milliseconds delay) noexcept { m_FlushDelay = delay; }

    void SetLogException(const std::runtime_error &ex) noexcept { m_LogExceptionPtr = std::make_exception_ptr(ex); }
    void ClearLogException() noexcept { m_LogExceptionPtr = nullptr; }

    void SetFlushException(const std::runtime_error &ex) noexcept { m_FlushExceptionPtr = std::make_exception_ptr(ex); }
    void ClearFlushException() noexcept { m_FlushExceptionPtr = nullptr; }

protected:
    void sink_it_(const LogMsg &msg) override {
        if (m_LogExceptionPtr)
            std::rethrow_exception(m_LogExceptionPtr);

        m_Msgs.emplace_back(LogMsgBuffer(msg));

        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        // save the line without the eol
        auto eol_len = strlen(spdlog::details::os::default_eol);
        m_Lines.emplace_back(formatted.begin(), formatted.end() - eol_len);

        m_LogCounter++;
        std::this_thread::sleep_for(m_LogDelay);
    }

    void flush_() override {
        if (m_FlushExceptionPtr)
            std::rethrow_exception(m_FlushExceptionPtr);

        m_FlushCounter++;
        std::this_thread::sleep_for(m_FlushDelay);
    }

    unsigned int m_LogCounter   = 0u;
    unsigned int m_FlushCounter = 0u;

    std::vector<LogMsgBuffer> m_Msgs;
    std::vector<std::string> m_Lines;

    std::chrono::milliseconds m_LogDelay   = std::chrono::milliseconds::zero();
    std::chrono::milliseconds m_FlushDelay = std::chrono::milliseconds::zero();

    std::exception_ptr m_LogExceptionPtr;  // will be thrown on next log if not null
    std::exception_ptr m_FlushExceptionPtr;// will be thrown on next flush if not null
};

using TestSinkMT = TestSink<std::mutex>;
using TestSinkST = TestSink<spdlog::details::null_mutex>;

}  // namespace Sinks
}  // namespace Log4sp
