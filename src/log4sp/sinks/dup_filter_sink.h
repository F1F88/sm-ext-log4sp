// Portions of this file are derived from spdlog (https://github.com/gabime/spdlog):
// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// ref: https://github.com/gabime/spdlog/blob/v2.x/include/spdlog/sinks/dup_filter_sink.h
#pragma once

#include "log4sp/sinks/dist_sink.h"


namespace Log4sp {
namespace Sinks {

class DupFilterSink final : public DistSink
{
public:
    using LogMsg = spdlog::details::log_msg;
    using SinkPtr = spdlog::sinks::sink*;

    template <class Rep, class Period>
    DupFilterSink(std::chrono::duration<Rep, Period> maxSkipDuration)
        : m_MaxSkipDuration{maxSkipDuration} {}

private:
    void sink_it_(const LogMsg &logMsg) override {
        bool filtered = Filter(logMsg);
        if (!filtered) {
            m_SkipCounter += 1;
            m_SkippedMsgLogLevel = logMsg.level;
            return;
        }

        // log the "skipped.." message
        if (m_SkipCounter > 0) {
            char buf[64];
            auto msg_size = ::snprintf(buf, sizeof(buf), "Skipped %u duplicate messages.", static_cast<unsigned>(m_SkipCounter));
            if (msg_size > 0 && static_cast<size_t>(msg_size) < sizeof(buf)) {
                LogMsg skippedMsg{logMsg.source, logMsg.logger_name, m_SkippedMsgLogLevel,
                                  spdlog::string_view_t{buf, static_cast<size_t>(msg_size)}};
                DistSink::sink_it_(skippedMsg);
            }
        }

        // log current message
        DistSink::sink_it_(logMsg);
        m_LastMsgTime = logMsg.time;
        m_SkipCounter = 0;
        m_LastMsgPayload.assign(logMsg.payload.data(), logMsg.payload.data() + logMsg.payload.size());
    }

    // return whether the log msg should be displayed (true) or skipped (false)
    bool Filter(const LogMsg &logMsg) {
        auto filter_duration = logMsg.time - m_LastMsgTime;
        return (filter_duration > m_MaxSkipDuration) || (logMsg.payload != m_LastMsgPayload);
    }

    // The SourcePawn API specifies max skip duration in milliseconds; storing
    // milliseconds keeps the constructor conversion lossless for any input.
    std::chrono::milliseconds m_MaxSkipDuration;
    std::chrono::system_clock::time_point m_LastMsgTime;
    std::string m_LastMsgPayload;
    size_t m_SkipCounter{0};
    spdlog::level::level_enum m_SkippedMsgLogLevel{spdlog::level::level_enum::off};
};


}       // namespace Sinks
}       // namespace Log4sp
