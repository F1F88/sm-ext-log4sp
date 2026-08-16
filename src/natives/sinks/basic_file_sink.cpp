#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


static cell_t BasicFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[1], &file);

    std::array<char, PLATFORM_MAX_PATH> absPath;
    smutils->BuildPath(Path_Game, absPath.data(), sizeof(absPath), "%s", file);

    auto truncate = static_cast<bool>(params[2]);
    SourcePawn::IPluginFunction *openFunc  = ctx->GetFunctionById(params[3]);
    SourcePawn::IPluginFunction *closeFunc = ctx->GetFunctionById(params[4]);

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    spdlog::sinks::basic_file_sink_st *sink;
    try
    {
        sink = new spdlog::sinks::basic_file_sink_st(absPath.data(), truncate, handlers);
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
        ctx->ReportError("Failed to creates a BasicFileSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t BasicFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto basicFileSink = dynamic_cast<spdlog::sinks::basic_file_sink_st*>(sink);
    if (!basicFileSink)
    {
        ctx->ReportError("Invalid BasicFileSink Handle %x.", params[1]);
        return 0;
    }

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], basicFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t BasicFileSink_Truncate(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto basicFileSink = dynamic_cast<spdlog::sinks::basic_file_sink_st*>(sink);
    if (!basicFileSink)
    {
        ctx->ReportError("Invalid BasicFileSink Handle %x.", params[1]);
        return 0;
    }

    try
    {
        basicFileSink->truncate();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

const sp_nativeinfo_t BasicFileSinkNatives[] =
{
    {"BasicFileSink.BasicFileSink",             BasicFileSink},
    {"BasicFileSink.GetFilename",               BasicFileSink_GetFilename},
    {"BasicFileSink.Truncate",                  BasicFileSink_Truncate},

    {nullptr,                                   nullptr}
};
