#include "log4sp/adapter/logger_handler.h"

#include "log4sp/utils.h"
#include "log4sp/logger.h"


namespace log4sp {

void logger::log(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, const spdlog::string_view_t msg, IPluginContext *ctx) const noexcept {
    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled) {
        sink_it_(spdlog::details::log_msg{loc, name_, lvl, msg}, ctx);
    }

    if (traceback_enabled) {
        tracer_->log(spdlog::details::log_msg{loc, name_, lvl, msg});
    }
}

void logger::log(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, const unsigned int param) const {
    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        std::string msg;

        try {
            msg = format_cell_to_string(ctx, params, param);
        } catch (const std::exception &ex) {
            if (loc.empty() && ctx) {
                err_helper_.handle_ex(name_, get_plugin_source_loc(ctx), ex);
            } else {
                err_helper_.handle_ex(name_, loc, ex);
            }
            return;
        } catch (...) {
            if (loc.empty() && ctx) {
                err_helper_.handle_unknown_ex(name_, get_plugin_source_loc(ctx));
            } else {
                err_helper_.handle_unknown_ex(name_, loc);
            }
            return;
        }

        if (log_enabled) {
            sink_it_(spdlog::details::log_msg{loc, name_, lvl, msg}, ctx);
        }

        if (traceback_enabled) {
            tracer_->log(spdlog::details::log_msg{loc, name_, lvl, msg});
        }
    }
}

// void logger::log(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, IPluginContext *ctx, const char *format, const cell_t *params, const unsigned int param) const {
//     if (should_log(lvl)) {
//         int lparam{param};
//         spdlog::fmt_lib::memory_buffer buffer;

//         try {
//             buffer = format_cell_to_memory_buf(format, ctx, params, &lparam);
//         } catch (const std::exception &ex) {
//             if (loc.empty() && ctx) {
//                 err_helper_.handle_ex(name_, get_plugin_source_loc(ctx), ex);
//             } else {
//                 err_helper_.handle_ex(name_, loc, ex);
//             }
//             return;
//         } catch (...) {
//             if (loc.empty() && ctx) {
//                 err_helper_.handle_unknown_ex(name_, get_plugin_source_loc(ctx));
//             } else {
//                 err_helper_.handle_unknown_ex(name_, loc);
//             }
//             return;
//         }

//         sink_it_(spdlog::details::log_msg{loc, name_, lvl, spdlog::fmt_lib::to_string(buffer)}, ctx);
//     }
// }

void logger::log_amx_tpl(const spdlog::source_loc loc, const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, const unsigned int param) const noexcept {
    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        char msg[2048];
        DetectExceptions eh(ctx);

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException()) {
            return;
        }

        if (log_enabled) {
            sink_it_(spdlog::details::log_msg{loc, name_, lvl, msg}, ctx);
        }

        if (traceback_enabled) {
            tracer_->log(spdlog::details::log_msg{loc, name_, lvl, msg});
        }
    }
}

void logger::log_stack_trace(const spdlog::level::level_enum lvl, spdlog::string_view_t msg, IPluginContext *ctx) const noexcept {
    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        std::vector<std::string> messages{
            spdlog::fmt_lib::format("Stack trace requested: {}", msg),
            spdlog::fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())
        };

        std::vector<std::string> stack_trace{get_stack_trace(ctx)};
        messages.insert(messages.end(), stack_trace.begin(), stack_trace.end());

        if (log_enabled) {
            for (auto &iter : messages) {
                sink_it_(spdlog::details::log_msg{name_, lvl, iter}, ctx);
            }
        }

        if (traceback_enabled) {
            for (auto &iter : messages) {
                tracer_->log(spdlog::details::log_msg{name_, lvl, iter});
            }
        }
    }
}

void logger::log_stack_trace(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const {
    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        std::string msg;

        try {
            msg = format_cell_to_string(ctx, params, param);
        } catch (const std::exception &ex) {
            err_helper_.handle_ex(name_, get_plugin_source_loc(ctx), ex);
            return;
        } catch (...) {
            err_helper_.handle_unknown_ex(name_, get_plugin_source_loc(ctx));
            return;
        }

        std::vector<std::string> messages{
            spdlog::fmt_lib::format("Stack trace requested: {}", msg),
            spdlog::fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())
        };

        std::vector<std::string> stack_trace{get_stack_trace(ctx)};
        messages.insert(messages.end(), stack_trace.begin(), stack_trace.end());

        if (log_enabled) {
            for (auto &iter : messages) {
                sink_it_(spdlog::details::log_msg{name_, lvl, iter}, ctx);
            }
        }

        if (traceback_enabled) {
            for (auto &iter : messages) {
                tracer_->log(spdlog::details::log_msg{name_, lvl, iter});
            }
        }
    }
}

