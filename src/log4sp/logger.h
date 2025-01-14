#pragma once

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/sinks/ringbuffer_sink.h"

#include "extension.h"


namespace log4sp {

class logger final {
public:
    explicit logger(std::string name)
        : name_(std::move(name)) {}

    template <typename It>
    logger(std::string name, It begin, It end)
        : name_(std::move(name)), sinks_(begin, end) {}

    logger(std::string name, spdlog::sink_ptr single_sink)
        : logger(std::move(name), {std::move(single_sink)}) {}

    logger(std::string name, spdlog::sinks_init_list sinks)
        : logger(std::move(name), sinks.begin(), sinks.end()) {}

    ~logger() = default;

    // ctx 用于发生异常且消息 loc 为空时，获取插件源码位置。如果 nullptr 则保持为 nullptr
    void log(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, const spdlog::string_view_t msg, IPluginContext *ctx) const noexcept;
    void log(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, const unsigned int param) const;
    // void log(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, IPluginContext *ctx, const char *format, const cell_t *params, const unsigned int param) const;
    void log_amx_tpl(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, const unsigned int param) const noexcept;

    void log_stack_trace(const spdlog::level::level_enum lvl, spdlog::string_view_t msg, IPluginContext *ctx) const noexcept;
    void log_stack_trace(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const;
    void log_stack_trace_amx_tpl(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const noexcept;

    void throw_error(const spdlog::level::level_enum lvl, spdlog::string_view_t msg, IPluginContext *ctx) const noexcept;
    void throw_error(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const noexcept;
    void throw_error_amx_tpl(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const noexcept;

    // return true if logging is enabled for the given level.
    [[nodiscard]] bool should_log(spdlog::level::level_enum msg_level) const noexcept;

    // return true if the given messages should be flushed
    [[nodiscard]] bool should_flush(const spdlog::details::log_msg msg) const noexcept;

    // set the level of logging
    void set_level(spdlog::level::level_enum level) noexcept;

    // return the active log level
    [[nodiscard]] spdlog::level::level_enum level() const noexcept;

    // return the name of the logger
    [[nodiscard]] const std::string &name() const noexcept;

    // set formatting for the sinks in this logger.
    // each sink will get a separate instance of the formatter object.
    void set_formatter(std::unique_ptr<spdlog::formatter> fmt) noexcept;

    // set formatting for the sinks in this logger.
    // equivalent to
    //     set_formatter(make_unique<pattern_formatter>(pattern, type))
    // Note: each sink will get a new instance of a formatter object, replacing the old one.
    void set_pattern(std::string pattern, spdlog::pattern_time_type type = spdlog::pattern_time_type::local) noexcept;

    // flush
    void flush(const spdlog::source_loc loc, IPluginContext *ctx) noexcept;
    void flush_on(spdlog::level::level_enum level) noexcept;
    [[nodiscard]] spdlog::level::level_enum flush_level() const noexcept;

    // sinks
    [[nodiscard]] const std::vector<spdlog::sink_ptr> &sinks() const noexcept;
    [[nodiscard]] std::vector<spdlog::sink_ptr> &sinks() noexcept;
    void add_sink(spdlog::sink_ptr sink) noexcept;
    void remove_sink(spdlog::sink_ptr sink) noexcept;

    // backtrace
    [[nodiscard]] bool should_backtrace() const noexcept;
    void enable_backtrace(size_t num) noexcept;
    void disable_backtrace() noexcept;
    void dump_backtrace(IPluginContext *ctx = nullptr) const noexcept;

    // error handler
    void set_error_handler(IChangeableForward *handler) noexcept;

private:
    void sink_it_(const spdlog::details::log_msg &msg, IPluginContext *ctx = nullptr) const noexcept;
    void flush_(const spdlog::source_loc &loc = {}, IPluginContext *ctx = nullptr) const noexcept;

    class err_helper final {
    public:
        void handle_ex(const std::string &origin, const spdlog::source_loc &loc, const std::exception &ex) const noexcept;
        void handle_unknown_ex(const std::string &origin, const spdlog::source_loc &loc) const noexcept;
        void set_err_handler(IChangeableForward *handler) noexcept;
        ~err_helper() noexcept;
    private:
        void release_forward() noexcept;

        IChangeableForward *custom_error_handler_{nullptr};
    };

    std::string name_;
    std::vector<spdlog::sink_ptr> sinks_;
    spdlog::level::level_enum level_{spdlog::level::info};
    spdlog::level::level_enum flush_level_{spdlog::level::off};
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_st> tracer_{nullptr};
    err_helper err_helper_;
};


}       // namespace log4sp
