#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::file_event_handlers;
using spdlog::filename_t;
using spdlog::sink_ptr;
using spdlog::sinks::rotating_file_sink_mt;
using spdlog::sinks::rotating_file_sink_st;


/**
 * 封装读取 rotating file sink handle 代码
 * 这会创建 1 个变量: rotatingFileSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(handle)                                             \
    std::shared_ptr<rotating_file_sink_st> rotatingFileSink;                                        \
    do {                                                                                            \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);        \
        if (!sink) {                                                                                \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        rotatingFileSink = std::dynamic_pointer_cast<rotating_file_sink_st>(sink);                  \
        if (!rotatingFileSink) {                                                                    \
            ctx->ReportError("Invalid RotatingFileSink Handle %x.", handle);                        \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RotatingFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t RotatingFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize  = static_cast<size_t>(params[2]);
    auto maxFiles     = static_cast<size_t>(params[3]);
    auto rotateOnOpen = static_cast<bool>(params[4]);
    SourcePawn::IPluginFunction *openPre = ctx->GetFunctionById(params[5]);
    SourcePawn::IPluginFunction *closePost = ctx->GetFunctionById(params[6]);

    file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_CALLBACK(openPre);
    handlers.after_close = FILE_EVENT_CALLBACK(closePost);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a RotatingFileSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t RotatingFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], rotatingFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t RotatingFileSink_GetFilenameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_ROTATING_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    return static_cast<cell_t>(rotatingFileSink->filename().length());
}

static cell_t RotatingFileSink_RotateNow(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

static cell_t RotatingFileSink_CalcFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[3], &file);
    auto index = static_cast<size_t>(params[4]);

    auto filename = rotating_file_sink_st::calc_filename(file, index);

    size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[1], params[2], filename.c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t RotatingFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    CTX_LOCAL_TO_STRING(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize  = static_cast<size_t>(params[3]);
    auto maxFiles     = static_cast<size_t>(params[4]);
    auto rotateOnOpen = static_cast<bool>(params[5]);
    SourcePawn::IPluginFunction *openPre = ctx->GetFunctionById(params[6]);
    SourcePawn::IPluginFunction *closePost = ctx->GetFunctionById(params[7]);

    file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_CALLBACK(openPre);
    handlers.after_close = FILE_EVENT_CALLBACK(closePost);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
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
    {"RotatingFileSink.GetFilename",            RotatingFileSink_GetFilename},
    {"RotatingFileSink.GetFilenameLength",      RotatingFileSink_GetFilenameLength},
    {"RotatingFileSink.RotateNow",              RotatingFileSink_RotateNow},

    {"RotatingFileSink.CalcFilename",           RotatingFileSink_CalcFilename},
    {"RotatingFileSink.CreateLogger",           RotatingFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};
