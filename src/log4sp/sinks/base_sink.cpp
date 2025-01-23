#include <log4sp/sinks/base_sink.h>


namespace log4sp {
namespace sinks {

base_sink::base_sink()
    : formatter_{spdlog::details::make_unique<spdlog::pattern_formatter>()} {}


base_sink::base_sink(std::unique_ptr<formatter> formatter)
    : formatter_{std::move(formatter)} {}

void base_sink::log(const details::log_msg &log_msg) {
    sink_it_(log_msg);
}

void base_sink::flush() {
    flush_();
}

void base_sink::set_pattern(const std::string &pattern) {
    set_pattern_(pattern);
}

void base_sink::set_formatter(std::unique_ptr<formatter> sink_formatter) {
    set_formatter_(std::move(sink_formatter));
}

void base_sink::set_pattern_(const std::string &pattern) {
    set_formatter_(spdlog::details::make_unique<spdlog::pattern_formatter>(pattern));
}

void base_sink::set_formatter_(std::unique_ptr<formatter> sink_formatter) {
    formatter_ = std::move(sink_formatter);
}

[[nodiscard]] std::string base_sink::to_pattern(const details::log_msg &log_msg) {
    spdlog::memory_buf_t formatted;
    formatter_->format(log_msg, formatted);
    return spdlog::fmt_lib::to_string(formatted);
}


}       // namespace sinks
}       // namespace log4sp
