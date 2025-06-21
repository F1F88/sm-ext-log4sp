#include <cassert>

#include "log4sp/common.h"

namespace log4sp {

namespace fmt_lib = spdlog::fmt_lib;
using spdlog::filename_t;
using spdlog::source_loc;

[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno) {
    spdlog::throw_spdlog_ex(msg, last_errno);
}

[[noreturn]] void throw_log4sp_ex(std::string msg) {
    spdlog::throw_spdlog_ex(std::move(msg));
}

// src_helper
[[nodiscard]] source_loc src_helper::get() const noexcept {
    if (!loc_.empty()) {
        return loc_;
    } else if (ctx_) {
        loc_ = get_from_plugin_ctx(ctx_); // 缓存以用于下一个节点(sink)也出错时
        return loc_;
    }

    assert(false);
    return source_loc{};
}

[[nodiscard]] source_loc src_helper::get_from_plugin_ctx(SourcePawn::IPluginContext *ctx) noexcept {
    assert(ctx);

    unsigned int line{0};
    const char *file{nullptr};
    const char *func{nullptr};

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    do {
        if (iter->IsScriptedFrame()) {
            line = iter->LineNumber();
            file = iter->FilePath();
            func = iter->FunctionName();
            break;
        }
        iter->Next();
    } while (!iter->Done());
    ctx->DestroyFrameIterator(iter);

    return source_loc(file, static_cast<int>(line), func);
}

[[nodiscard]] std::vector<std::string> src_helper::get_stack_trace(SourcePawn::IPluginContext *ctx) noexcept {
    assert(ctx);

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    if (iter->Done()) {
        ctx->DestroyFrameIterator(iter);
        return {};
    }

    std::vector<std::string> trace{"Call stack trace:"};

    for (int index = 0; !iter->Done(); iter->Next(), ++index) {
        if (iter->IsNativeFrame()) {
            const char *func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            trace.emplace_back(fmt_lib::format("  [{}] {}", index, func));
        } else if (iter->IsScriptedFrame()) {
            const char *func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            const char *file = iter->FilePath();
            if (!file) {
                func = "<unknown>";
            }

            trace.emplace_back(fmt_lib::format("  [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
}

// err_helper
void err_helper::handle_ex(const std::string &origin, const src_helper &src, const std::exception &ex) const noexcept {
    try {
        const source_loc loc{src.get()};
        if (custom_error_handler_) {
            custom_error_handler_->PushString(ex.what());
            custom_error_handler_->PushString(origin.c_str());
            custom_error_handler_->PushString(loc.filename);
            custom_error_handler_->PushCell(loc.line);
            custom_error_handler_->PushString(loc.funcname);
#ifndef DEBUG
            custom_error_handler_->Execute();
#else
            assert(custom_error_handler_->Execute() == SP_ERROR_NONE);
#endif
            return;
        }
        smutils->LogError(myself, "[%s::%d] [%s] %s", get_path_filename(loc.filename), loc.line, origin.c_str(), ex.what());
    } catch (const std::exception &handler_ex) {
        smutils->LogError(myself, "[%s] caught exception during error handler: %s", origin.c_str(), handler_ex.what());
    } catch (...) {
        smutils->LogError(myself, "[%s] caught unknown exception during error handler", origin.c_str());
    }
}

void err_helper::handle_unknown_ex(const std::string &origin, const src_helper &src) const noexcept {
    handle_ex(origin, src, std::runtime_error("unknown exception"));
}

void err_helper::set_err_handler(SourceMod::IChangeableForward *handler) noexcept {
    assert(handler);
    release_forward();
    custom_error_handler_ = handler;
}

err_helper::~err_helper() noexcept {
    release_forward();
}

void err_helper::release_forward() noexcept {
    if (custom_error_handler_) {
        forwards->ReleaseForward(custom_error_handler_);
        custom_error_handler_ = nullptr;
    }
}


[[nodiscard]] spdlog::filename_t unbuild_path(SourceMod::PathType type, const filename_t &filename) noexcept
{
    const char *base = nullptr;
    switch (type)
    {
    case SourceMod::PathType::Path_Game:
        base = smutils->GetGamePath();
        break;
    case SourceMod::PathType::Path_SM:
        base = smutils->GetSourceModPath();
        break;
    case SourceMod::PathType::Path_SM_Rel:
        // TODO
        break;
    default:
        break;
    }

    if (base)
    {
        return filename.substr(std::strlen(base) + 1);
    }
    return filename;
}


}       // namespace log4sp
