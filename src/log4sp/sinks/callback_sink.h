#pragma once

#include <charconv>
#include <set>

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"


namespace Log4sp {
namespace Sinks {

class CallbackSink final : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    using LogMsg = spdlog::details::log_msg;

    CallbackSink(SourceMod::IChangeableForward *logFwd,
                 SourceMod::IChangeableForward *flushFwd,
                 SourceMod::IChangeableForward *closeFwd,
                 cell_t data) noexcept
     : m_LogFwd{logFwd},
       m_FlushFwd(flushFwd),
       m_CloseFwd(closeFwd),
       m_Data(data) {}

    ~CallbackSink() noexcept override {
        if (m_LogFwd) {
            forwards->ReleaseForward(m_LogFwd);
            m_LogFwd = nullptr;
        }

        if (m_FlushFwd) {
            forwards->ReleaseForward(m_FlushFwd);
            m_FlushFwd = nullptr;
        }

        if (m_CloseFwd) {
            OnClose();
            forwards->ReleaseForward(m_CloseFwd);
            m_CloseFwd = nullptr;
        }
    }

    [[nodiscard]]
    cell_t GetData() const noexcept        { return m_Data; }
    void   SetData(cell_t data) noexcept   { m_Data = data; }

    void TryRegisterHandle(SourceMod::Handle_t handle) noexcept {
        m_Handles.insert(handle);
    }

