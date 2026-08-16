#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"


static cell_t CallbackSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::IPlugin *logPlugin;
    if (!params[1])
    {
        logPlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        logPlugin = plsys->PluginFromHandle(params[1], &error);
        if (!logPlugin)
        {
            ctx->ReportError("Invalid log Plugin Handle %x (error %d)", params[1], error);
            return BAD_HANDLE;
        }
    }

    SourcePawn::IPluginFunction *logFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[2]))
    {
        logFunc = logPlugin->GetBaseContext()->GetFunctionById(params[2]);
        if (!logFunc)
        {
            ctx->ReportError("Invalid log function id %x.", params[2]);
            return BAD_HANDLE;
        }
    }

    SourceMod::IPlugin *flushPlugin;
    if (!params[3])
    {
        flushPlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        flushPlugin = plsys->PluginFromHandle(params[3], &error);
        if (!flushPlugin)
        {
            ctx->ReportError("Invalid flush Plugin Handle %x (error %d)", params[3], error);
            return BAD_HANDLE;
        }
    }

    SourcePawn::IPluginFunction *flushFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[4]))
    {
        flushFunc = flushPlugin->GetBaseContext()->GetFunctionById(params[4]);
        if (!flushFunc)
        {
            ctx->ReportError("Invalid flush function id %x.", params[4]);
            return BAD_HANDLE;
        }
    }

    SourceMod::IPlugin *closePlugin;
    if (!params[5])
    {
        closePlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        closePlugin = plsys->PluginFromHandle(params[5], &error);
        if (!closePlugin)
        {
            ctx->ReportError("Invalid close Plugin Handle %x (error %d)", params[5], error);
            return BAD_HANDLE;
        }
    }

    SourcePawn::IPluginFunction *closeFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[6]))
    {
        closeFunc = closePlugin->GetBaseContext()->GetFunctionById(params[6]);
        if (!closeFunc)
        {
            ctx->ReportError("Invalid close function id %x.", params[6]);
            return BAD_HANDLE;
        }
    }

    cell_t data = params[7];

    SourceMod::IChangeableForward *logFwd = nullptr;
    if (logFunc)
    {
        // bool (Sink sink, char[] error, int maxlen, const char[] logTime, SourceLoc loc,
        //       const char[] name, LogLevel lvl, const char[] msg);
        logFwd = forwards->CreateForwardEx(nullptr,
                    SourceMod::ExecType::ET_Single,
                    8,
                    nullptr,
                    SourceMod::ParamType::Param_Cell,    // sink
                    SourceMod::ParamType::Param_String,  // error
                    SourceMod::ParamType::Param_Cell,    // maxlen
                    SourceMod::ParamType::Param_String,  // logTime
                    SourceMod::ParamType::Param_Array,   // loc
                    SourceMod::ParamType::Param_String,  // name
                    SourceMod::ParamType::Param_Cell,    // lvl
                    SourceMod::ParamType::Param_String); // msg

        if (!logFwd)
        {
            ctx->ReportError("Could not create log forward.");
            return BAD_HANDLE;
        }

        if (!logFwd->AddFunction(logFunc))
        {
            forwards->ReleaseForward(logFwd);
            ctx->ReportError("Could not add log function.");
            return BAD_HANDLE;
        }
    }

    SourceMod::IChangeableForward *flushFwd = nullptr;
    if (flushFunc)
    {
        // bool (Sink sink, char[] error, int maxlen);
        flushFwd = forwards->CreateForwardEx(nullptr,
                    SourceMod::ExecType::ET_Single,
                    3,
                    nullptr,
                    SourceMod::ParamType::Param_Cell,    // sink
                    SourceMod::ParamType::Param_String,  // error
                    SourceMod::ParamType::Param_Cell);   // maxlen

        if (!flushFwd)
        {
            if (logFwd)
                forwards->ReleaseForward(logFwd);
            ctx->ReportError("Could not create flush forward.");
            return BAD_HANDLE;
        }

        if (!flushFwd->AddFunction(flushFunc))
        {
            if (logFwd)
                forwards->ReleaseForward(logFwd);
            forwards->ReleaseForward(flushFwd);
            ctx->ReportError("Could not add flush function.");
            return BAD_HANDLE;
        }
    }

    SourceMod::IChangeableForward *closeFwd = nullptr;
    if (closeFunc)
    {
        // void (any data);
        closeFwd = forwards->CreateForwardEx(nullptr,
                        SourceMod::ExecType::ET_Ignore,
                        1,
                        nullptr,
                        SourceMod::ParamType::Param_Cell);   // data
        if (!closeFwd)
        {
            if (logFwd)
                forwards->ReleaseForward(logFwd);
            if (flushFwd)
                forwards->ReleaseForward(flushFwd);
            ctx->ReportError("Could not create close forward.");
            return BAD_HANDLE;
        }

        if (!closeFwd->AddFunction(closeFunc))
        {
            if (logFwd)
                forwards->ReleaseForward(logFwd);
            if (flushFwd)
                forwards->ReleaseForward(flushFwd);
            forwards->ReleaseForward(closeFwd);
            ctx->ReportError("Could not add close function.");
            return BAD_HANDLE;
        }
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = new Log4sp::Sinks::CallbackSink(logFwd, flushFwd, closeFwd, data);
    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        delete sink;
        ctx->ReportError("Failed to creates a CallbackSink Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t SetData(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(sink);
    if (!callbackSink)
    {
        ctx->ReportError("Invalid CallbackSink Handle %x.", params[1]);
        return 0;
    }

    callbackSink->SetData(params[2]);
    return 0;
}

static cell_t GetData(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(sink);
    if (!callbackSink)
    {
        ctx->ReportError("Invalid CallbackSink Handle %x.", params[1]);
        return 0;
    }

    return callbackSink->GetData();
}

const sp_nativeinfo_t CallbackSinkNatives[] =
{
    {"CallbackSink.CallbackSink",                   CallbackSink},
    {"CallbackSink.SetData",                        SetData},
    {"CallbackSink.GetData",                        GetData},

    {nullptr,                                       nullptr}
};
