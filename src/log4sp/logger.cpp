#include <cassert>

#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

void logger::log(const source_loc loc, const level::level_enum lvl, const string_view_t msg, SourcePawn::IPluginContext *ctx) const noexcept {
    assert(!loc.empty() || ctx);

    if (should_log(lvl)) {
        sink_it_(details::log_msg{loc, name_, lvl, msg}, ctx);
    }
}

void logger::log(const source_loc loc, const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, const uint32_t param) const noexcept {
    assert(ctx && params);

    if (should_log(lvl)) {
        std::string msg;

        try {
            msg = format_cell_to_string(ctx, params, param);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, loc.empty() ? source_loc::from_plugin_ctx(ctx) : source_loc{}, ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, loc.empty() ? source_loc::from_plugin_ctx(ctx) : source_loc{});
            return;
        }

        sink_it_(details::log_msg{loc, name_, lvl, msg}, ctx);
    }
}

void logger::log_amx_tpl(const source_loc loc, const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, const uint32_t param) const noexcept {
    assert(ctx && params);

    if (should_log(lvl)) {
        char msg[2048];
        DetectExceptions eh{ctx};

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException()) {
            sink_it_(details::log_msg{name_, lvl, eh.Message()});
            return;
        }

        sink_it_(details::log_msg{loc, name_, lvl, msg}, ctx);
    }
}

void logger::log_stack_trace(const level::level_enum lvl, string_view_t msg, SourcePawn::IPluginContext *ctx) const noexcept {
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()));

    if (should_log(lvl)) {
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Stack trace requested: {}", msg)});
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())});

        std::vector<std::string> messages{get_plugin_ctx_stack_trace(ctx)};
        for (auto &iter : messages) {
            sink_it_(details::log_msg{name_, lvl, iter}, ctx);
        }
    }
}

void logger::log_stack_trace(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept {
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()));

    if (should_log(lvl)) {
        std::string msg;

        try {
            msg = format_cell_to_string(ctx, params, param);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, source_loc::from_plugin_ctx(ctx), ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, source_loc::from_plugin_ctx(ctx));
            return;
        }

        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Stack trace requested: {}", msg)});
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())});

        std::vector<std::string> messages{get_plugin_ctx_stack_trace(ctx)};
        for (auto &iter : messages) {
            sink_it_(details::log_msg{name_, lvl, iter}, ctx);
        }
    }
}

void logger::log_stack_trace_amx_tpl(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept {
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()) && params);

    if (should_log(lvl)) {
        char msg[2048];
        DetectExceptions eh{ctx};

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException()) {
            sink_it_(details::log_msg{name_, lvl, eh.Message()});
            return;
        }

        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Stack trace requested: {}", msg)});
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())});

        std::vector<std::string> messages{get_plugin_ctx_stack_trace(ctx)};
        for (auto &iter : messages) {
            sink_it_(details::log_msg{name_, lvl, iter}, ctx);
        }
    }
}

void logger::throw_error(const level::level_enum lvl, string_view_t msg, SourcePawn::IPluginContext *ctx) const noexcept {
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()));

    ctx->ReportError(msg.data());

    if (should_log(lvl)) {
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Exception reported: {}", msg)});
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())});

        std::vector<std::string> messages{get_plugin_ctx_stack_trace(ctx)};
        for (auto &iter : messages) {
            sink_it_(details::log_msg{name_, lvl, iter}, ctx);
        }
    }
}

void logger::throw_error(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept {
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()) && params);

    std::string msg;
    try {
        msg = format_cell_to_string(ctx, params, param);
    } catch (const std::exception &ex) {
        ctx->ReportError(ex.what());
        err_helper_.handle_ex(name_, source_loc::from_plugin_ctx(ctx), ex);
        return;
    } catch (...) {
        ctx->ReportError("unknown exception");
        err_helper_.handle_unknown_ex(name_, source_loc::from_plugin_ctx(ctx));
        return;
    }

    ctx->ReportError(msg.c_str());

    if (should_log(lvl)) {
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Exception reported: {}", msg)});
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())});

        std::vector<std::string> messages{get_plugin_ctx_stack_trace(ctx)};
        for (auto &iter : messages) {
            sink_it_(details::log_msg{name_, lvl, iter}, ctx);
        }
    }
}

