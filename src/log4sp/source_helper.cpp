#include <cassert>

#include "log4sp/common.h"
#include "log4sp/source_helper.h"

namespace log4sp {

namespace fmt_lib = spdlog::fmt_lib;
using spdlog::source_loc;

[[nodiscard]] source_loc src_helper::get() const noexcept {
    if (!loc_.empty()) {
        return loc_;
    } else if (ctx_) {
        loc_ = get_from_plugin_ctx(ctx_); // 缓存以用于下一个节点(sink)也出错时
        return loc_;
    }

    assert(false);                        // 说明初始化的代码存在错误 (至少一项有效)
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
            auto forward = custom_error_handler_;
            FWD_PUSH_STRING(ex.what());             // msg
            FWD_PUSH_STRING(origin.c_str());        // name
            FWD_PUSH_STRING(loc.filename);          // file
            FWD_PUSH_CELL(loc.line);                // line
            FWD_PUSH_STRING(loc.funcname);          // func
            FWD_EXECUTE();
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


}       // namespace log4sp
