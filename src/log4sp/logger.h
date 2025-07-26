#pragma once

#include "spdlog/sinks/sink.h"

#include "log4sp/common.h"


namespace log4sp {
/**
 * spdlog 1.x 的 logger 不便于通过继承实现自定义功能（非虚函数）
 * 出于个性化需求以及性能考虑，实现一个新的 logger 以替代
 */
class logger final {
public:
    using formatter         = spdlog::formatter;
    using level_enum        = spdlog::level::level_enum;
    using level_t           = spdlog::level_t;
    using log_msg           = spdlog::details::log_msg;
    using pattern_time_type = spdlog::pattern_time_type;
    using sink_ptr          = spdlog::sink_ptr;
    using sinks_init_list   = spdlog::sinks_init_list;
    using source_loc        = spdlog::source_loc;
    using string_view_t     = spdlog::string_view_t;
    using plugin_ctx        = SourcePawn::IPluginContext;

    explicit logger(std::string name)
        : name_(std::move(name)) {}

    template <typename It>
    logger(std::string name, It begin, It end)
        : name_(std::move(name)), sinks_(begin, end) {}

    logger(std::string name, sink_ptr single_sink)
        : logger(std::move(name), {std::move(single_sink)}) {}

    logger(std::string name, sinks_init_list sinks)
        : logger(std::move(name), sinks.begin(), sinks.end()) {}

    ~logger() = default;

    // log with no format string, just string message
    void log(plugin_ctx *ctx, level_enum lvl, string_view_t msg) const noexcept {
        assert(ctx);
        if (should_log(lvl)) {
            sink_it_(log_msg(name_, lvl, msg), err_helper::src_helper(ctx));
        }
    }
    void log(const source_loc &loc, level_enum lvl, string_view_t msg) const noexcept {
        assert(!loc.empty());
        if (should_log(lvl)) {
            sink_it_(log_msg(loc, name_, lvl, msg), err_helper::src_helper(loc));
        }
    }

    // log with log4sp format
    void log(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
        log(ctx, source_loc{}, lvl, params, param);
    }
    void log(plugin_ctx *ctx, const source_loc &loc, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;

    // log with sourcemod format
    void log_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
        log_amx_tpl(ctx, source_loc{}, lvl, params, param);
    }
    void log_amx_tpl(plugin_ctx *ctx, const source_loc &loc, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;

    // special log
    // log with stack trace
    void log_stack_trace(plugin_ctx *ctx, level_enum lvl, string_view_t msg) const noexcept;
    void log_stack_trace(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;
    void log_stack_trace_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;

    // log with throw error
    void throw_error(plugin_ctx *ctx, level_enum lvl, string_view_t msg) const noexcept;
    void throw_error(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;
    void throw_error_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;

    // return true if logging is enabled for the given level.
    [[nodiscard]]
    bool should_log(level_enum msg_level) const noexcept {
        return msg_level >= level_.load(std::memory_order_relaxed);
    }

    // return true if the given messages should be flushed
    [[nodiscard]]
    bool should_flush(const log_msg msg) const noexcept {
        return (msg.level >= flush_level_.load(std::memory_order_relaxed)) && (msg.level != level_enum::off);
    }

    // set the level of logging
    void set_level(level_enum level) noexcept {
        level_.store(level);
    }

    // return the active log level
    [[nodiscard]]
    level_enum level() const noexcept {
        return static_cast<level_enum>(level_.load(std::memory_order_relaxed));
    }

    // return the name of the logger
    [[nodiscard]]
    const std::string &name() const noexcept {
        return name_;
    }

    // set formatting for the sinks in this logger.
    // each sink will get a separate instance of the formatter object.
    void set_formatter(std::unique_ptr<formatter> fmt) noexcept;

    // set formatting for the sinks in this logger.
    // equivalent to
    //     set_formatter(make_unique<pattern_formatter>(pattern, type))
    // Note: each sink will get a new instance of a formatter object, replacing the old one.
    void set_pattern(std::string pattern, pattern_time_type type = pattern_time_type::local) noexcept;

    // flush
    void flush(plugin_ctx *ctx) noexcept        { assert(ctx);          flush_(err_helper::src_helper(ctx)); }
    void flush(const source_loc &loc) noexcept  { assert(!loc.empty()); flush_(err_helper::src_helper(loc)); }
    void flush_on(level_enum lvl) noexcept      { flush_level_.store(lvl); }
    [[nodiscard]]
    level_enum flush_level() const noexcept {
        return static_cast<level_enum>(flush_level_.load(std::memory_order_relaxed));
    }

    // sinks
    [[nodiscard]]
    const std::vector<sink_ptr> &sinks() const noexcept { return sinks_; }
    [[nodiscard]]
    std::vector<sink_ptr> &sinks() noexcept             { return sinks_; }
    void add_sink(sink_ptr sink) noexcept               { sinks_.push_back(sink); }
    void remove_sink(sink_ptr sink) noexcept {
        sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
    }

    // error handler
    void set_error_handler(SourcePawn::IPluginFunction *function) noexcept {
        err_helper_.set_err_handler(function);
    }

private:
    class err_helper final {
    public:
        class src_helper {
        public:
            // 若 source_loc == empty 则 ctx 必须 != nullptr （反之亦然）
            constexpr src_helper(plugin_ctx *ctx) noexcept : ctx_(ctx) {}
            constexpr src_helper(const source_loc &loc) noexcept : loc_(loc) {}
            constexpr src_helper(const source_loc &loc, plugin_ctx *ctx) noexcept : loc_(loc), ctx_(ctx) {
                assert(ctx || !loc.empty());
            }

            [[nodiscard]]
            const source_loc &loc() const noexcept;

        private:
            mutable source_loc loc_;
            SourcePawn::IPluginContext *ctx_{nullptr};
        };

    public:
        err_helper() noexcept = default;
        ~err_helper() noexcept;

        void set_err_handler(SourcePawn::IPluginFunction *function) noexcept;
        void handle_ex(const std::string &origin, const src_helper &src, const std::exception &ex) const noexcept;
        void handle_unknown_ex(const std::string &origin, const src_helper &src) const noexcept;

    private:
        void release_forward() noexcept;

        SourceMod::IChangeableForward *custom_err_handler_{nullptr};
    };

    // source 用于发生错误时获取错误发生的源码位置
    void sink_it_(const log_msg &msg, const err_helper::src_helper &source) const noexcept;
    void flush_(const err_helper::src_helper &source) const noexcept;

    std::string name_;
    std::vector<sink_ptr> sinks_;
    level_t level_{level_enum::info};
    level_t flush_level_{level_enum::off};
    err_helper err_helper_;
};


}       // namespace log4sp
