#include "spdlog/pattern_formatter.h"

#include "log4sp/format.h"
#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

// log with log4sp format
void logger::log(plugin_ctx *ctx, const source_loc &loc, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    assert(ctx && params);

    if (should_log(lvl)) {
        err_helper::src_helper source(loc, ctx);
        try {
            std::string msg = format_to_string(ctx, params, param);
            sink_it_(log_msg(loc, name_, lvl, msg), source);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, source, ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, source);
            return;
        }
    }
}

// log with sourcemod format
void logger::log_amx_tpl(plugin_ctx *ctx, const source_loc &loc, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    assert(ctx && params);

    if (should_log(lvl)) {
        err_helper::src_helper source(loc, ctx);
        std::array<char, 2048> msg;
        DetectExceptions eh(ctx);

        smutils->FormatString(msg.data(), msg.size(), ctx, params, param);
        if (eh.HasException()) {
            return;
        }

        sink_it_(log_msg(loc, name_, lvl, msg.data()), source);
    }
}

// special log
// log with stack trace
void logger::log_stack_trace(plugin_ctx *ctx, level_enum lvl, string_view_t msg) const noexcept {
    using spdlog::fmt_lib::format;
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()));  // ! FIXME: "ctx->GetContext()" 被标记为过时

    if (should_log(lvl)) {
        log(ctx, lvl, format("Stack trace requested: {}", msg));
        log(ctx, lvl, format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename()));
        for(const auto &info : stack_trace_info_from(ctx)) {
            log(ctx, lvl, info);
        }
    }
}

void logger::log_stack_trace(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    if (should_log(lvl)) {
        auto src = err_helper::src_helper(ctx);
        try {
            std::string msg = format_to_string(ctx, params, param);
            log_stack_trace(ctx, lvl, msg);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, src, ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, src);
            return;
        }
    }
}

void logger::log_stack_trace_amx_tpl(plugin_ctx *ctx, level_enum lvl, const cell_t *params, unsigned int param) const noexcept {
    if (should_log(lvl)) {
        std::array<char, 2048> msg;
        DetectExceptions eh(ctx);
        smutils->FormatString(msg.data(), msg.size(), ctx, params, param);
        if (eh.HasException()) {
            return;
        }
        log_stack_trace(ctx, lvl, msg.data());
    }
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
    using spdlog::pattern_formatter;
    set_formatter(std::make_unique<pattern_formatter>(pattern, type));
}

void logger::sink_it_(const log_msg &msg, const err_helper::src_helper &source) const noexcept {
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

void logger::flush_(const err_helper::src_helper &source) const noexcept {
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


/**
 * src_helper 的设计初衷:
 *  由于仅少数如 LogSrc, LogLoc 等 Log Natives 明确指定了 source_loc 的值
 *  其余大部分 Log Natives 的 source_loc 都使用默认值 (empty).
 *  这会意味着 format, sink_it_, flush_ 发生错误时 source_loc 值为 empty.
 *  即无法获取造成的错误的源码位置信息, 显然这是不利于排查错误的.
 *
 *  考虑到 logger 是一个单线程类, 且 Log Natives 必然包含一个有效的 ctx,
 *  因此可以在发生错误时根据 ctx 获取 Frame 信息.
 */
// err_helper::src_helper
const log4sp::logger::source_loc &log4sp::logger::err_helper::src_helper::loc() const noexcept {
    if (loc_.empty()) {
        loc_ = source_loc_from(ctx_);           // 仅含 plugin_ctx 构造时才可能执行
    }
    return loc_;
}


// err_helper
log4sp::logger::err_helper::~err_helper() noexcept {
    release_forward();
}

void logger::err_helper::set_err_handler(SourcePawn::IPluginFunction *function) noexcept {
    // void (const char[] msg, const char[] name, const char[] file, int line, const char[] func)
    FWDS_CREATE_EX(nullptr, ET_Ignore, 5, nullptr,
                   Param_String,                // msg
                   Param_String,                // name
                   Param_String,                // file
                   Param_Cell,                  // line
                   Param_String);               // func
    FWD_ADD_FUNCTION(function);
    release_forward();
    custom_err_handler_ = forward;
}

void logger::err_helper::handle_ex(const std::string &origin, const src_helper &src, const std::exception &ex) const noexcept {
    try {
        auto loc = src.loc();
        if (custom_err_handler_) {
            auto forward = custom_err_handler_;
            FWD_PUSH_STRING(ex.what());         // msg
            FWD_PUSH_STRING(origin.c_str());    // name
            FWD_PUSH_STRING(loc.filename);      // file
            FWD_PUSH_CELL(loc.line);            // line
            FWD_PUSH_STRING(loc.funcname);      // func
            FWD_EXECUTE();
            return;
        }
        smutils->LogError(myself, "[%s::%d] [%s] %s", filename_from(loc.filename), loc.line, origin.c_str(), ex.what());
    } catch (const std::exception &handler_ex) {
        smutils->LogError(myself, "[%s] caught exception during %s handler: %s", origin.c_str(), custom_err_handler_ ? "custom" : "default", handler_ex.what());
    } catch (...) {
        smutils->LogError(myself, "[%s] caught unknown exception during %s handler", origin.c_str(), custom_err_handler_ ? "custom" : "default");
    }
}

void logger::err_helper::handle_unknown_ex(const std::string &origin, const src_helper &src) const noexcept {
    handle_ex(origin, src, std::runtime_error("unknown exception"));
}

void logger::err_helper::release_forward() noexcept {
    if (custom_err_handler_) {
        forwards->ReleaseForward(custom_err_handler_);
        custom_err_handler_ = nullptr;
    }
}


}       // namespace log4sp