    void DropHandle(SourceMod::Handle_t handle) noexcept {
        m_Handles.erase(handle);
    }

private:
    void sink_it_(const LogMsg &logMsg) override {
        if (m_LogFwd) {
            using spdlog::fmt_lib::format;
            using spdlog::throw_spdlog_ex;

            auto fwd    = m_LogFwd;
            auto handle = GetHandle();
            auto owner  = myself->GetIdentity();

            SourceMod::HandleSecurity security(owner, myself->GetIdentity());
            SourceMod::HandleError error;

            auto sink = SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
            if (!sink)
                throw_spdlog_ex(format("Failed to clone a CallbackSink Handle (error {})", static_cast<int>(error)));

            if (auto err = fwd->PushCell(sink)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push sink into CallbackSink log forward (error {})", err));
            }

            std::array<char, 256> buffer;
            if (auto err = fwd->PushStringEx(buffer.data(), sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push error into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushCell(static_cast<cell_t>(buffer.max_size()))) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push maxlen into CallbackSink log forward (error {})", err));
            }

            std::array<char, 21> logTime; {
                auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(logMsg.time.time_since_epoch()).count();
                auto result = std::to_chars(logTime.data(), logTime.data() + sizeof(logTime) - 1, nanoseconds);
                if (result.ec == std::errc{})
                    *result.ptr = '\0';  // 明确写入字符串终止符
            }

            if (auto err = fwd->PushString(logTime.data())) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push logTime into CallbackSink log forward (error {})", err));
            }

            auto loc = CellSourceLoc(logMsg.source);
            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&loc), sizeof(loc), 0)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push loc into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushString(logMsg.logger_name.data())) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push name into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushCell(logMsg.level)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push lvl into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushString(logMsg.payload.data())) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push msg into CallbackSink log forward (error {})", err));
            }

            cell_t result = 0;
            if (auto err = fwd->Execute(&result)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to execute CallbackSink log forward (error {})", err));
            }

            if (auto err = SinkHandler::Instance().FreeHandle(sink, &security))
                throw_spdlog_ex(format("Failed to free a CallbackSink Handle (error {})", static_cast<int>(err)));

            if (!result)
                throw_spdlog_ex(buffer.data());
        }
    }

    void flush_() override {
        if (m_FlushFwd) {
            using spdlog::fmt_lib::format;
            using spdlog::throw_spdlog_ex;

            auto fwd    = m_FlushFwd;
            auto handle = GetHandle();
            auto owner  = myself->GetIdentity();

            SourceMod::HandleSecurity security(owner, myself->GetIdentity());
            SourceMod::HandleError error;

            auto sink = SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
            if (!sink)
                throw_spdlog_ex(format("Failed to clone a CallbackSink Handle (error {})", static_cast<int>(error)));

            if (auto err = fwd->PushCell(sink)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push sink into CallbackSink log forward (error {})", err));
            }

            std::array<char, 256> buffer;
            if (auto err = fwd->PushStringEx(buffer.data(), sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push error into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushCell(static_cast<cell_t>(buffer.max_size()))) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push maxlen into CallbackSink log forward (error {})", err));
            }

            cell_t result = 0;
            if (auto err = fwd->Execute(&result)) {
                SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to execute CallbackSink log forward (error {})", err));
            }

            if (auto err = SinkHandler::Instance().FreeHandle(sink, &security))
                throw_spdlog_ex(format("Failed to free a CallbackSink Handle (error {})", static_cast<int>(err)));

            if (!result)
                throw_spdlog_ex(buffer.data());
        }
    }

    void OnClose() const noexcept {
        if (m_CloseFwd) {
            auto fwd = m_CloseFwd;
            if (auto err = fwd->PushCell(m_Data)) {
                smutils->LogError(myself, "Failed to push data into CallbackSink close forward (error %d)", err);
                return;
            }

            if (auto err = fwd->Execute()) {
                smutils->LogError(myself, "Failed to execute CallbackSink close forward (error {})", err);
                return;
            }
        }
    }

    /**
     * HACK: CallbackSink 回调的第一个参数是 CallbackSink Handle
     *
     * Header Only 方案
     *      任何调用 Sink.Log() 和 Sink.Flush() 都通过 Handle
     *      直接将 this Handle 传递给 CallbackSink 回调即可
     *
     * Extension 问题
     *      除了 Sink.Log() 和 Sink.Flush() Native 通过 Handle
     *      Logger.Log() Native 也会调用 sink.log() 和 sink.flush() 且不通过 Handle
     *
     *      Logger 内部虽然维护着一份 m_SinkHandles 但目的是增加引用计数以延长生命周期
     *      不会传递给 sink.log() 和 sink.flush()
     *      并且 spdlog::sinks::base_sink 的所有方法都不包含 Handle 参数
     *
     * 总结
     *      CallbackSink 需要在重写的 sink_it_() 和 flush_() 中想办法自己得到一个 Handle
     *
     * 补充
     *      克隆: CloneHandle(), Sink.Clone(), Logger.AddSink()
     *      关闭: CloseHandle(), Sink.Close(), Logger.Drop(), delete
     *      回调: Sink.Log(), Sink.Flush(), Logger.Log()
     *
     * Extension 方案 1
     *      创建时在 CallbackSink 内部保存一份创建的 Handle
     * 缺点
     *      创建的 Sink Handle 通常在添加到 Logger 后就会关闭
     *      导致 CallbackSink 内部保存的 Handle 失效
     *
     * Extension 方案 2
     *      每次 CallbackSink sink_it_ 和 flush_ 时通过 SinkHandler 创建一个新的 Handle
     * 缺点
     *      存在 Double Free 致命问题
     *      sink_it_ 创建新的 Handle, 方法执行完毕关闭新 Handle 后
     *      CallbackSink 将销毁, 而旧 Handle 仍指向已销毁的 CallbackSink
     *
     * Extension 方案 3
     *      在 SinkHandler 中维护一份 sink - Handle 映射表
     *      CallbackSink 需要时从 SinkHandler 中查找一个 Handle
     * 缺点
     *      CallbackSink sink = new ...
     *      CallbackSink cloned = CloneHandle(sink);
     *      cloned.Log() 的 Handle 无法在 SinkHandler 中查到
     *      需要在 Sink.Log(), Sink.Flush() 方法中打补丁来维护映射表
     *      (为什么 SourceMod::IHandleTypeDispatch 没有 OnHandleClone 接口呢)
     *
     * Extension 方案 4
     *      CallbackSink 自己维护一份 CallbackSink - Handle 映射表
     *      CallbackSink 需要时从 CallbackSink 中查找一个 Handle
     * 缺点
     *      需要在 Sink.Log(), Sink.Flush(), Logger.AddSink(), Logger.Drop() 打补丁
     *      不在 CallbackSink 创建时记入映射表是因为通常创建的 Handle 会马上关闭
     *      而 Logger.AddSink() 克隆的 Sink Handle 生命周期相对更长, 对性能影响更小
     *      即使没销毁, 使用 Sink.Log(), Sink.Flush() 也会被补丁拦截到
     */
    SourceMod::Handle_t GetHandle() {
        SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());
        SourceMod::HandleError error;

        for (auto it = m_Handles.begin(); it != m_Handles.end();) {
            auto sink = SinkHandler::Instance().ReadHandle(*it, &security, &error);
            if (!sink) {
                it = m_Handles.erase(it);
            } else {
                return *it;
            }
        }
        spdlog::throw_spdlog_ex("Failed to get a valid CallbackSink Handle.");
    }

    SourceMod::IChangeableForward *m_LogFwd{nullptr};
    SourceMod::IChangeableForward *m_FlushFwd{nullptr};
    SourceMod::IChangeableForward *m_CloseFwd{nullptr};
    cell_t m_Data{0};
    std::set<SourceMod::Handle_t> m_Handles;
};


}       // namespace Sinks
}       // namespace Log4sp
