#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/dist_sink.h"


static cell_t DistSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = new Log4sp::Sinks::DistSink();
    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        delete sink;
        ctx->ReportError("Failed to creates a DistSink Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t GetSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto distSink = dynamic_cast<Log4sp::Sinks::DistSink*>(sink);
    if (!distSink)
    {
        ctx->ReportError("Invalid DistSink Handle %x.", params[1]);
        return 0;
    }

    auto index = static_cast<std::size_t>(params[2]);
    if (index >= distSink->Sinks().size())
    {
        ctx->ReportError("Invalid index %u. [0-%u]", index, distSink->Sinks().size());
        return 0;
    }

    // 克隆体所有权归于 native caller
    SourceMod::IdentityToken_t *owner = ctx->GetIdentity();
    security = SourceMod::HandleSecurity(owner, myself->GetIdentity());

    auto handle = distSink->SinksHandle().at(index);
    auto cloned = Log4sp::SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
    if (!cloned)
    {
        ctx->ReportError("Failed to clone Sub-Sink Handle %x (error %d)", handle, error);
        return 0;
    }
    return cloned;
}

static cell_t GetSinksLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto distSink = dynamic_cast<Log4sp::Sinks::DistSink*>(sink);
    if (!distSink)
    {
        ctx->ReportError("Invalid DistSink Handle %x.", params[1]);
        return 0;
    }

    return distSink->Sinks().size();
}

static cell_t AddSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return false;
    }

    auto distSink = dynamic_cast<Log4sp::Sinks::DistSink*>(sink);
    if (!distSink)
    {
        ctx->ReportError("Invalid DistSink Handle %x.", params[1]);
        return false;
    }

    auto handle = static_cast<SourceMod::Handle_t>(params[2]);
    auto subSink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);
    if (!subSink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", handle, error);
        return false;
    }

    // 避免无限递归
    if (distSink == dynamic_cast<Log4sp::Sinks::DistSink*>(subSink))
        return false;

    auto &sinks = distSink->Sinks();
    auto &handles = distSink->SinksHandle();

    // 避免重复添加
    auto it = std::find(sinks.begin(), sinks.end(), subSink);
    if (it != sinks.end())
        return false;

    // 克隆体所有权归于 extension
    SourceMod::IdentityToken_t *owner = myself->GetIdentity();
    security = SourceMod::HandleSecurity(owner, myself->GetIdentity());
    auto cloned = Log4sp::SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
    if (!cloned)
    {
        ctx->ReportError("Failed to clone Sink Handle %x (error %d)", handle, error);
        return false;
    }

    sinks.push_back(subSink);
    handles.push_back(cloned);

    // HACK: CallbackSink
    if (auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(subSink))
        callbackSink->TryRegisterHandle(cloned);
    return true;
}

static cell_t DropSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return false;
    }

    auto distSink = dynamic_cast<Log4sp::Sinks::DistSink*>(sink);
    if (!distSink)
    {
        ctx->ReportError("Invalid DistSink Handle %x.", params[1]);
        return false;
    }

    auto handle = static_cast<SourceMod::Handle_t>(params[2]);
    auto subSink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);
    if (!subSink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", handle, error);
        return false;
    }

    auto &sinks = distSink->Sinks();
    auto &handles = distSink->SinksHandle();

    // 按 extension 身份关闭克隆体
    security = SourceMod::HandleSecurity(myself->GetIdentity(), myself->GetIdentity());

    for (std::size_t i = 0; i < sinks.size(); ++i)
    {
        if (sinks.at(i) != subSink)
            continue;

        // 必须先摘除再释放: FreeHandle 会同步销毁 sink, 其关闭回调 (FileCloseFunction /
        // OnClose) 可能重入插件代码并再次遍历 m_Sinks, 若此时仍持有已释放的对象指针就是 UAF.
        auto cloned = handles.at(i);

        // HACK: CallbackSink
        if (auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(subSink))
            callbackSink->DropHandle(cloned);

        sinks.erase(sinks.begin() + i);
        handles.erase(handles.begin() + i);

        error = Log4sp::SinkHandler::Instance().FreeHandle(cloned, &security);
        if (error != SourceMod::HandleError::HandleError_None)
        {
            ctx->ReportError("Failed to free Sink Handle %x (error %d)", cloned, error);
            return false;
        }
        return true;
    }
    return false;
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);
    return !!sink && !!dynamic_cast<Log4sp::Sinks::DistSink*>(sink);
}

const sp_nativeinfo_t DistSinkNatives[] =
{
    {"DistSink.DistSink",                       DistSink},
    {"DistSink.GetSink",                        GetSink},
    {"DistSink.GetSinksLength",                 GetSinksLength},
    {"DistSink.AddSink",                        AddSink},
    {"DistSink.DropSink",                       DropSink},
    {"DistSink.IsValid",                        IsValid},

    {nullptr,                                   nullptr}
};
