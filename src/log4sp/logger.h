#pragma once

#include "spdlog/sinks/sink.h"

#include "log4sp/common.h"


namespace Log4sp {
/**
 * spdlog 1.x 的 logger 不便于通过继承实现自定义功能（非虚函数）
 * 出于个性化需求以及性能考虑，实现一个新的 logger 以替代
 */
class Logger final
{
public:
    using Formatter         = spdlog::formatter;
    using LevelEnum         = spdlog::level::level_enum;
    using Level_t           = LevelEnum;
    using LogMsg            = spdlog::details::log_msg;
    using PatternTimeType   = spdlog::pattern_time_type;
    using SinkPtr           = spdlog::sinks::sink*;
    using SinksInitList     = std::initializer_list<SinkPtr>;
    using SourceLoc         = spdlog::source_loc;
    using string_view_t     = spdlog::string_view_t;
    using Handle_t          = SourceMod::Handle_t;
    using IdentityToken_t   = SourceMod::IdentityToken_t;
    using IPluginContext    = SourcePawn::IPluginContext;
    using IPluginFunction   = SourcePawn::IPluginFunction;

    explicit Logger(std::string name) noexcept
        : m_Name(std::move(name)) {}

    ~Logger();

    // Log with no format string, just string message
    void Log(IPluginContext *ctx, LevelEnum lvl, string_view_t msg) const noexcept {
        assert(ctx);
        if (ShouldLog(lvl))
            SinkIt(LogMsg(m_Name, lvl, msg), ErrHelper::SrcHelper(ctx));
    }

    void Log(const SourceLoc &loc, LevelEnum lvl, string_view_t msg) const noexcept {
        assert(!loc.empty());
        if (ShouldLog(lvl))
            SinkIt(LogMsg(loc, m_Name, lvl, msg), ErrHelper::SrcHelper(loc));
    }

    // Log with format
    void Log(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept {
        Log(ctx, SourceLoc{}, lvl, params, param);
    }
    void Log(IPluginContext *ctx, const SourceLoc &loc, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;

    // special log
    // log with stack trace
    void LogStackTrace(IPluginContext *ctx, LevelEnum lvl, string_view_t msg) const noexcept;
    void LogStackTrace(IPluginContext *ctx, LevelEnum lvl, const cell_t *params, unsigned int param) const noexcept;

    // return true if logging is enabled for the given level.
    [[nodiscard]]
    bool ShouldLog(LevelEnum lvl) const noexcept {
        return lvl >= m_Level;
    }

    // return the active log level
    [[nodiscard]]
    LevelEnum GetLevel() const noexcept {
        return m_Level;
    }

    // set the level of logging
    void SetLevel(LevelEnum lvl) noexcept {
        m_Level = lvl;
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
    void Flush(IPluginContext *ctx) noexcept    { assert(ctx);          Flush(ErrHelper::SrcHelper(ctx)); }
    void Flush(const SourceLoc &loc) noexcept   { assert(!loc.empty()); Flush(ErrHelper::SrcHelper(loc)); }

    // return true if the given messages should be flushed
    [[nodiscard]]
    bool ShouldFlush(const LevelEnum lvl) const noexcept {
        return (lvl >= m_FlushLevel) && (lvl != LevelEnum::off);
    }

    [[nodiscard]]
    LevelEnum GetFlushLevel() const noexcept {
        return m_FlushLevel;
    }

    void SetFlushLevel(LevelEnum lvl) noexcept {
        m_FlushLevel = lvl;
    }

    // sinks
    [[nodiscard]]
    std::size_t GetSinksHandleClone(cell_t *sinks, std::size_t size, IdentityToken_t *owner) const;
    [[nodiscard]]
    std::size_t GetSinksLength() const noexcept;

    void AddSink(Handle_t sink);
    void DropSink(Handle_t sink);

    // error handler
    void SetErrorHandler(IPluginFunction *handler) {
        m_ErrHelper.SetErrHandler(handler);
    }

private:
    class ErrHelper final
    {
    public:
        class SrcHelper final
        {
        public:
            // 若 SourceLoc == empty 则 ctx 必须 != nullptr （反之亦然）
            constexpr SrcHelper(IPluginContext *ctx) noexcept : m_Ctx(ctx) {}
            constexpr SrcHelper(const SourceLoc &loc) noexcept : m_Loc(loc) {}
            constexpr SrcHelper(const SourceLoc &loc, IPluginContext *ctx) noexcept : m_Loc(loc), m_Ctx(ctx) {
                assert(ctx || !loc.empty());
            }

            [[nodiscard]]
            const SourceLoc &Loc() const noexcept;

        private:
            mutable SourceLoc m_Loc;
            IPluginContext *m_Ctx{nullptr};
        };

    public:
        ErrHelper() noexcept = default;
        ~ErrHelper() noexcept;

        void SetErrHandler(IPluginFunction *function);
        void HandleEx(const std::string &origin, const SrcHelper &src, const std::exception &ex) const noexcept;
        void HandleUnknownEx(const std::string &origin, const SrcHelper &src) const noexcept;

    private:
        void ReleaseForward() noexcept;

        SourceMod::IChangeableForward *m_CustomErrHandler{nullptr};
    };

    // source 用于发生错误时获取错误发生的源码位置
    void SinkIt(const LogMsg &msg, const ErrHelper::SrcHelper &source) const noexcept;
    void Flush(const ErrHelper::SrcHelper &source) const noexcept;

    const std::string m_Name;
    std::vector<SinkPtr> m_Sinks;
    std::vector<Handle_t> m_SinkHandles;
    Level_t m_Level{LevelEnum::info};
    Level_t m_FlushLevel{LevelEnum::off};
    ErrHelper m_ErrHelper;
};


}       // namespace Log4sp
