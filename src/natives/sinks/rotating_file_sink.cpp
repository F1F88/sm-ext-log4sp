#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


static cell_t RotatingFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[1], &file);

    char absPath[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, absPath, sizeof(absPath), "%s", file);

    auto maxFileSize  = static_cast<std::size_t>(params[2]);
    auto maxFiles     = static_cast<std::size_t>(params[3]);
    auto rotateOnOpen = static_cast<bool>(params[4]);

    SourceMod::IPlugin *openPlugin;
    if (!params[5])
    {
        openPlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        openPlugin = plsys->PluginFromHandle(params[5], &error);
        if (!openPlugin)
        {
            ctx->ReportError("Invalid open Plugin Handle %x (error %d)", params[5], error);
            return BAD_HANDLE;
        }
    }

    SourcePawn::IPluginFunction *openFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[6]))
    {
        openFunc = openPlugin->GetBaseContext()->GetFunctionById(params[6]);
        if (!openFunc)
        {
            ctx->ReportError("Invalid open function id %x.", params[6]);
            return BAD_HANDLE;
        }
    }

    SourceMod::IPlugin *closePlugin;
    if (!params[7])
    {
        closePlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        closePlugin = plsys->PluginFromHandle(params[7], &error);
        if (!closePlugin)
        {
            ctx->ReportError("Invalid close Plugin Handle %x (error %d)", params[7], error);
            return BAD_HANDLE;
        }
    }

    SourcePawn::IPluginFunction *closeFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[8]))
    {
        closeFunc = closePlugin->GetBaseContext()->GetFunctionById(params[8]);
        if (!closeFunc)
        {
            ctx->ReportError("Invalid close function id %x.", params[8]);
            return BAD_HANDLE;
        }
    }

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    spdlog::sinks::rotating_file_sink_st *sink;
    try
    {
        sink = new spdlog::sinks::rotating_file_sink_st(absPath, maxFileSize, maxFiles, rotateOnOpen, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a RotatingFileSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return 0;
    }

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], rotatingFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t RotateNow(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return false;
    }

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return false;
    }

    try
    {
        rotatingFileSink->rotate_now();
        return true;
    }
    catch (const std::exception &ex)
    {
        if (auto err = ctx->StringToLocalUTF8(params[2], params[3], ex.what(), nullptr))
            ctx->ReportError("Failed to write error to buffer (error %d)", err);
        return false;
    }
}

static cell_t GetMaxSize(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return 0;
    }

    return static_cast<cell_t>(rotatingFileSink->get_max_size());
}

static cell_t SetMaxSize(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return false;
    }

    auto maxSize = static_cast<std::size_t>(params[2]);

    try
    {
        rotatingFileSink->set_max_size(maxSize);
        return true;
    }
    catch (const std::exception &ex)
    {
        if (auto err = ctx->StringToLocalUTF8(params[3], params[4], ex.what(), nullptr))
            ctx->ReportError("Failed to write error to buffer (error %d)", err);
        return false;
    }
}

static cell_t GetMaxFiles(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return 0;
    }

    return static_cast<cell_t>(rotatingFileSink->get_max_files());
}

static cell_t SetMaxFiles(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return false;
    }

    auto maxFiles = static_cast<std::size_t>(params[2]);

    try
    {
        rotatingFileSink->set_max_files(maxFiles);
        return true;
    }
    catch (const std::exception &ex)
    {
        if (auto err = ctx->StringToLocalUTF8(params[3], params[4], ex.what(), nullptr))
            ctx->ReportError("Failed to write error to buffer (error %d)", err);
        return false;
    }
}

static cell_t CalcFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[3], &file);
    auto index = static_cast<std::size_t>(params[4]);

    auto filename = spdlog::sinks::rotating_file_sink_st::calc_filename(file, index);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[1], params[2], filename.c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

const sp_nativeinfo_t RotatingFileSinkNatives[] =
{
    {"RotatingFileSink.RotatingFileSink",       RotatingFileSink},
    {"RotatingFileSink.GetFilename",            GetFilename},
    {"RotatingFileSink.RotateNow",              RotateNow},
    {"RotatingFileSink.GetMaxSize",             GetMaxSize},
    {"RotatingFileSink.SetMaxSize",             SetMaxSize},
    {"RotatingFileSink.GetMaxFiles",            GetMaxFiles},
    {"RotatingFileSink.SetMaxFiles",            SetMaxFiles},

    {"RotatingFileSink.CalcFilename",           CalcFilename},

    {nullptr,                                   nullptr}
};
