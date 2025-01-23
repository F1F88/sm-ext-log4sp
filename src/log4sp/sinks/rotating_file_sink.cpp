#include <cerrno>

#include "spdlog/details/os.h"

#include "log4sp/sinks/rotating_file_sink.h"


namespace log4sp {

namespace details = spdlog::details;

namespace sinks {

rotating_file_sink::rotating_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open, const file_event_handlers &event_handlers)
    : base_filename_(std::move(base_filename)), max_size_(max_size), max_files_(max_files), file_helper_{event_handlers} {
    if (max_size == 0) {
        throw std::runtime_error{"rotating sink constructor: max_size arg cannot be zero"};
    }

    if (max_files > 200000) {
        throw std::runtime_error{"rotating sink constructor: max_files arg cannot exceed 200000"};
    }
    file_helper_.open(calc_filename(base_filename_, 0));
    current_size_ = file_helper_.size();  // expensive. called only once
    if (rotate_on_open && current_size_ > 0) {
        rotate_();
        current_size_ = 0;
    }
}

// calc filename according to index and file extension if exists.
// e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
filename_t rotating_file_sink::calc_filename(const filename_t &filename, std::size_t index) {
    if (index == 0U) {
        return filename;
    }

    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return spdlog::fmt_lib::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}.{}{}")), basename, index, ext);
}

filename_t rotating_file_sink::filename() {
    return file_helper_.filename();
}

void rotating_file_sink::rotate_now() {
    rotate_();
}

void rotating_file_sink::sink_it_(const details::log_msg &msg) {
    spdlog::memory_buf_t formatted;
    base_sink::formatter_->format(msg, formatted);
    auto new_size = current_size_ + formatted.size();

    // rotate if the new estimated file size exceeds max size.
    // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
    // we only check the real size when new_size > max_size_ because it is relatively expensive.
    if (new_size > max_size_) {
        file_helper_.flush();
        if (file_helper_.size() > 0) {
            rotate_();
            new_size = formatted.size();
        }
    }
    file_helper_.write(formatted);
    current_size_ = new_size;
}

void rotating_file_sink::flush_() {
    file_helper_.flush();
}

// Rotate files:
// log.txt -> log.1.txt
// log.1.txt -> log.2.txt
// log.2.txt -> log.3.txt
// log.3.txt -> delete
void rotating_file_sink::rotate_() {
    using details::os::filename_to_str;
    using details::os::path_exists;

    file_helper_.close();
    for (auto i = max_files_; i > 0; --i) {
        filename_t src = calc_filename(base_filename_, i - 1);
        if (!path_exists(src)) {
            continue;
        }
        filename_t target = calc_filename(base_filename_, i);

        if (!rename_file_(src, target)) {
            // if failed try again after a small delay.
            // this is a workaround to a windows issue, where very high rotation
            // rates can cause the rename to fail with permission denied (because of antivirus?).
            details::os::sleep_for_millis(100);
            if (!rename_file_(src, target)) {
                file_helper_.reopen(
                    true);  // truncate the log file anyway to prevent it to grow beyond its limit!
                current_size_ = 0;

                auto msg{spdlog::fmt_lib::format("rotating_file_sink: failed renaming {} to {}", src, target)};
                spdlog::memory_buf_t outbuf;
                spdlog::fmt_lib::format_system_error(outbuf, errno, msg.c_str());
                throw spdlog::fmt_lib::to_string(outbuf);
            }
        }
    }
    file_helper_.reopen(true);
}

// delete the target if exists, and rename the src file  to target
// return true on success, false otherwise.
SPDLOG_INLINE bool rotating_file_sink::rename_file_(const filename_t &src_filename,
                                                           const filename_t &target_filename) {
    // try to delete the target file in case it already exists.
    (void)details::os::remove(target_filename);
    return details::os::rename(src_filename, target_filename) == 0;
}

}   // namespace sinks
}   // namespace log4sp