void logger::log_stack_trace_amx_tpl(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const noexcept  {
    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        char msg[2048];
        DetectExceptions eh(ctx);

        smutils->FormatString(msg, sizeof(msg), ctx, params, param);
        if (eh.HasException()) {
            return;
        }

        std::vector<std::string> messages{
            spdlog::fmt_lib::format("Stack trace requested: {}", msg),
            spdlog::fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())
        };

        std::vector<std::string> stack_trace{get_stack_trace(ctx)};
        messages.insert(messages.end(), stack_trace.begin(), stack_trace.end());

        if (log_enabled) {
            for (auto &iter : messages) {
                sink_it_(spdlog::details::log_msg{name_, lvl, iter}, ctx);
            }
        }

        if (traceback_enabled) {
            for (auto &iter : messages) {
                tracer_->log(spdlog::details::log_msg{name_, lvl, iter});
            }
        }
    }
}

void logger::throw_error(const spdlog::level::level_enum lvl, spdlog::string_view_t msg, IPluginContext *ctx) const noexcept  {
    ctx->ReportError(msg.data());

    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        std::vector<std::string> messages{
            spdlog::fmt_lib::format("Exception reported: {}", msg),
            spdlog::fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())
        };

        std::vector<std::string> stack_trace{get_stack_trace(ctx)};
        messages.insert(messages.end(), stack_trace.begin(), stack_trace.end());

        if (log_enabled) {
            for (auto &iter : messages) {
                sink_it_(spdlog::details::log_msg{name_, lvl, iter}, ctx);
            }
        }

        if (traceback_enabled) {
            for (auto &iter : messages) {
                tracer_->log(spdlog::details::log_msg{name_, lvl, iter});
            }
        }
    }
}

void logger::throw_error(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const noexcept  {
    std::string msg;
    try {
        msg = format_cell_to_string(ctx, params, param);
    } catch (const std::exception &ex) {
        ctx->ReportError(ex.what());
        err_helper_.handle_ex(name_, get_plugin_source_loc(ctx), ex);
        return;
    } catch (...) {
        ctx->ReportError("unknown exception");
        err_helper_.handle_unknown_ex(name_, get_plugin_source_loc(ctx));
        return;
    }

    ctx->ReportError(msg.c_str());

    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        std::vector<std::string> messages{
            spdlog::fmt_lib::format("Exception reported: {}", msg),
            spdlog::fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())
        };

        std::vector<std::string> stack_trace{get_stack_trace(ctx)};
        messages.insert(messages.end(), stack_trace.begin(), stack_trace.end());

        if (log_enabled) {
            for (auto &iter : messages) {
                sink_it_(spdlog::details::log_msg{name_, lvl, iter}, ctx);
            }
        }

        if (traceback_enabled) {
            for (auto &iter : messages) {
                tracer_->log(spdlog::details::log_msg{name_, lvl, iter});
            }
        }
    }
}

void logger::throw_error_amx_tpl(const spdlog::level::level_enum lvl, IPluginContext *ctx, const cell_t *params, unsigned int param) const noexcept  {
    char msg[2048];
    DetectExceptions eh(ctx);

    smutils->FormatString(msg, sizeof(msg), ctx, params, param);
    if (eh.HasException()) {
        return;
    }

    ctx->ReportError(msg);

    bool log_enabled = should_log(lvl);
    bool traceback_enabled = (tracer_ && lvl <= spdlog::level::debug);

    if (log_enabled || traceback_enabled) {
        std::vector<std::string> messages{
            spdlog::fmt_lib::format("Exception reported: {}", msg),
            spdlog::fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename())
        };

        std::vector<std::string> stack_trace{get_stack_trace(ctx)};
        messages.insert(messages.end(), stack_trace.begin(), stack_trace.end());

        if (log_enabled) {
            for (auto &iter : messages) {
                sink_it_(spdlog::details::log_msg{name_, lvl, iter}, ctx);
            }
        }

        if (traceback_enabled) {
            for (auto &iter : messages) {
                tracer_->log(spdlog::details::log_msg{name_, lvl, iter});
            }
        }
    }
}

[[nodiscard]] bool logger::should_log(spdlog::level::level_enum msg_level) const noexcept {
    return msg_level >= level_;
}

[[nodiscard]] bool logger::should_flush(const spdlog::details::log_msg msg) const noexcept {
    return (msg.level >= flush_level_) && (msg.level != spdlog::level::off);
}

