#include "log4sp/common.h"

namespace log4sp {

[[nodiscard]]
spdlog::source_loc source_loc_from(SourcePawn::IPluginContext *ctx) noexcept {
    using spdlog::source_loc;
    assert(ctx);

    unsigned int line = 0;
    const char *file = nullptr;
    const char *func = nullptr;

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

[[nodiscard]]
std::vector<std::string> stack_trace_info_from(SourcePawn::IPluginContext *ctx) noexcept {
    using spdlog::fmt_lib::format;
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

            trace.emplace_back(format("  [{}] {}", index, func));
        } else if (iter->IsScriptedFrame()) {
            const char *func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            const char *file = iter->FilePath();
            if (!file) {
                func = "<unknown>";
            }

            trace.emplace_back(format("  [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
}


}       // namespace log4sp
