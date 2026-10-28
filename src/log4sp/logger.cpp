#include "spdlog/pattern_formatter.h"

#include "log4sp/format.h"
#include "log4sp/logger.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"


namespace Log4sp {

Logger::~Logger() noexcept
{
    SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());

    while (!m_SinksHandle.empty())
    {
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

// Log with no format string, just string message
void Logger::Log(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, StringView_t msg) const noexcept
{
    bool shouldLog = ShouldLog(lvl);
    bool shouldThrow = ShouldThrow(lvl);
    if (!shouldLog && !shouldThrow)
        return;

    auto logMsg = LogMsg(loc, m_Name, lvl, msg);
    if (shouldLog)
        LogIt(logMsg, ctx);

    if (shouldThrow)
        ThrowIt(logMsg, ctx);
}

// log with log4sp format
void Logger::Log(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    bool shouldLog = ShouldLog(lvl);
    bool shouldThrow = ShouldThrow(lvl);
    if (!shouldLog && !shouldThrow)
        return;

    try
    {
        std::string msg = FormatToString(ctx, params, param);

        auto logMsg = LogMsg(loc, m_Name, lvl, msg);
        if (shouldLog)
            LogIt(logMsg, ctx);

        if (shouldThrow)
            ThrowIt(logMsg, ctx);
        return;
    }
    catch (const std::exception &ex)
    {
        m_ErrHelper.HandleEx(m_Name, ErrHelper::SrcHelper(loc, ctx), ex);

        if (shouldThrow)
            ThrowIt(LogMsg(loc, m_Name, lvl, ex.what()), ctx);
        return;
    }
    catch (...)
    {
        m_ErrHelper.HandleUnknownEx(m_Name, ErrHelper::SrcHelper(loc, ctx));

        if (shouldThrow)
            ThrowIt(LogMsg(loc, m_Name, lvl, "unknown format exception"), ctx);

        assert(false);
        return;
    }
}

// special log
// log with stack trace
void Logger::LogStackTrace(IPluginContext *ctx, LevelEnum lvl, StringView_t msg) const noexcept
{
    bool shouldLog = ShouldLog(lvl);
    bool shouldThrow = ShouldThrow(lvl);
    if (!shouldLog && !shouldThrow)
        return;

    if (shouldLog)
    {
        using spdlog::fmt_lib::format;
        LogIt(LogMsg(m_Name, lvl, format("Stack trace requested: {}", msg)), ctx);
        LogIt(LogMsg(m_Name, lvl, format("Called from: {}", PluginSysFindPluginByCtx(ctx)->GetFilename())), ctx);
        for(const auto &info : StackTraceInfoFrom(ctx))
        {
            LogIt(LogMsg(m_Name, lvl, info), ctx);
        }
    }

    if (shouldThrow)
        ThrowIt(LogMsg(m_Name, lvl, msg), ctx);
}

void Logger::LogStackTrace(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    bool shouldLog = ShouldLog(lvl);
    bool shouldThrow = ShouldThrow(lvl);
    if (!shouldLog && !shouldThrow)
        return;

    try
    {
        std::string msg = FormatToString(ctx, params, param);

        if (shouldLog)
        {
            using spdlog::fmt_lib::format;
            LogIt(LogMsg(m_Name, lvl, format("Stack trace requested: {}", msg)), ctx);
            LogIt(LogMsg(m_Name, lvl, format("Called from: {}", PluginSysFindPluginByCtx(ctx)->GetFilename())), ctx);
            for(const auto &info : StackTraceInfoFrom(ctx))
            {
                LogIt(LogMsg(m_Name, lvl, info), ctx);
            }
        }

        if (shouldThrow)
            ThrowIt(LogMsg(m_Name, lvl, msg), ctx);
        return;
    }
    catch (const std::exception &ex)
    {
        m_ErrHelper.HandleEx(m_Name, ErrHelper::SrcHelper(ctx), ex);

        if (shouldThrow)
            ThrowIt(LogMsg(m_Name, lvl, ex.what()), ctx);
        return;
    }
    catch (...)
    {
        m_ErrHelper.HandleUnknownEx(m_Name, ErrHelper::SrcHelper(ctx));

        if (shouldThrow)
            ThrowIt(LogMsg(m_Name, lvl, "unknown format exception"), ctx);

        assert(false);
        return;
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

const std::vector<Log4sp::Logger::SinkPtr> &Logger::Sinks() const noexcept
{
    return m_Sinks;
}

std::vector<Log4sp::Logger::SinkPtr> &Logger::Sinks() noexcept
{
    return m_Sinks;
}

const std::vector<Log4sp::Logger::Handle_t> &Logger::SinksHandle() const noexcept
{
    return m_SinksHandle;
}

std::vector<Log4sp::Logger::Handle_t> &Logger::SinksHandle() noexcept
{
    return m_SinksHandle;
}

void Logger::LogIt(const LogMsg &msg, IPluginContext *ctx) const noexcept
{
    assert(msg.level >= Level_t::trace);
    assert(msg.level <  Level_t::n_levels);

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
                m_ErrHelper.HandleEx(m_Name, ErrHelper::SrcHelper(msg.source, ctx), ex);
            }
            catch (...)
            {
                m_ErrHelper.HandleUnknownEx(m_Name, ErrHelper::SrcHelper(msg.source, ctx));
                assert(false);
            }
        }
    }

    if (ShouldFlush(msg.level))
        FlushIt(ErrHelper::SrcHelper(msg.source, ctx));
}

void Logger::FlushIt(const ErrHelper::SrcHelper &source) const noexcept
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
            assert(false);
        }
    }
}

void Logger::ThrowIt(const LogMsg &msg, IPluginContext *ctx) const noexcept
{
    assert(msg.level >= Level_t::trace);
    assert(msg.level <  Level_t::n_levels);

    // append short level
    spdlog::memory_buf_t output;
    output.push_back('[');
    output.push_back(to_short_c_str(msg.level)[0]);
    output.push_back(']');
    output.push_back(' ');

    // append source location if present
    if (!msg.source.empty()) {
        using spdlog::details::null_scoped_padder;
        using spdlog::details::short_filename_formatter;
        using spdlog::details::fmt_helper::append_int;
        using spdlog::details::fmt_helper::append_string_view;

        const char *filename = short_filename_formatter<null_scoped_padder>::basename(msg.source.filename);

        output.push_back('[');
        append_string_view(filename, output);
        output.push_back(':');
        append_int(msg.source.line, output);
        output.push_back(']');
        output.push_back(' ');
    }

    auto prefix = spdlog::fmt_lib::to_string(output);
    std::string message{msg.payload.data(), msg.payload.size()};
    ctx->ReportError("%s%s", prefix.c_str(), message.c_str());
}


/**
 * SrcHelper 的设计初衷
 *  由于仅少数如 LogSrc, LogLoc 等 Log Natives 明确指定了 SourceLoc 的值
 *  其余大部分 Log Natives 的 SourceLoc 都使用默认值 (empty).
 *  这会意味着 Format, LogIt, Flush 发生错误时 SourceLoc 值为 empty.
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

            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&loc), sizeof(loc) / sizeof(cell_t), 0))
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
        smutils->LogError(myself, "[%s:%d] [%s] %s", FilenameFrom(loc.filename), loc.line, origin.c_str(), ex.what());
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
