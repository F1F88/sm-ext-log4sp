#include "log4sp/sinks/basic_file_sink.h"


namespace log4sp {
namespace sinks {

basic_file_sink::basic_file_sink(const filename_t &filename, bool truncate, const file_event_handlers &event_handlers)
    : file_helper_{event_handlers} {
    file_helper_.open(filename, truncate);
}

const filename_t &basic_file_sink::filename() const {
    return file_helper_.filename();
}

void basic_file_sink::truncate() {
    file_helper_.reopen(true);
}

void basic_file_sink::sink_it_(const details::log_msg &msg) {
    memory_buf_t formatted;
    base_sink::formatter_->format(msg, formatted);
    file_helper_.write(formatted);
}

void basic_file_sink::flush_() {
    file_helper_.flush();
}


}   // namespace sinks
}   // namespace log4sp
