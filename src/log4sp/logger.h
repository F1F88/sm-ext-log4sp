#pragma once

#include "spdlog/sinks/sink.h"

#include "extension.h"

#include "log4sp/common.h"
#include "log4sp/source_helper.h"


namespace Log4sp {
/**
 * spdlog 1.x 的 Logger 不便于通过继承实现自定义功能（非虚函数）
 * 出于个性化需求以及性能考虑，实现一个新的 Logger 以替代
 */
class Logger final
{
public:
    using Formatter         = spdlog::formatter;
    using LevelEnum         = spdlog::level::level_enum;
    using Level_t           = spdlog::level_t;
    using LogMsg            = spdlog::details::log_msg;
    using PatternTimeType   = spdlog::pattern_time_type;
    using SinkPtr           = spdlog::sink_ptr;
    using SinksInitList     = spdlog::sinks_init_list;
    using SourceLoc         = spdlog::source_loc;
    using string_view_t     = spdlog::string_view_t;
    using IPluginContext    = SourcePawn::IPluginContext;

    explicit Logger(std::string name) noexcept
        : m_Name(std::move(name)) {}

    template <typename It>
    Logger(std::string name, It begin, It end)
        : m_Name(std::move(name)), m_Sinks(begin, end) {}

    Logger(std::string name, SinkPtr single_sink)
        : Logger(std::move(name), {std::move(single_sink)}) {}

    Logger(std::string name, SinksInitList sinks)
        : Logger(std::move(name), sinks.begin(), sinks.end()) {}

    ~Logger() = default;

    // Log with no format string, just string message
    void Log(IPluginContext *ctx, LevelEnum lvl, string_view_t msg) const noexcept {
        assert(ctx);
        if (ShouldLog(lvl))
            SinkIt(LogMsg(m_Name, lvl, msg), SrcHelper(ctx));
    }

    void Log(const SourceLoc &loc, LevelEnum lvl, string_view_t msg) const noexcept {
        assert(!loc.empty());
        if (ShouldLog(lvl))
            SinkIt(LogMsg(loc, m_Name, lvl, msg), SrcHelper(loc));
    }

    // Log with log4sp format
    void Log(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept {
        Log(ctx, SourceLoc{}, lvl, params, param);
    }
    void Log(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;

    // Log with sourcemod format
    void LogAmxTpl(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept {
        LogAmxTpl(ctx, SourceLoc{}, lvl, params, param);
    }
    void LogAmxTpl(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;

    // special log
    void LogStackTrace(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;
    void LogStackTraceAmxTpl(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;

    void ThrowError(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;
    void ThrowErrorAmxTpl(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;

    // return true if logging is enabled for the given level.
    [[nodiscard]]
    bool ShouldLog(LevelEnum msgLevel) const noexcept {
        return msgLevel >= m_Level.load(std::memory_order_relaxed);
    }

    // return the active log level
    [[nodiscard]]
    LevelEnum GetLevel() const noexcept {
        return static_cast<LevelEnum>(m_Level.load(std::memory_order_relaxed));
    }

    // set the level of logging
    void SetLevel(LevelEnum level) noexcept {
        m_Level.store(level);
    }

    // return the name of the logger
    [[nodiscard]]
    const std::string &Name() const noexcept {
        return m_Name;
    }

    // set formatting for the sinks in this logger.
    // equivalent to
    //     set_formatter(make_unique<pattern_formatter>(pattern, type))
    // Note: each sink will get a new instance of a formatter object, replacing the old one.
    void SetPattern(std::string pattern, PatternTimeType type = PatternTimeType::local) noexcept;

    // set formatting for the sinks in this logger.
    // each sink will get a separate instance of the formatter object.
    void SetPatternFormatter(std::unique_ptr<Formatter> fmt) noexcept;

    // flush
    void Flush(IPluginContext *ctx) noexcept    { assert(ctx);          Flush(SrcHelper(ctx)); }
    void Flush(const SourceLoc &loc) noexcept   { assert(!loc.empty()); Flush(SrcHelper(loc)); }

    // return true if the given messages should be flushed
    [[nodiscard]]
    bool ShouldFlush(const LevelEnum lvl) const noexcept {
        return (lvl >= m_FlushLevel.load(std::memory_order_relaxed)) && (lvl != LevelEnum::off);
    }

    [[nodiscard]]
    LevelEnum GetFlushLevel() const noexcept {
        return static_cast<LevelEnum>(m_FlushLevel.load(std::memory_order_relaxed));
    }

    void SetFlushLevel(LevelEnum lvl) noexcept {
        m_FlushLevel.store(lvl);
    }

    // sinks
    [[nodiscard]] const std::vector<SinkPtr> &Sinks() const noexcept { return m_Sinks; }
    [[nodiscard]] std::vector<SinkPtr> &sinks() noexcept             { return m_Sinks; }
    void AddSink(SinkPtr sink) noexcept;
    void DropSink(SinkPtr sink) noexcept;

    // error handler
    void SetErrorHandler(SourceMod::IChangeableForward *handler) noexcept {
        m_ErrHelper.SetErrHandler(handler);
    }

private:
    // source 用于发生错误时获取错误发生的源码位置
    void SinkIt(const LogMsg &msg, const SrcHelper &source) const noexcept;
    void Flush(const SrcHelper &source) const noexcept;

    const std::string m_Name;
    std::vector<SinkPtr> m_Sinks;
    Level_t m_Level{LevelEnum::info};
    Level_t m_FlushLevel{LevelEnum::off};
    ErrHelper m_ErrHelper;
};


}       // namespace Log4sp
