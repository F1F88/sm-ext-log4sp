#include "log4sp/sinks/ringbuffer_sink.h"


namespace log4sp {
namespace sinks {

void ringbuffer_sink::drain(std::function<void(const spdlog::details::log_msg_buffer &)> callback) {
    while (!q_.empty()) {
        callback(q_.front());
        q_.pop_front();
    }
}

void ringbuffer_sink::drain_formatted(std::function<void(std::string_view)> callback) {
    spdlog::memory_buf_t formatted;
    while (!q_.empty()) {
        std::string formatted = to_pattern(q_.front());
        callback(std::string_view{formatted.data(), formatted.size()});
        q_.pop_front();
    }
}

void ringbuffer_sink::sink_it_(const spdlog::details::log_msg &msg) {
    q_.push_back(spdlog::details::log_msg_buffer{msg});
}


}       // namespace sinks
}       // namespace log4sp
