#pragma once

#include <cstdio>

#include "log4sp/sinks/base_sink.h"


namespace log4sp {
namespace sinks {

class stdout_sink_base : public base_sink {
public:
    explicit stdout_sink_base(FILE *file);
    ~stdout_sink_base() override = default;

private:
    FILE *file_;
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;
#ifdef _WIN32
    HANDLE handle_;
#endif  // _WIN32
};


class server_console_sink final : public stdout_sink_base {
public:
    server_console_sink() : stdout_sink_base(stdout) {}
};


}   // namespace sinks
}   // namespace log4sp
