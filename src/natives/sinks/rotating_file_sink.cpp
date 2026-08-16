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
    SourcePawn::IPluginFunction *openFunc  = ctx->GetFunctionById(params[5]);
    SourcePawn::IPluginFunction *closeFunc = ctx->GetFunctionById(params[6]);

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
        return 0;
    }

    auto rotatingFileSink = dynamic_cast<spdlog::sinks::rotating_file_sink_st*>(sink);
    if (!rotatingFileSink)
    {
        ctx->ReportError("Invalid RotatingFileSink Handle %x.", params[1]);
        return 0;
    }

    try
    {
        rotatingFileSink->rotate_now();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
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

    {"RotatingFileSink.CalcFilename",           CalcFilename},

    {nullptr,                                   nullptr}
};
