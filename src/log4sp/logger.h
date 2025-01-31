#pragma once

#include "spdlog/pattern_formatter.h"
#include "spdlog/details/log_msg.h"

#include "extension.h"

#include "log4sp/common.h"
#include "log4sp/sinks/base_sink.h"


namespace log4sp {
// 自己实现 logger 而不是依赖于 spdlog
class logger final {
public:
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

    // loc 和 ctx 必选其一，用于发生异常时标记来源
    // 当调用者是 SourcePawn 时，ctx 不为 nullptr，所以总是能获取源代码位置
    // 当调用者是其他时，例如 控制台指令模块，ctx 可以为 null，但 loc 必须是相应的代码位置
    void log(const source_loc loc, const level::level_enum lvl, const string_view_t msg, SourcePawn::IPluginContext *ctx = nullptr) const noexcept;
    void log(const source_loc loc, const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, const uint32_t param) const noexcept;
    // void log(const source_loc loc, const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, const uint32_t param) const noexcept;
    void log_amx_tpl(const source_loc loc, const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, const uint32_t param) const noexcept;

    // 所有参数都是必选项，ctx 用于获取堆栈信息
    void log_stack_trace(const level::level_enum lvl, string_view_t msg, SourcePawn::IPluginContext *ctx) const noexcept;
    void log_stack_trace(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept;
    void log_stack_trace_amx_tpl(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept;

    // 所有参数都是必选项，ctx 用于获取堆栈信息
    void throw_error(const level::level_enum lvl, string_view_t msg, SourcePawn::IPluginContext *ctx) const noexcept;
    void throw_error(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept;
    void throw_error_amx_tpl(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept;

    // return true if logging is enabled for the given level.
    [[nodiscard]] bool should_log(level::level_enum msg_level) const noexcept;

    // return true if the given messages should be flushed
    [[nodiscard]] bool should_flush(const details::log_msg msg) const noexcept;

    // set the level of logging
    void set_level(level::level_enum level) noexcept;

    // return the active log level
    [[nodiscard]] level::level_enum level() const noexcept;

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
    void flush(const source_loc loc, SourcePawn::IPluginContext *ctx) noexcept;
    void flush_on(level::level_enum lvl) noexcept;
    [[nodiscard]] level::level_enum flush_level() const noexcept;

    // sinks
    [[nodiscard]] const std::vector<sink_ptr> &sinks() const noexcept;
    [[nodiscard]] std::vector<sink_ptr> &sinks() noexcept;
    void add_sink(sink_ptr sink) noexcept;
    void remove_sink(sink_ptr sink) noexcept;

    // error handler
    void set_error_handler(SourceMod::IChangeableForward *handler) noexcept;

private:
    void sink_it_(const details::log_msg &msg, SourcePawn::IPluginContext *ctx = nullptr) const noexcept;
    void flush_(const source_loc loc = {}, SourcePawn::IPluginContext *ctx = nullptr) const noexcept;

    class err_helper final {
    public:
        void handle_ex(const std::string &origin, const source_loc &loc, const std::exception &ex) const noexcept;
        void handle_unknown_ex(const std::string &origin, const source_loc &loc) const noexcept;
        void set_err_handler(SourceMod::IChangeableForward *handler) noexcept;
        ~err_helper() noexcept;
    private:
        void release_forward() noexcept;

        SourceMod::IChangeableForward *custom_error_handler_{nullptr};
    };

    std::string name_;
    std::vector<sink_ptr> sinks_;
    level_t level_{level::info};
    level_t flush_level_{level::off};
    err_helper err_helper_;
};


}       // namespace log4sp
