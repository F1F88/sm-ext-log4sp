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


namespace log4sp {
namespace sinks {

template <typename Mutex>
class test_sink final : public spdlog::sinks::base_sink<Mutex> {
    using log_msg = spdlog::details::log_msg;
    using log_msg_buffer = spdlog::details::log_msg_buffer;
    using drain_cb = std::function<void(const log_msg_buffer &)>;
    using drain_line_cb = std::function<void(std::string_view)>;

public:
    [[nodiscard]] unsigned int get_log_counter() noexcept {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return log_counter_;
    }
    [[nodiscard]] unsigned int get_flush_counter() noexcept {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return flush_counter_;
    }

    void drain_msgs(drain_cb callback) noexcept {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        for (auto msg : msgs_) {
            callback(msg);
        }
        msgs_.clear();
    }
    void drain_last_msg(drain_cb callback) noexcept {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        if (!msgs_.empty()) {
            callback(msgs_.back());
            msgs_.pop_back();
        }
    }

    void drain_lines(drain_line_cb callback) noexcept {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        for (auto line : lines_) {
            callback(line);
        }
        lines_.clear();
    }
    void drain_last_line(drain_line_cb callback) noexcept {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        if (!lines_.empty()) {
            callback(lines_.back());
            lines_.pop_back();
        }
    }

    void set_log_delay(std::chrono::milliseconds delay) noexcept { log_delay_ = delay; }
    void set_flush_delay(std::chrono::milliseconds delay) noexcept { flush_delay_ = delay; }

    void set_log_exception(const std::runtime_error &ex) noexcept { log_exception_ptr_ = std::make_exception_ptr(ex); }
    void clear_log_exception() noexcept { log_exception_ptr_ = nullptr; }

    void set_flush_exception(const std::runtime_error &ex) noexcept { flush_exception_ptr_ = std::make_exception_ptr(ex); }
    void clear_flush_exception() noexcept { flush_exception_ptr_ = nullptr; }

protected:
    void sink_it_(const log_msg &msg) override {
        if (log_exception_ptr_) {
            std::rethrow_exception(log_exception_ptr_);
        }

        msgs_.emplace_back(log_msg_buffer{msg});

        spdlog::memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        // save the line without the eol
        auto eol_len = strlen(spdlog::details::os::default_eol);
        lines_.emplace_back(formatted.begin(), formatted.end() - eol_len);

        log_counter_++;
        std::this_thread::sleep_for(log_delay_);
    }

    void flush_() override {
        if (flush_exception_ptr_) {
            std::rethrow_exception(flush_exception_ptr_);
        }
        flush_counter_++;
        std::this_thread::sleep_for(flush_delay_);
    }

    unsigned int log_counter_{0u};
    unsigned int flush_counter_{0u};


    std::vector<log_msg_buffer> msgs_;
    std::vector<std::string> lines_;

    std::chrono::milliseconds log_delay_{std::chrono::milliseconds::zero()};
    std::chrono::milliseconds flush_delay_{std::chrono::milliseconds::zero()};

    std::exception_ptr log_exception_ptr_;  // will be thrown on next log if not null
    std::exception_ptr flush_exception_ptr_;// will be thrown on next flush if not null
};

using test_sink_mt = test_sink<std::mutex>;
using test_sink_st = test_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace spdlog
