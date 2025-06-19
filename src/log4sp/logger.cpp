#include <cassert>

#include "spdlog/pattern_formatter.h"

#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

using spdlog::pattern_formatter;
using spdlog::pattern_time_type;
using spdlog::sink_ptr;
using spdlog::source_loc;
using spdlog::fmt_lib::format;
using spdlog::level::level_enum;

// log with no format string, just string message
void logger::log(const source_loc &loc, level_enum lvl, string_view_t msg) const noexcept {
    assert(!loc.empty());

    if (should_log(lvl)) {
        sink_it_(log_msg(loc, name_, lvl, msg), src_helper(loc));
    }
}

void logger::log(plugin_ctx *ctx, level_enum lvl, string_view_t msg) const noexcept {
    assert(ctx);

    if (should_log(lvl)) {
        sink_it_(log_msg(name_, lvl, msg), src_helper(ctx));
    }
}

// log with log4sp format
void logger::log(plugin_ctx *ctx, const source_loc &loc, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    assert(ctx && params);

    if (should_log(lvl)) {
        src_helper source(loc, ctx);
        std::string msg;

        try {
            msg = format_cell_to_string(ctx, params, param);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, source, ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, source);
            return;
        }

        sink_it_(log_msg(loc, name_, lvl, msg), source);
    }
}

// log with sourcemod format
void logger::log_amx_tpl(plugin_ctx *ctx, const source_loc &loc, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    assert(ctx && params);

    if (should_log(lvl)) {
        src_helper source(loc, ctx);
        char msg[2048];
        DetectExceptions eh(ctx);

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException()) {
            return;
        }

        sink_it_(log_msg(loc, name_, lvl, msg), source);
    }
}

// special log
void logger::log_stack_trace(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    // ! FIXME: "ctx->GetContext()" 被标记为过时，但没找到可替代的方案
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()));

    if (should_log(lvl)) {
        src_helper source(ctx);
        std::string msg;

        try {
            msg = format_cell_to_string(ctx, params, param);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, source, ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, source);
            return;
        }

        sink_it_(log_msg(name_, lvl, format("Stack trace requested: {}", msg)), source);
        sink_it_(log_msg(name_, lvl, format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())), source);

        std::vector<std::string> messages = log4sp::src_helper::get_stack_trace(ctx);
        for (auto &iter : messages) {
            sink_it_(log_msg(name_, lvl, iter), source);
        }
    }
}

void logger::log_stack_trace_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    // ! FIXME: "ctx->GetContext()" 被标记为过时，但没找到可替代的方案
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()) && params);

    if (should_log(lvl)) {
        src_helper source(ctx);
        char msg[2048];
        DetectExceptions eh(ctx);

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException()) {
            return;
        }

        sink_it_(log_msg(name_, lvl, format("Stack trace requested: {}", msg)), source);
        sink_it_(log_msg(name_, lvl, format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())), source);

        std::vector<std::string> messages = log4sp::src_helper::get_stack_trace(ctx);
        for (auto &iter : messages) {
            sink_it_(log_msg(name_, lvl, iter), source);
        }
    }
}

void logger::throw_error(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    // ! FIXME: "ctx->GetContext()" 被标记为过时，但没找到可替代的方案
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()) && params);

    src_helper source(ctx);
    std::string msg;
    try {
        msg = format_cell_to_string(ctx, params, param);
    } catch (const std::exception &ex) {
        ctx->ReportError(ex.what());
        err_helper_.handle_ex(name_, source, ex);
        return;
    } catch (...) {
        ctx->ReportError("unknown exception");
        err_helper_.handle_unknown_ex(name_, source);
        return;
    }

    ctx->ReportError(msg.c_str());

    if (should_log(lvl)) {
        sink_it_(log_msg(name_, lvl, format("Exception reported: {}", msg)), source);
        sink_it_(log_msg(name_, lvl, format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())), source);

        std::vector<std::string> messages = log4sp::src_helper::get_stack_trace(ctx);
        for (auto &iter : messages) {
            sink_it_(log_msg(name_, lvl, iter), source);
        }
    }
}

void logger::throw_error_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    // ! FIXME: "ctx->GetContext()" 被标记为过时，但没找到可替代的方案
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()) && params);

    char msg[2048];
    DetectExceptions eh(ctx);

    smutils->FormatString(msg, sizeof(msg), ctx, params, param);
    if (eh.HasException()) {
        return;
    }

    ctx->ReportError(msg);

    if (should_log(lvl)) {
        src_helper source(ctx);

        sink_it_(log_msg(name_, lvl, format("Exception reported: {}", msg)), source);
        sink_it_(log_msg(name_, lvl, format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())), source);

        std::vector<std::string> messages = log4sp::src_helper::get_stack_trace(ctx);
        for (auto &iter : messages) {
            sink_it_(log_msg(name_, lvl, iter), source);
        }
    }
}

[[nodiscard]] bool logger::should_log(level_enum msg_level) const noexcept {
    return msg_level >= level_.load(std::memory_order_relaxed);
}

[[nodiscard]] bool logger::should_flush(const log_msg msg) const noexcept {
    return (msg.level >= flush_level_.load(std::memory_order_relaxed)) && (msg.level != level_enum::off);
}

void logger::set_level(level_enum level) noexcept {
    level_.store(level);
}

[[nodiscard]] level_enum logger::level() const noexcept {
    return static_cast<level_enum>(level_.load(std::memory_order_relaxed));
}

[[nodiscard]] const std::string &logger::name() const noexcept {
    return name_;
}

void logger::set_formatter(std::unique_ptr<formatter> fmt) noexcept {
    for (auto it = sinks_.begin(); it != sinks_.end(); ++it) {
        if (std::next(it) == sinks_.end()) {
            // last element - we can move it.
            (*it)->set_formatter(std::move(fmt));
            break;  // to prevent clang-tidy warning
        }
        (*it)->set_formatter(fmt->clone());
    }
}

void logger::set_pattern(std::string pattern, pattern_time_type type) noexcept {
    set_formatter(std::make_unique<pattern_formatter>(pattern, type));
}

void logger::flush(plugin_ctx *ctx) noexcept {
    flush_(src_helper{source_loc(), ctx});
}

void logger::flush(const source_loc &loc) noexcept {
    flush_(src_helper(loc, nullptr));
}

void logger::flush_on(level_enum level) noexcept {
    flush_level_.store(level);
}

[[nodiscard]] level_enum logger::flush_level() const noexcept {
    return static_cast<level_enum>(flush_level_.load(std::memory_order_relaxed));
}

[[nodiscard]] const std::vector<sink_ptr> &logger::sinks() const noexcept {
    return sinks_;
}

[[nodiscard]] std::vector<sink_ptr> &logger::sinks() noexcept {
    return sinks_;
}

void logger::add_sink(sink_ptr sink) noexcept {
    sinks_.push_back(sink);
}

void logger::remove_sink(sink_ptr sink) noexcept {
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
}

void logger::set_error_handler(SourceMod::IChangeableForward *handler) noexcept {
    err_helper_.set_err_handler(handler);
}

void logger::sink_it_(const log_msg &msg, const src_helper &source) const noexcept {
    for (auto &sink : sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                err_helper_.handle_ex(name_, source, ex);
            } catch (...) {
                err_helper_.handle_unknown_ex(name_, source);
            }
        }
    }

    if (should_flush(msg)) {
        flush_(source);
    }
}

void logger::flush_(const src_helper &source) const noexcept {
    for (auto &sink : sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, source, ex);
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, source);
        }
    }
}


}       // namespace log4sp
