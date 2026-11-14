// Portions of this file are derived from spdlog (https://github.com/gabime/spdlog):
// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// ref: https://github.com/gabime/spdlog/blob/v2.x/include/spdlog/sinks/dist_sink.h
#pragma once

#include <charconv>
#include <set>

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"


namespace Log4sp {
namespace Sinks {

class DistSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    using LogMsg = spdlog::details::log_msg;
    using SinkPtr = spdlog::sinks::sink*;

    DistSink() = default;
    ~DistSink() noexcept override {
        SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());

        while (!m_SinksHandle.empty()) {
            auto cloned = m_SinksHandle.back();
            auto sink = SinkHandler::Instance().ReadHandle(cloned, &security, nullptr);

            assert(sink);

            // HACK: CallbackSink
            if (auto callbackSink = dynamic_cast<Sinks::CallbackSink*>(sink))
                callbackSink->DropHandle(cloned);

            m_Sinks.pop_back();
            m_SinksHandle.pop_back();
            auto error = SinkHandler::Instance().FreeHandle(cloned, &security);
            assert(error == SourceMod::HandleError::HandleError_None);
        }
    }

    // sinks
    [[nodiscard]]
    const std::vector<SinkPtr> &Sinks() const {
        return m_Sinks;
    }

    [[nodiscard]]
    std::vector<SinkPtr> &Sinks() {
        return m_Sinks;
    }

    [[nodiscard]]
    const std::vector<Handle_t> &SinksHandle() const {
        return m_SinksHandle;
    }

    [[nodiscard]]
    std::vector<Handle_t> &SinksHandle() {
        return m_SinksHandle;
    }

protected:
    void sink_it_(const LogMsg &logMsg) override {
        for (auto &sink : m_Sinks) {
            if (sink->should_log(logMsg.level)) {
                sink->log(logMsg);
            }
        }
    }

    void flush_() override {
        for (auto &sink : m_Sinks) {
            sink->flush();
        }
    }

    std::vector<SinkPtr> m_Sinks;
    std::vector<Handle_t> m_SinksHandle;
};


}       // namespace Sinks
}       // namespace Log4sp
