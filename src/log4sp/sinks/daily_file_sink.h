#pragma once

#include <functional>

#include "spdlog/details/circular_q.h"
#include "spdlog/details/file_helper.h"

#include "log4sp/sinks/base_sink.h"


namespace log4sp {
namespace sinks {

using daily_filename_calculator = std::function<filename_t(const filename_t &filename, const tm &now_tm)>;

filename_t daily_filename_default_calculator(const filename_t &filename, const tm &now_tm);


/*
 * Rotating file sink based on date.
 * If truncate != false , the created file will be truncated.
 * If max_files > 0, retain only the last max_files and delete previous.
 * Note that old log files from previous executions will not be deleted by this class,
 * rotation and deletion is only applied while the program is running.
 */
class daily_file_sink final : public base_sink {
public:
    // create daily file sink which rotates on given time
    daily_file_sink(filename_t base_filename,
                    int rotation_hour,
                    int rotation_minute,
                    bool truncate = false,
                    uint16_t max_files = 0,
                    daily_filename_calculator calculator = daily_filename_default_calculator,
                    const file_event_handlers &event_handlers = {});

    filename_t filename() noexcept;

protected:
    void sink_it_(const details::log_msg &msg) override;

    void flush_() override;

private:
    void init_filenames_q_();

    tm now_tm(log_clock::time_point tp);

    log_clock::time_point next_rotation_tp_();

    // Delete the file N rotations ago.
    // Throw log4sp_ex on failure to delete the old file.
    void delete_old_();

    filename_t base_filename_;
    int rotation_h_;
    int rotation_m_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    bool truncate_;
    uint16_t max_files_;
    details::circular_q<filename_t> filenames_q_;
    daily_filename_calculator calculator_{nullptr};
};


}   // namespace sinks
}   // namespace log4sp
