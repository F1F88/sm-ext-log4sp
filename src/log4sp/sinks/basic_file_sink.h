#pragma once

#include "spdlog/details/file_helper.h"

#include "extension.h"

#include "log4sp/sinks/base_sink.h"


namespace log4sp {
namespace sinks {
/*
 * Trivial file sink with single file as target
 */
class basic_file_sink final : public base_sink {
public:

    explicit basic_file_sink(const filename_t &filename,
                             bool truncate = false,
                             const file_event_handlers &event_handlers = {});
    const filename_t &filename() const;
    void truncate();

protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

private:
    details::file_helper file_helper_;
};


}   // namespace sinks
}   // namespace log4sp
