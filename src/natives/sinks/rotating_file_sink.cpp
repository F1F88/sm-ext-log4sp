#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


/**
 * 封装读取 rotating file sink handle 代码
 * 这会创建 1 个变量: rotatingFileSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(handle)                                             \
    std::shared_ptr<spdlog::sinks::rotating_file_sink_st> rotatingFileSink;                         \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);          \
        if (!sink)                                                                                  \
        {                                                                                           \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        rotatingFileSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(sink);   \
        if (!rotatingFileSink)                                                                      \
        {                                                                                           \
            ctx->ReportError("Invalid RotatingFileSink Handle %x.", handle);                        \
            return 0;                                                                               \
        }                                                                                           \
    }


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

    std::shared_ptr<spdlog::sinks::rotating_file_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(absPath, maxFileSize, maxFiles, rotateOnOpen, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
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
    READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], rotatingFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t GetFilenameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    return static_cast<cell_t>(rotatingFileSink->filename().length());
}

static cell_t RotateNow(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(params[1]);

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

static cell_t CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (Log4sp::LoggerHandler::Instance().FindHandle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    CTX_LOCAL_TO_STRING(params[2], &file);

    char absPath[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, absPath, sizeof(absPath), "%s", file);

    auto maxFileSize  = static_cast<std::size_t>(params[3]);
    auto maxFiles     = static_cast<std::size_t>(params[4]);
    auto rotateOnOpen = static_cast<bool>(params[5]);
    SourcePawn::IPluginFunction *openFunc  = ctx->GetFunctionById(params[6]);
    SourcePawn::IPluginFunction *closeFunc = ctx->GetFunctionById(params[7]);

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    std::shared_ptr<spdlog::sinks::rotating_file_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(absPath, maxFileSize, maxFiles, rotateOnOpen, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<Log4sp::Logger>(name, sink);
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t RotatingFileSinkNatives[] =
{
    {"RotatingFileSink.RotatingFileSink",       RotatingFileSink},
    {"RotatingFileSink.GetFilename",            GetFilename},
    {"RotatingFileSink.GetFilenameLength",      GetFilenameLength},
    {"RotatingFileSink.RotateNow",              RotateNow},

    {"RotatingFileSink.CalcFilename",           CalcFilename},
    {"RotatingFileSink.CreateLogger",           CreateLogger},

    {nullptr,                                   nullptr}
};