void logger::throw_error_amx_tpl(const level::level_enum lvl, SourcePawn::IPluginContext *ctx, const cell_t *params, uint32_t param) const noexcept {
    assert(ctx && ctx->GetContext() && plsys->FindPluginByContext(ctx->GetContext()) && params);

    char msg[2048];
    DetectExceptions eh{ctx};

    smutils->FormatString(msg, sizeof(msg), ctx, params, param);
    if (eh.HasException()) {
        sink_it_(details::log_msg{name_, lvl, eh.Message()});
        return;
    }

    ctx->ReportError(msg);

    if (should_log(lvl)) {
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Exception reported: {}", msg)});
        sink_it_(details::log_msg{name_, lvl, fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())});

        std::vector<std::string> messages{get_plugin_ctx_stack_trace(ctx)};
        for (auto &iter : messages) {
            sink_it_(details::log_msg{name_, lvl, iter}, ctx);
        }
    }
}

[[nodiscard]] bool logger::should_log(level::level_enum msg_level) const noexcept {
    return msg_level >= level_.load(std::memory_order_relaxed);
}

[[nodiscard]] bool logger::should_flush(const details::log_msg msg) const noexcept {
    return (msg.level >= flush_level_.load(std::memory_order_relaxed)) && (msg.level != level::level_enum::off);
}

void logger::set_level(level::level_enum level) noexcept {
    level_.store(level);
}

[[nodiscard]] level::level_enum logger::level() const noexcept {
    return static_cast<level::level_enum>(level_.load(std::memory_order_relaxed));
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

void logger::flush(const source_loc loc, SourcePawn::IPluginContext *ctx) noexcept {
    flush_(loc, ctx);
}

void logger::flush_on(level::level_enum level) noexcept {
    flush_level_.store(level);
}

[[nodiscard]] level::level_enum logger::flush_level() const noexcept {
    return static_cast<level::level_enum>(flush_level_.load(std::memory_order_relaxed));
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

void logger::sink_it_(const details::log_msg &msg, SourcePawn::IPluginContext *ctx) const noexcept {
    for (auto &sink : sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                err_helper_.handle_ex(name_, msg.source.empty() && ctx ? source_loc::from_plugin_ctx(ctx) : source_loc{}, ex);
            } catch (...) {
                err_helper_.handle_unknown_ex(name_, msg.source.empty() && ctx ? source_loc::from_plugin_ctx(ctx) : source_loc{});
            }
        }
    }

    if (should_flush(msg)) {
        flush_({msg.source.filename, static_cast<uint32_t>(msg.source.line), msg.source.funcname}, ctx);
    }
}

void logger::flush_(const source_loc loc, SourcePawn::IPluginContext *ctx) const noexcept {
    for (auto &sink : sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, loc.empty() && ctx ? source_loc::from_plugin_ctx(ctx) : source_loc{}, ex);
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, loc.empty() && ctx ? source_loc::from_plugin_ctx(ctx) : source_loc{});
        }
    }
}


// err_helper
void logger::err_helper::handle_ex(const std::string &origin, const source_loc &loc, const std::exception &ex) const noexcept {
    try {
        if (custom_error_handler_) {
            custom_error_handler_->PushString(ex.what());
            custom_error_handler_->PushString(origin.c_str());
            custom_error_handler_->PushString(loc.filename);
            custom_error_handler_->PushCell(loc.line);
            custom_error_handler_->PushString(loc.funcname);
            custom_error_handler_->Execute();
            return;
        }
        smutils->LogError(myself, "[%s::%d] [%s] %s", source_loc::basename(loc.filename), loc.line, origin.c_str(), ex.what());
    } catch (const std::exception &handler_ex) {
        smutils->LogError(myself, "[%s] caught exception during error handler: %s", origin.c_str(), handler_ex.what());
    } catch (...) {
        smutils->LogError(myself, "[%s] caught unknown exception during error handler", origin.c_str());
    }
}

void logger::err_helper::handle_unknown_ex(const std::string &origin, const source_loc &loc) const noexcept {
    handle_ex(origin, loc, std::runtime_error{"unknown exception"});
}

void logger::err_helper::set_err_handler(SourceMod::IChangeableForward *handler) noexcept {
    assert(handler);
    release_forward();
    custom_error_handler_ = handler;
}

logger::err_helper::~err_helper() noexcept {
    release_forward();
}

void logger::err_helper::release_forward() noexcept {
    if (custom_error_handler_) {
        forwards->ReleaseForward(custom_error_handler_);
        custom_error_handler_ = nullptr;
    }
}


}       // namespace log4sp