void logger::set_level(spdlog::level::level_enum level) noexcept {
    level_ = level;
}

[[nodiscard]] spdlog::level::level_enum logger::level() const noexcept {
    return level_;
}

[[nodiscard]] const std::string &logger::name() const noexcept {
    return name_;
}

void logger::set_formatter(std::unique_ptr<spdlog::formatter> fmt) noexcept {
    for (auto it = sinks_.begin(); it != sinks_.end(); ++it) {
        if (std::next(it) == sinks_.end()) {
            // last element - we can move it.
            (*it)->set_formatter(std::move(fmt));
            break;  // to prevent clang-tidy warning
        }
        (*it)->set_formatter(fmt->clone());
    }
}

void logger::set_pattern(std::string pattern, spdlog::pattern_time_type type) noexcept {
    set_formatter(std::make_unique<spdlog::pattern_formatter>(pattern, type));
}

void logger::flush(const spdlog::source_loc loc, IPluginContext *ctx) noexcept {
    flush_(loc, ctx);
}

void logger::flush_on(spdlog::level::level_enum level) noexcept {
    flush_level_ = level;
}

[[nodiscard]] spdlog::level::level_enum logger::flush_level() const noexcept {
    return flush_level_;
}

[[nodiscard]] const std::vector<spdlog::sink_ptr> &logger::sinks() const noexcept {
    return sinks_;
}

[[nodiscard]] std::vector<spdlog::sink_ptr> &logger::sinks() noexcept {
    return sinks_;
}

void logger::add_sink(spdlog::sink_ptr sink) noexcept {
    sinks_.push_back(sink);
}

void logger::remove_sink(spdlog::sink_ptr sink) noexcept {
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
}

[[nodiscard]] bool logger::should_backtrace() const noexcept {
    return tracer_ != nullptr;
}

void logger::enable_backtrace(size_t num) noexcept {
    tracer_ = std::make_shared<spdlog::sinks::ringbuffer_sink_st>(num);
    add_sink(tracer_);
}

void logger::disable_backtrace() noexcept {
    remove_sink(tracer_);
    tracer_.reset();
}

void logger::dump_backtrace(IPluginContext *ctx) const noexcept {
    if (tracer_) {
        auto messages = tracer_->last_raw();
        for (auto &msg : messages) {
            sink_it_(msg, ctx);
        }
    }
}

void logger::set_error_handler(IChangeableForward *handler) noexcept {
    err_helper_.set_err_handler(handler);
}

void logger::sink_it_(const spdlog::details::log_msg &msg, IPluginContext *ctx) const noexcept {
    spdlog::source_loc loc{msg.source};
    for (auto &sink : sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                if (loc.empty() && ctx) {
                    loc = get_plugin_source_loc(ctx);
                }
                err_helper_.handle_ex(name_, loc, ex);
            } catch (...) {
                if (loc.empty() && ctx) {
                    loc = get_plugin_source_loc(ctx);
                }
                err_helper_.handle_unknown_ex(name_, loc);
            }
        }
    }

    if (should_flush(msg)) {
        flush_(loc, ctx);
    }
}

void logger::flush_(const spdlog::source_loc &loc, IPluginContext *ctx) const noexcept {
    spdlog::source_loc source_location{loc};
    for (auto &sink : sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            if (source_location.empty() && ctx) {
                source_location = get_plugin_source_loc(ctx);
            }
            err_helper_.handle_ex(name_, loc, ex);
        } catch (...) {
            if (source_location.empty() && ctx) {
                source_location = get_plugin_source_loc(ctx);
            }
            err_helper_.handle_unknown_ex(name_, source_location);
        }
    }
}


// err_helper
void logger::err_helper::handle_ex(const std::string &origin, const spdlog::source_loc &loc, const std::exception &ex) const noexcept {
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
        const char *file{spdlog::details::short_filename_formatter<spdlog::details::null_scoped_padder>::basename(loc.filename)};
        smutils->LogError(myself, "[%s::%d] [%s] %s", file, loc.line, origin.c_str(), ex.what());
    } catch (const std::exception &handler_ex) {
        smutils->LogError(myself, "[%s] caught exception during error handler: %s", origin.c_str(), handler_ex.what());
    } catch (...) {
        smutils->LogError(myself, "[%s] caught unknown exception during error handler", origin.c_str());
    }
}

void logger::err_helper::handle_unknown_ex(const std::string &origin, const spdlog::source_loc &loc) const noexcept {
    handle_ex(origin, loc, std::runtime_error{"unknown exception"});
}

void logger::err_helper::set_err_handler(IChangeableForward *handler) noexcept {
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
