#pragma once

#include "spdlog/details/file_helper.h"

#include "log4sp/sinks/base_sink.h"


namespace log4sp {
namespace sinks {

using filename_t = spdlog::filename_t;
using file_event_handlers = spdlog::file_event_handlers;

/**
 * Rotating file sink based on size
 */
class rotating_file_sink final : public base_sink {
public:
    rotating_file_sink(filename_t base_filename,
                       std::size_t max_size,
                       std::size_t max_files,
                       bool rotate_on_open = false,
                       const file_event_handlers &event_handlers = {});

    static filename_t calc_filename(const filename_t &filename, std::size_t index);
    filename_t filename();
    void rotate_now();

protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

private:
    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    void rotate_();

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    bool rename_file_(const filename_t &src_filename, const filename_t &target_filename);

    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    spdlog::details::file_helper file_helper_;
};

}   // namespace sinks
}   // namespace log4sp
