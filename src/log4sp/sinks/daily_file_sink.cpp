#include "spdlog/details/os.h"

#include "log4sp/sinks/daily_file_sink.h"


namespace log4sp {
namespace sinks {

filename_t daily_filename_default_calculator(const filename_t &filename, const tm &now_tm) {
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}_{:04d}{:02d}{:02d}{}")),
                           basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                           ext);
};


daily_file_sink::daily_file_sink(filename_t base_filename, int rotation_hour, int rotation_minute, bool truncate, uint16_t max_files, daily_filename_calculator calculator, const file_event_handlers &event_handlers)
    : base_filename_(std::move(base_filename)), rotation_h_(rotation_hour), rotation_m_(rotation_minute), file_helper_{event_handlers},
        truncate_(truncate), max_files_(max_files), filenames_q_(), calculator_(calculator) {
    if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59) {
        throw_log4sp_ex("daily_file_sink: Invalid rotation time in ctor");
    }

    auto now = log_clock::now();
    auto filename = calculator_(base_filename_, now_tm(now));
    file_helper_.open(filename, truncate_);
    rotation_tp_ = next_rotation_tp_();

    if (max_files_ > 0) {
        init_filenames_q_();
    }
}

filename_t daily_file_sink::filename() noexcept {
    return file_helper_.filename();
}

void daily_file_sink::sink_it_(const details::log_msg &msg) {
    auto time = msg.time;
    bool should_rotate = time >= rotation_tp_;
    if (should_rotate) {
        auto filename = calculator_(base_filename_, now_tm(time));
        file_helper_.open(filename, truncate_);
        rotation_tp_ = next_rotation_tp_();
    }
    memory_buf_t formatted;
    base_sink::formatter_->format(msg, formatted);
    file_helper_.write(formatted);

    // Do the cleaning only at the end because it might throw on failure.
    if (should_rotate && max_files_ > 0) {
        delete_old_();
    }
}

void daily_file_sink::flush_() {
    file_helper_.flush();
}

void daily_file_sink::init_filenames_q_() {
    using details::os::path_exists;

    filenames_q_ = details::circular_q<filename_t>(static_cast<size_t>(max_files_));
    std::vector<filename_t> filenames;
    auto now = log_clock::now();
    while (filenames.size() < max_files_) {
        auto filename = calculator_(base_filename_, now_tm(now));
        if (!path_exists(filename)) {
            break;
        }
        filenames.emplace_back(filename);
        now -= std::chrono::hours(24);
    }
    for (auto iter = filenames.rbegin(); iter != filenames.rend(); ++iter) {
        filenames_q_.push_back(std::move(*iter));
    }
}

tm daily_file_sink::now_tm(log_clock::time_point tp) {
    time_t tnow = log_clock::to_time_t(tp);
    return details::os::localtime(tnow);
}

log_clock::time_point daily_file_sink::next_rotation_tp_() {
    auto now = log_clock::now();
    tm date = now_tm(now);
    date.tm_hour = rotation_h_;
    date.tm_min = rotation_m_;
    date.tm_sec = 0;
    auto rotation_time = log_clock::from_time_t(std::mktime(&date));
    if (rotation_time > now) {
        return rotation_time;
    }
    return {rotation_time + std::chrono::hours(24)};
}

void daily_file_sink::delete_old_() {
    using details::os::filename_to_str;
    using details::os::remove_if_exists;

    filename_t current_file = file_helper_.filename();
    if (filenames_q_.full()) {
        auto old_filename = std::move(filenames_q_.front());
        filenames_q_.pop_front();
        bool ok = remove_if_exists(old_filename) == 0;
        if (!ok) {
            filenames_q_.push_back(std::move(current_file));
            throw_log4sp_ex("Failed removing daily file " + filename_to_str(old_filename),
                            errno);
        }
    }
    filenames_q_.push_back(std::move(current_file));
}


}   // namespace sinks
}   // namespace log4sp
