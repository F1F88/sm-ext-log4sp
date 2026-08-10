#include <cassert>

#include "log4sp/common.h"
#include "log4sp/source_helper.h"

namespace Log4sp {

[[nodiscard]]
spdlog::source_loc SrcHelper::Get() const noexcept
{
    if (!m_Loc.empty())
    {
        return m_Loc;
    }
    else if (m_Ctx)
    {
        m_Loc = GetFromPluginCtx(m_Ctx);    // 缓存以用于下一个节点(sink)也出错时
        return m_Loc;
    }

    assert(false);                        // 说明初始化的代码存在错误 (至少一项有效)
    return spdlog::source_loc();
}

[[nodiscard]]
spdlog::source_loc SrcHelper::GetFromPluginCtx(SourcePawn::IPluginContext *ctx) noexcept
{
    assert(ctx);

    unsigned int line = 0;
    const char *file = nullptr;
    const char *func = nullptr;

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    do
    {
        if (iter->IsScriptedFrame())
        {
            line = iter->LineNumber();
            file = iter->FilePath();
            func = iter->FunctionName();
            break;
        }
        iter->Next();
    } while (!iter->Done());
    ctx->DestroyFrameIterator(iter);

    return spdlog::source_loc(file, static_cast<int>(line), func);
}

[[nodiscard]]
std::vector<std::string> SrcHelper::GetStackTrace(SourcePawn::IPluginContext *ctx) noexcept
{
    assert(ctx);

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    if (iter->Done())
    {
        ctx->DestroyFrameIterator(iter);
        return {};
    }

    std::vector<std::string> trace{"Call stack trace:"};

    for (int index = 0; !iter->Done(); iter->Next(), ++index)
    {
        using spdlog::fmt_lib::format;

        if (iter->IsNativeFrame())
        {
            const char *func = iter->FunctionName();
            if (!func)
            {
                func = "<unknown function>";
            }

            trace.emplace_back(format("  [{}] {}", index, func));
        }
        else if (iter->IsScriptedFrame())
        {
            const char *func = iter->FunctionName();
            if (!func)
            {
                func = "<unknown function>";
            }

            const char *file = iter->FilePath();
            if (!file)
            {
                file = "<unknown>";
            }

            trace.emplace_back(format("  [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
}

// ErrHelper
void ErrHelper::HandleEx(const std::string &origin, const SrcHelper &src, const std::exception &ex) const noexcept
{
    try
    {
        const spdlog::source_loc loc = src.Get();
        if (m_CustomErrorHandler)
        {
            auto fwd = m_CustomErrorHandler;
            FWD_PUSH_STRING(ex.what());             // msg
            FWD_PUSH_STRING(origin.c_str());        // name
            FWD_PUSH_STRING(loc.filename);          // file
            FWD_PUSH_CELL(loc.line);                // line
            FWD_PUSH_STRING(loc.funcname);          // func
            FWD_EXECUTE();
            return;
        }
        smutils->LogError(myself, "[%s::%d] [%s] %s", FilenameFrom(loc.filename), loc.line, origin.c_str(), ex.what());
    }
    catch (const std::exception &handler_ex)
    {
        smutils->LogError(myself, "[%s] caught exception during error handler: %s", origin.c_str(), handler_ex.what());
    }
    catch (...)
    {
        smutils->LogError(myself, "[%s] caught unknown exception during error handler", origin.c_str());
    }
}

void ErrHelper::HandleUnknownEx(const std::string &origin, const SrcHelper &src) const noexcept
{
    HandleEx(origin, src, std::runtime_error("unknown exception"));
}

void ErrHelper::SetErrHandler(SourceMod::IChangeableForward *handler) noexcept
{
    assert(handler);
    ReleaseForward();
    m_CustomErrorHandler = handler;
}

ErrHelper::~ErrHelper() noexcept
{
    ReleaseForward();
}

void ErrHelper::ReleaseForward() noexcept
{
    if (m_CustomErrorHandler)
    {
        forwards->ReleaseForward(m_CustomErrorHandler);
        m_CustomErrorHandler = nullptr;
    }
}


}       // namespace Log4sp
