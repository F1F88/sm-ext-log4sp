// ref: https://github.com/gabime/spdlog/blob/v2.x/include/spdlog/sinks/daily_file_sink.h
#pragma once

#include <array>
#include <chrono>

#include "spdlog/details/circular_q.h"
#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include "log4sp/common.h"


namespace Log4sp {
namespace Sinks {

/*
 * Rotating file sink based on date.
 * If truncate != false , the created file will be truncated.
 * If max_files > 0, retain only the last max_files and delete previous.
 * Note that old log files from previous executions will not be deleted by this class,
 * rotation and deletion is only applied while the program is running.
 */
class DailyFileSink final : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
    using FileEventHandlers = spdlog::file_event_handlers;
    using FileHelper        = spdlog::details::file_helper;
    using Filename_t        = spdlog::filename_t;
    using LogClock          = spdlog::log_clock;
    using LogMsg            = spdlog::details::log_msg;

public:
    using Calculator = std::function<Filename_t(const Filename_t &, const tm &)>;

    // create daily file sink which rotates on given time
    DailyFileSink(Filename_t baseFilename,
                  int rotationHour,
                  int rotationMinute,
                  bool truncate = false,
                  uint16_t maxFiles = 0,
                  const FileEventHandlers &eventHandlers = {},
                  Calculator calculator = nullptr)
        : m_BaseFilename(std::move(baseFilename)),
          m_RotationHour(rotationHour),
          m_RotationMinute(rotationMinute),
          m_FileHelper{eventHandlers},
          m_Truncate(truncate),
          m_MaxFiles(maxFiles),
          m_FilenamesQueue(),
          m_Calculator(calculator) {
        if (rotationHour < 0 || rotationHour > 23 || rotationMinute < 0 || rotationMinute > 59)
            spdlog::throw_spdlog_ex("daily_file_sink: Invalid rotation time in ctor");

        auto now = LogClock::now();
        auto filename = CalcFilename(m_BaseFilename, NowTime(now));
        m_FileHelper.open(filename, m_Truncate);
        m_RotationTimePoints = NextRotationTimePoint();

        if (m_MaxFiles > 0) {
            InitFilenamesQueue();
        }
    }

    [[nodiscard]]
    Filename_t Filename() {
        return m_FileHelper.filename();
    }

protected:
    void sink_it_(const LogMsg &msg) override {
        auto time = msg.time;
        bool shouldRotate = time >= m_RotationTimePoints;
        if (shouldRotate) {
            auto filename = CalcFilename(m_BaseFilename, NowTime(time));
            m_FileHelper.open(filename, m_Truncate);
            m_RotationTimePoints = NextRotationTimePoint();
        }
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        m_FileHelper.write(formatted);

        // Do the cleaning only at the end because it might throw on failure.
        if (shouldRotate && m_MaxFiles > 0) {
            DeleteOld();
        }
    }

    void flush_() override { m_FileHelper.flush(); }

private:
    void InitFilenamesQueue() {
        using spdlog::details::circular_q;
        using spdlog::details::os::path_exists;

        m_FilenamesQueue = circular_q<Filename_t>(static_cast<size_t>(m_MaxFiles));
        std::vector<Filename_t> filenames;
        auto now = LogClock::now();
        while (filenames.size() < m_MaxFiles) {
            auto filename = CalcFilename(m_BaseFilename, NowTime(now));
            if (!path_exists(filename)) {
                break;
            }
            filenames.emplace_back(filename);
            now -= std::chrono::hours(24);
        }
        for (auto iter = filenames.rbegin(); iter != filenames.rend(); ++iter) {
            m_FilenamesQueue.push_back(std::move(*iter));
        }
    }

    [[nodiscard]]
    tm NowTime(LogClock::time_point tp) {
        time_t tnow = LogClock::to_time_t(tp);
        return spdlog::details::os::localtime(tnow);
    }

    [[nodiscard]]
    LogClock::time_point NextRotationTimePoint() {
        auto now = LogClock::now();
        tm date = NowTime(now);
        date.tm_hour = m_RotationHour;
        date.tm_min = m_RotationMinute;
        date.tm_sec = 0;
        auto rotation_time = LogClock::from_time_t(std::mktime(&date));
        if (rotation_time > now) {
            return rotation_time;
        }
        return {rotation_time + std::chrono::hours(24)};
    }

    // Delete the file N rotations ago.
    // Throw spdlog_ex on failure to delete the old file.
    void DeleteOld() {
        using spdlog::details::os::filename_to_str;
        using spdlog::details::os::remove_if_exists;

        Filename_t current_file = m_FileHelper.filename();
        if (m_FilenamesQueue.full()) {
            auto old_filename = std::move(m_FilenamesQueue.front());
            m_FilenamesQueue.pop_front();
            bool ok = remove_if_exists(old_filename) == 0;
            if (!ok) {
                m_FilenamesQueue.push_back(std::move(current_file));
                spdlog::throw_spdlog_ex(
                    "Failed removing daily file " +
                    filename_to_str(old_filename),
                    errno
                );
            }
        }
        m_FilenamesQueue.push_back(std::move(current_file));
    }

    Filename_t CalcFilename(const Filename_t &filename, const tm &nowTime) const {
        if (m_Calculator) {
            return m_Calculator(filename, nowTime);
        } else {
            spdlog::filename_t basename, ext;
            std::tie(basename, ext) = spdlog::details::file_helper::split_by_extension(filename);
            auto relPath = spdlog::fmt_lib::format(
                SPDLOG_FMT_STRING(
                    SPDLOG_FILENAME_T("{}_{:04d}{:02d}{:02d}{}")),
                        basename, nowTime.tm_year + 1900, nowTime.tm_mon + 1, nowTime.tm_mday, ext);

            std::array<char, PLATFORM_MAX_PATH> absPath;
            smutils->BuildPath(Path_Game, absPath.data(), sizeof(absPath), "%s", relPath.c_str());
            return spdlog::filename_t(absPath.data());
        }
    }

    Filename_t m_BaseFilename;
    int m_RotationHour;
    int m_RotationMinute;
    LogClock::time_point m_RotationTimePoints;
    FileHelper m_FileHelper;
    bool m_Truncate;
    uint16_t m_MaxFiles;
    spdlog::details::circular_q<Filename_t> m_FilenamesQueue;
    Calculator m_Calculator;
};


}       // namespace Sinks
}       // namespace Log4sp
