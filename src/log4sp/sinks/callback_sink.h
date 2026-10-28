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
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push sink into CallbackSink log forward (error {})", err));
            }

            std::array<char, 256> buffer;
            if (auto err = fwd->PushStringEx(buffer.data(), sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK)) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push error into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushCell(static_cast<cell_t>(buffer.max_size()))) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push maxlen into CallbackSink log forward (error {})", err));
            }

            auto logTime = Log4sp::CellLong(
                static_cast<std::int64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        logMsg.time.time_since_epoch()).count()));

            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&logTime), sizeof(logTime) / sizeof(cell_t))) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push logTime into CallbackSink log forward (error {})", err));
            }

            auto loc = CellSourceLoc(logMsg.source);
            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&loc), sizeof(loc) / sizeof(cell_t))) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push loc into CallbackSink log forward (error {})", err));
            }

            std::string name{logMsg.logger_name.data(), logMsg.logger_name.size()};
            if (auto err = fwd->PushString(name.c_str())) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push name into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushCell(logMsg.level)) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push lvl into CallbackSink log forward (error {})", err));
            }

            std::string msg{logMsg.payload.data(), logMsg.payload.size()};
            if (auto err = fwd->PushString(msg.c_str())) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push msg into CallbackSink log forward (error {})", err));
            }

            cell_t result = 0;
            if (auto err = fwd->Execute(&result)) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
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
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push sink into CallbackSink log forward (error {})", err));
            }

            std::array<char, 256> buffer;
            if (auto err = fwd->PushStringEx(buffer.data(), sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK)) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push error into CallbackSink log forward (error {})", err));
            }

            if (auto err = fwd->PushCell(static_cast<cell_t>(buffer.max_size()))) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
                throw_spdlog_ex(format("Failed to push maxlen into CallbackSink log forward (error {})", err));
            }

            cell_t result = 0;
            if (auto err = fwd->Execute(&result)) {
                (void)SinkHandler::Instance().FreeHandle(sink, &security);
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
                smutils->LogError(myself, "Failed to execute CallbackSink close forward (error %d)", err);
                return;
            }
        }
    }

    /**
     * HACK: 为什么本类要持有 m_Handles
     *
     * 契约: 回调的第一个参数必须是本 sink 自身的 Sink Handle
     * (见 include/log4sp/sinks/callback_sink.inc). Header-only 的回调入口在
     * SourcePawn 层, 调用 sink.Log() 本就持有 Handle, 直接把 this 压栈即可.
     * Extension 的回调入口是 spdlog 定义的 sink_it_/flush_, 签名里只有 log_msg,
     * 没有 Handle; Logger 的 m_SinkHandles 只为延长生命周期, 经 spdlog 基类签名
     * 传不进来, 本仓库也不 fork spdlog 改签名. 所以 CallbackSink 必须自己记住
     * 一份 Handle.
     *
     * Handle 表机制 (决定下面两个方案能否成立)
     *  - 克隆体不带 object, 只带 clone 指回 master; object 只在 master 上.
     *    ReadHandle 会顺 clone 追到 master, 故克隆体的 token 同样有效.
     *  - 失效 token 因 serial 变化永远无法解析到其他对象, 陈旧缓存只占内存.
     *  - IHandleTypeDispatch 只有 OnHandleDestroy(type, object), 拿不到 token,
     *    也没有 OnHandleClone / OnHandleClose: 插件侧的克隆与关闭不可感知.
     *
     * 方案 A: 在 HandleTypeDispatchAdapter 内部维护 object -> token 映射
     *   成立. ReadHandle 每次都能拿到 (token, object) 配对并登记; CloneHandle 可先
     *   自己 ReadHandle 一次再把克隆体登记进去 (克隆是相对冷路径); OnHandleDestroy
     *   可按 object 权威清理, 不需要惰性探测. CallbackSink 可退化成普通 spdlog
     *   sink, 也不必改动 Logger.
     *   代价: 成本从「每次回调日志、仅本类承担」挪到「每次 ReadHandle、所有 Sink
     *   类型承担」, 包括 GetLevel / ShouldLog 这类不产生日志的读取; Handle 短命
     *   导致映射频繁增删; 共享组件多一份状态.
     *
     * 方案 B: 本类自持 std::set<Handle_t>, 在少量点位打补丁    <-- 采用
     *   成本收敛在唯一需要它的 Sink 类型上, SinkHandler 保持不动. 代价是需要补丁,
     *   且因插件侧关闭不可感知必须保留惰性探测. 若日后需要的 Sink 类型变多, 把
     *   映射下沉到 adapter 是自然的下一步.
     *
     * 实现: m_Handles 不拥有任何 Handle (从不对其 FreeHandle), 只作簿记.
     *  登记 Logger::AddSink          登记其保留的克隆体
     *  登记 Sink.Log/Flush Native    登记插件传入的 params[1]
     *  摘除 Logger 析构 / DropSink 析构
     *  解析 GetHandle()              逐个 ReadHandle 探测, 就地剔除失效项
     *
     * sink_it_/flush_ 克隆解析结果后压栈、回调后释放, 使 forward 执行期间 master
     * 引用计数 >= 2, 插件回调里重入关闭其他 Handle 也不会销毁正在执行的 sink.
     *
     * 不变量
     *  1. 任何能到达 sink_it_/flush_ 的路径必须先登记 Handle, 否则 GetHandle()
     *     抛异常 (硬失败, 非降级).
     *  2. 新增会克隆 Sink 的类, 必须在其克隆点登记、在释放克隆体处摘除.
     *  3. 只释放 sink_it_/flush_ 自己克隆出来的那份 Handle, 每条路径恰好一次;
     *     m_Handles 中的项与 GetHandle() 的返回值都不释放.
     *  4. 克隆只用 CloneHandle (共享 master 引用计数), 不用 CreateHandle.
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
