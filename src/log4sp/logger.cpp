#include "spdlog/pattern_formatter.h"

#include "log4sp/format.h"
#include "log4sp/logger.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"


namespace Log4sp {

Logger::~Logger()
{
    while (!m_SinkHandles.empty())
    {
        DropSink(m_SinkHandles.back());
    }
}

// log with log4sp format
void Logger::Log(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    if (ShouldLog(lvl))
    {
        ErrHelper::SrcHelper source(loc, ctx);
        try
        {
            std::string msg = FormatToString(ctx, params, param);
            SinkIt(LogMsg(loc, m_Name, lvl, msg), source);
        }
        catch (const std::exception &ex)
        {
            m_ErrHelper.HandleEx(m_Name, source, ex);
            return;
        }
        catch (...)
        {
            m_ErrHelper.HandleUnknownEx(m_Name, source);
            return;
        }
    }
}

// special log
// log with stack trace
void Logger::LogStackTrace(IPluginContext *ctx, LevelEnum lvl, string_view_t msg) const noexcept
{
    assert(ctx);

    if (ShouldLog(lvl))
    {
        using spdlog::fmt_lib::format;
        Log(ctx, lvl, format("Stack trace requested: {}", msg));
        Log(ctx, lvl, format("Called from: {}", PluginSysFindPluginByCtx(ctx)->GetFilename()));
        for(const auto &info : StackTraceInfoFrom(ctx))
        {
            Log(ctx, lvl, info);
        }
    }
}

void Logger::LogStackTrace(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    if (ShouldLog(lvl))
    {
        auto src = ErrHelper::SrcHelper(ctx);
        try
        {
            std::string msg = FormatToString(ctx, params, param);
            LogStackTrace(ctx, lvl, msg);
        }
        catch (const std::exception &ex)
        {
            m_ErrHelper.HandleEx(m_Name, src, ex);
            return;
        }
        catch (...)
        {
            m_ErrHelper.HandleUnknownEx(m_Name, src);
            return;
        }
    }
}

void Logger::SetPattern(std::string pattern, PatternTimeType type) noexcept
{
    using spdlog::pattern_formatter;
    SetPatternFormatter(std::make_unique<pattern_formatter>(pattern, type));
}

void Logger::SetPatternFormatter(std::unique_ptr<Formatter> fmt) noexcept
{
    for (auto it = m_Sinks.begin(); it != m_Sinks.end(); ++it)
    {
        if (std::next(it) == m_Sinks.end())
        {
            // last element - we can move it.
            (*it)->set_formatter(std::move(fmt));
            break;  // to prevent clang-tidy warning
        }
        (*it)->set_formatter(fmt->clone());
    }
}

[[nodiscard]]
std::size_t Logger::GetSinksHandleClone(cell_t *sinks, std::size_t size, SourceMod::IdentityToken_t *owner) const
{
    // 克隆体所有权归于 native caller
    SourceMod::HandleError error;
    SourceMod::HandleSecurity security(owner, myself->GetIdentity());

    std::size_t counter = 0;
    while (counter < size && counter < m_SinkHandles.size())
    {
        auto handle = m_SinkHandles.at(counter);
        auto cloned = SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
        if (!cloned)
        {
            // 发生错误, 先清理已克隆的 sink handle 再抛出错误信息
            for (std::size_t i = 0; i < counter; ++i)
            {
                SinkHandler::Instance().FreeHandle(sinks[i], &security);
            }

            using spdlog::fmt_lib::format;
            using spdlog::throw_spdlog_ex;
            throw_spdlog_ex(
                format("Failed to clone a Sink Handle {:x} (error {})",
                    static_cast<int>(handle), static_cast<int>(error)));
        }
        sinks[counter++] = cloned;
    }
    return static_cast<cell_t>(counter);
}

[[nodiscard]]
std::size_t Logger::GetSinksLength() const noexcept
{
    return m_Sinks.size();
}

void Logger::AddSink(Handle_t handle)
{
    // Sink Handle 可以被任意读取
    SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = SinkHandler::Instance().ReadHandle(handle, &security, &error);
    if (!sink)
    {
        using spdlog::fmt_lib::format;
        using spdlog::throw_spdlog_ex;
        throw_spdlog_ex(
            format("Invalid Sink Handle {:x} (error {})",
                static_cast<int>(handle), static_cast<int>(error)));
    }

    // 克隆的 Sink 所有权归于拓展
    // 1. 外部不能获取也不能操纵这些 Sinks, 只能通过 Logger.DropSink() 关闭部分 Sink
    //    或关闭 Logger 来关闭所有 Sinks
    // 2. 不必存储 AddSink 时的 Plugin Info, 关闭 Logger 时清理 Sinks 更轻松
    // 3. 不必担心插件卸载导致克隆体被自动关闭
    IdentityToken_t *owner = myself->GetIdentity();
    auto cloned = SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
    if (!cloned)
    {
        using spdlog::fmt_lib::format;
        using spdlog::throw_spdlog_ex;
        throw_spdlog_ex(
            format("Failed to clone a Sink Handle {:x} (error {})",
                static_cast<int>(handle), static_cast<int>(error)));
    }

    m_Sinks.push_back(sink);
    m_SinkHandles.push_back(cloned);

    // HACK: CallbackSink
    if (auto callbackSink = dynamic_cast<Sinks::CallbackSink*>(sink))
        callbackSink->TryRegisterHandle(cloned);
}

void Logger::DropSink(Handle_t handle)
{
    // Sink Handle 可以被任意读取
    SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = SinkHandler::Instance().ReadHandle(handle, &security, &error);
    if (!sink)
    {
        using spdlog::fmt_lib::format;
        using spdlog::throw_spdlog_ex;
        throw_spdlog_ex(
            format("Invalid Sink Handle {:x} (error {})",
                static_cast<int>(handle), static_cast<int>(error)));
    }

    // HACK: CallbackSink
    if (auto callbackSink = dynamic_cast<Sinks::CallbackSink*>(sink))
        callbackSink->DropHandle(handle);

    // 以拓展身份关闭 Sink
    security.pOwner = myself->GetIdentity();

    // Sinks 里可能有多个同样的 Sink
    // 从后向前遍历，避免删除元素时索引变化的问题
    for (int i = static_cast<int>(m_Sinks.size() - 1); i >= 0; --i)
    {
        if (m_Sinks.at(i) == sink)
        {
            SinkHandler::Instance().FreeHandle(m_SinkHandles.at(i), &security);
            m_Sinks.erase(m_Sinks.begin() + i);
            m_SinkHandles.erase(m_SinkHandles.begin() + i);
        }
    }
}

void Logger::SinkIt(const LogMsg &msg, const ErrHelper::SrcHelper &source) const noexcept
{
    for (auto &sink : m_Sinks)
    {
        if (sink->should_log(msg.level))
        {
            try
            {
                sink->log(msg);
            }
            catch (const std::exception &ex)
            {
                m_ErrHelper.HandleEx(m_Name, source, ex);
            }
            catch (...)
            {
                m_ErrHelper.HandleUnknownEx(m_Name, source);
            }
        }
    }

    if (ShouldFlush(msg.level))
        Flush(source);
}

void Logger::Flush(const ErrHelper::SrcHelper &source) const noexcept
{
    for (auto &sink : m_Sinks)
    {
        try
        {
            sink->flush();
        }
        catch (const std::exception &ex)
        {
            m_ErrHelper.HandleEx(m_Name, source, ex);
        }
        catch (...)
        {
            m_ErrHelper.HandleUnknownEx(m_Name, source);
        }
    }
}


/**
 * SrcHelper 的设计初衷
 *  由于仅少数如 LogSrc, LogLoc 等 Log Natives 明确指定了 SourceLoc 的值
 *  其余大部分 Log Natives 的 SourceLoc 都使用默认值 (empty).
 *  这会意味着 Format, SinkIt, Flush 发生错误时 SourceLoc 值为 empty.
 *  即无法获取造成的错误的源码位置信息, 显然这是不利于排查错误的.
 *
 *  考虑到 logger 是一个单线程类, 且 Log Natives 必然包含一个有效的 ctx,
 *  因此发生错误时, 如果 SourceLoc 为空, 则可以根据 ctx 获取 Frame 信息.
 */
// ErrHelper::SrcHelper
const Logger::SourceLoc &Logger::ErrHelper::SrcHelper::Loc() const noexcept
{
    if (m_Loc.empty())
    {
        m_Loc = SourceLocFrom(m_Ctx); // 仅含 IPluginContext 构造时才可能执行
    }
    return m_Loc;
}


// ErrHelper
Logger::ErrHelper::~ErrHelper() noexcept
{
    ReleaseForward();
}

void Logger::ErrHelper::SetErrHandler(SourcePawn::IPluginFunction *func)
{
    ReleaseForward();

    if (func)
    {
        // void (const char[] origin, SourceLoc loc, const char[] msg);
        auto fwd = forwards->CreateForwardEx(nullptr,
                    SourceMod::ExecType::ET_Ignore,
                    3,
                    nullptr,
                    SourceMod::Param_String,     // name
                    SourceMod::Param_Array,      // loc
                    SourceMod::Param_String);    // msg

        using spdlog::throw_spdlog_ex;
        if (!fwd)
            throw_spdlog_ex("Could not create forward.");

        if (!fwd->AddFunction(func))
        {
            forwards->ReleaseForward(fwd);
            throw_spdlog_ex("Could not add function.");
        }
        m_CustomErrHandler = fwd;
    }
}

void Logger::ErrHelper::HandleEx(const std::string &origin, const SrcHelper &src, const std::exception &ex) const noexcept
{
    try
    {
        auto loc = CellSourceLoc(src.Loc());
        if (m_CustomErrHandler)
        {
            auto fwd = m_CustomErrHandler;

            if (auto err = fwd->PushString(origin.c_str()))
            {
                smutils->LogError(myself, "Failed to push name into ErrorHandler forward (error %d)", err);
                return;
            }

            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&loc), sizeof(loc), 0))
            {
                smutils->LogError(myself, "Failed to push loc into ErrorHandler forward (error %d)", err);
                return;
            }

