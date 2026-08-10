#include <cassert>

#include "spdlog/pattern_formatter.h"

#include "log4sp/format.h"
#include "log4sp/adapter/logger_handler.h"


namespace Log4sp {

// log with log4sp format
void Logger::Log(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    if (ShouldLog(lvl))
    {
        SrcHelper source(loc, ctx);
        std::string msg;

        try
        {
            msg = FormatToString(ctx, params, param);
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

        SinkIt(LogMsg(loc, m_Name, lvl, msg), source);
    }
}

// log with sourcemod format
void Logger::LogAmxTpl(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    if (ShouldLog(lvl))
    {
        SrcHelper src(loc, ctx);
        char msg[2048];
        DetectExceptions eh(ctx);

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException())
            return;

        SinkIt(LogMsg(loc, m_Name, lvl, msg), src);
    }
}

// special log
void Logger::LogStackTrace(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    if (ShouldLog(lvl))
    {
        SrcHelper src(ctx);
        std::string msg;

        try
        {
            msg = FormatToString(ctx, params, param);
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

        using spdlog::fmt_lib::format;
        SinkIt(LogMsg(m_Name, lvl, format("Stack trace requested: {}", msg)), src);
        SinkIt(LogMsg(m_Name, lvl, format("Called from: {}", PluginSysFindPluginByCtx(ctx)->GetFilename())), src);

        std::vector<std::string> messages = SrcHelper::GetStackTrace(ctx);
        for (auto &iter : messages)
        {
            SinkIt(LogMsg(m_Name, lvl, iter), src);
        }
    }
}

void Logger::LogStackTraceAmxTpl(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    if (ShouldLog(lvl))
    {
        SrcHelper source(ctx);
        char msg[2048];
        DetectExceptions eh(ctx);

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException())
            return;

        using spdlog::fmt_lib::format;
        SinkIt(LogMsg(m_Name, lvl, format("Stack trace requested: {}", msg)), source);
        SinkIt(LogMsg(m_Name, lvl, format("Called from: {}", PluginSysFindPluginByCtx(ctx)->GetFilename())), source);

        std::vector<std::string> messages = SrcHelper::GetStackTrace(ctx);
        for (auto &iter : messages)
        {
            SinkIt(LogMsg(m_Name, lvl, iter), source);
        }
    }
}

void Logger::ThrowError(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    SrcHelper source(ctx);
    std::string msg;
    try
    {
        msg = FormatToString(ctx, params, param);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        m_ErrHelper.HandleEx(m_Name, source, ex);
        return;
    }
    catch (...)
    {
        ctx->ReportError("unknown exception");
        m_ErrHelper.HandleUnknownEx(m_Name, source);
        return;
    }

    ctx->ReportError(msg.c_str());

    if (ShouldLog(lvl))
    {
        using spdlog::fmt_lib::format;
        SinkIt(LogMsg(m_Name, lvl, format("Exception reported: {}", msg)), source);
        SinkIt(LogMsg(m_Name, lvl, format("Blaming: {}", PluginSysFindPluginByCtx(ctx)->GetFilename())), source);

        std::vector<std::string> messages = SrcHelper::GetStackTrace(ctx);
        for (auto &iter : messages)
        {
            SinkIt(LogMsg(m_Name, lvl, iter), source);
        }
    }
}

void Logger::ThrowErrorAmxTpl(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept
{
    assert(ctx && params);

    char msg[2048];
    DetectExceptions eh(ctx);

    smutils->FormatString(msg, sizeof(msg), ctx, params, param);
    if (eh.HasException())
        return;

    ctx->ReportError(msg);

    if (ShouldLog(lvl))
    {
        SrcHelper source(ctx);

        using spdlog::fmt_lib::format;
        SinkIt(LogMsg(m_Name, lvl, format("Exception reported: {}", msg)), source);
        SinkIt(LogMsg(m_Name, lvl, format("Blaming: {}", PluginSysFindPluginByCtx(ctx)->GetFilename())), source);

        std::vector<std::string> messages = SrcHelper::GetStackTrace(ctx);
        for (auto &iter : messages)
        {
            SinkIt(LogMsg(m_Name, lvl, iter), source);
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

void Logger::AddSink(SinkPtr sink) noexcept
{
    m_Sinks.push_back(sink);
}

void Logger::DropSink(SinkPtr sink) noexcept
{
    m_Sinks.erase(std::remove(m_Sinks.begin(), m_Sinks.end(), sink), m_Sinks.end());
}

void Logger::SinkIt(const LogMsg &msg, const SrcHelper &source) const noexcept
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

void Logger::Flush(const SrcHelper &source) const noexcept
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


}       // namespace Log4sp
