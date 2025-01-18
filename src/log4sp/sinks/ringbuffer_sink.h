#pragma once

#include "spdlog/details/circular_q.h"
#include "spdlog/details/log_msg_buffer.h"

#include "extension.h"

#include "log4sp/sinks/base_sink.h"

namespace log4sp {
namespace sinks {

class ringbuffer_sink final : public base_sink {
public:
    explicit ringbuffer_sink(size_t n_items)
        : q_{n_items} {}
    ~ringbuffer_sink() override = default;

    void drain(std::function<void(const spdlog::details::log_msg_buffer &)> callback);
    void drain_formatted(std::function<void(std::string_view)> callback);

private:
    spdlog::details::circular_q<spdlog::details::log_msg_buffer> q_;

    void sink_it_(const spdlog::details::log_msg &log_msg) override;
    void flush_() override {}
};


}       // namespace sinks
}       // namespace log4sp
