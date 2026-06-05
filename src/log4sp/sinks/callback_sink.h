#pragma once

#include "spdlog/sinks/base_sink.h"

#include "extension.h"


namespace Log4sp {
namespace Sinks {

/**
 * spdlog 1.x 的 callback_sink 仅支持单回调 (在 log -> sink_it 时)
 * 且回调函数初始化完毕后无法修改，因此重新实现一个增强版
 * 初始化后仍支持修改似乎并不是一个特别好的特性，但目前没有遇到阻碍，暂时保留
 */
class CallbackSink final : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    using LogMsg = spdlog::details::log_msg;

    CallbackSink(IPluginFunction *logFunc = nullptr,
                  IPluginFunction *logPostFun = nullptr,
                  IPluginFunction *flushFunc = nullptr) noexcept {
        SetLogCallback(logFunc);
        SetLogPostCallback(logPostFun);
        SetFlushCallback(flushFunc);
    }

    ~CallbackSink() noexcept override {
        ReleaseForwards();
    }

    void SetLogCallback(IPluginFunction *logFunc) noexcept {
        if (m_LogFwd) {
            forwards->ReleaseForward(m_LogFwd);
            m_LogFwd = nullptr;
        }

        if (logFunc) {
            // void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime);
            FWDS_CREATE_EX(nullptr, ET_Ignore, 7, nullptr,
                        Param_String,    // name
                        Param_Cell,      // lvl
                        Param_String,    // msg
                        Param_String,    // file
                        Param_Cell,      // line
                        Param_String,    // func
                        Param_Cell);     // logTime
            FWD_ADD_FUNCTION(logFunc);
            m_LogFwd = fwd;
        }
    }

    void SetLogPostCallback(IPluginFunction *logPostFun) noexcept {
        if (m_LogPostFwd) {
            forwards->ReleaseForward(m_LogPostFwd);
            m_LogPostFwd = nullptr;
        }

        if (logPostFun) {
            // void (const char[] msg);
            FWDS_CREATE_EX(nullptr, ET_Ignore, 1, nullptr, Param_String);
            FWD_ADD_FUNCTION(logPostFun);
            m_LogPostFwd = fwd;
        }
    }

    void SetFlushCallback(IPluginFunction *flushFunc) noexcept {
        if (m_FlushFwd) {
            forwards->ReleaseForward(m_FlushFwd);
            m_FlushFwd = nullptr;
        }

        if (flushFunc) {
            // void ();
            FWDS_CREATE_EX(nullptr, ET_Ignore, 0, nullptr);
            FWD_ADD_FUNCTION(flushFunc);
            m_FlushFwd = fwd;
        }
    }

private:
    SourceMod::IChangeableForward *m_LogFwd{nullptr};
    SourceMod::IChangeableForward *m_LogPostFwd{nullptr};
    SourceMod::IChangeableForward *m_FlushFwd{nullptr};

    void sink_it_(const LogMsg &logMsg) noexcept override {
        if (m_LogFwd) {
            auto fwd = m_LogFwd;
            auto logTime = std::chrono::duration_cast<std::chrono::seconds>(logMsg.time.time_since_epoch());
            auto file    = logMsg.source.filename ? logMsg.source.filename : "";
            auto func    = logMsg.source.funcname ? logMsg.source.funcname : "";

            FWD_PUSH_STRING(logMsg.logger_name.data());             // name
            FWD_PUSH_CELL(logMsg.level);                            // lvl
            FWD_PUSH_STRING(logMsg.payload.data());                 // msg
            FWD_PUSH_STRING(file);                                  // file
            FWD_PUSH_CELL(logMsg.source.line);                      // line
            FWD_PUSH_STRING(func);                                  // func
            FWD_PUSH_CELL(static_cast<cell_t>(logTime.count()));    // logTime
            FWD_EXECUTE();
        }

        if (m_LogPostFwd) {
            auto fwd = m_LogPostFwd;
            std::string formatted = to_pattern(logMsg);

            FWD_PUSH_STRING(formatted.c_str());
            FWD_EXECUTE();
        }
    }

    void flush_() noexcept override {
        if (m_FlushFwd) {
            auto fwd = m_FlushFwd;
            FWD_EXECUTE();
        }
    }

    void ReleaseForwards() noexcept {
        if (m_LogFwd) {
            forwards->ReleaseForward(m_LogFwd);
            m_LogFwd = nullptr;
        }

        if (m_LogPostFwd) {
            forwards->ReleaseForward(m_LogPostFwd);
            m_LogPostFwd = nullptr;
        }

        if (m_FlushFwd) {
            forwards->ReleaseForward(m_FlushFwd);
            m_FlushFwd = nullptr;
        }
    }
};


}       // namespace Sinks
}       // namespace Log4sp
