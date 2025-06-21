#pragma once

#include "spdlog/sinks/sink.h"

#include "extension.h"

#include "log4sp/common.h"
#include "log4sp/source_helper.h"


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
    void log(plugin_ctx *ctx, level_enum lvl, string_view_t msg) const noexcept;
    void log(const source_loc &loc, level_enum lvl, string_view_t msg) const noexcept;

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
    void log_stack_trace(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;
    void log_stack_trace_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;

    void throw_error(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;
    void throw_error_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept;

    // return true if logging is enabled for the given level.
    [[nodiscard]] bool should_log(level_enum msg_level) const noexcept;

    // return true if the given messages should be flushed
    [[nodiscard]] bool should_flush(const log_msg msg) const noexcept;

    // set the level of logging
    void set_level(level_enum level) noexcept;

    // return the active log level
    [[nodiscard]] level_enum level() const noexcept;

    // return the name of the logger
    [[nodiscard]] const std::string &name() const noexcept;

    // set formatting for the sinks in this logger.
    // each sink will get a separate instance of the formatter object.
    void set_formatter(std::unique_ptr<formatter> fmt) noexcept;

    // set formatting for the sinks in this logger.
    // equivalent to
    //     set_formatter(make_unique<pattern_formatter>(pattern, type))
    // Note: each sink will get a new instance of a formatter object, replacing the old one.
    void set_pattern(std::string pattern, pattern_time_type type = pattern_time_type::local) noexcept;

    // flush
    void flush(plugin_ctx *ctx) noexcept;
    void flush(const source_loc &loc) noexcept;
    void flush_on(level_enum lvl) noexcept;
    [[nodiscard]] level_enum flush_level() const noexcept;

    // sinks
    [[nodiscard]] const std::vector<sink_ptr> &sinks() const noexcept;
    [[nodiscard]] std::vector<sink_ptr> &sinks() noexcept;
    void add_sink(sink_ptr sink) noexcept;
    void remove_sink(sink_ptr sink) noexcept;

    // error handler
    void set_error_handler(SourceMod::IChangeableForward *handler) noexcept;

private:
    // source 用于发生错误时获取错误发生的源码位置
    void sink_it_(const log_msg &msg, const src_helper &source) const noexcept;
    void flush_(const src_helper &source) const noexcept;

    std::string name_;
    std::vector<sink_ptr> sinks_;
    level_t level_{level_enum::info};
    level_t flush_level_{level_enum::off};
    err_helper err_helper_;
};


}       // namespace log4sp