            if (auto err = fwd->PushString(ex.what()))
            {
                smutils->LogError(myself, "Failed to push msg into ErrorHandler forward (error %d)", err);
                return;
            }

            if (auto err = fwd->Execute())
            {
                smutils->LogError(myself, "Failed to execute ErrorHandler forward (error %d)", err);
                return;
            }
            return;
        }
        smutils->LogError(myself, "[%s::%d] [%s] %s", FilenameFrom(loc.filename), loc.line, origin.c_str(), ex.what());
    }
    catch (const std::exception &ex)
    {
        smutils->LogError(myself, "[%s] caught exception during %s handler: %s", origin.c_str(), m_CustomErrHandler ? "custom" : "default", ex.what());
    }
    catch (...)
    {
        smutils->LogError(myself, "[%s] caught unknown exception during %s handler", origin.c_str(), m_CustomErrHandler ? "custom" : "default");
    }
}

void Logger::ErrHelper::HandleUnknownEx(const std::string &origin, const SrcHelper &src) const noexcept
{
    HandleEx(origin, src, std::runtime_error("unknown exception"));
}

void Logger::ErrHelper::ReleaseForward() noexcept
{
    if (m_CustomErrHandler)
    {
        forwards->ReleaseForward(m_CustomErrHandler);
        m_CustomErrHandler = nullptr;
    }
}


}       // namespace Log4sp
