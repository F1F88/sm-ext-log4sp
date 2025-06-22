#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::file_event_handlers;
using spdlog::filename_t;
using spdlog::sink_ptr;
using spdlog::sinks::basic_file_sink_mt;
using spdlog::sinks::basic_file_sink_st;


/**
 * 封装读取 daily file sink handle 代码
 * 这会创建 4 个变量: security, error, sink, basicFileSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_BASIC_FILE_SINK_HANDLE_OR_ERROR(handle)                                                \
    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                             \
    SourceMod::HandleError error;                                                                   \
    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);            \
    if (!sink) {                                                                                    \
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);                      \
        return 0;                                                                                   \
    }                                                                                               \
    auto basicFileSink = std::dynamic_pointer_cast<basic_file_sink_st>(sink);                       \
    if (!basicFileSink) {                                                                           \
        ctx->ReportError("Invalid basic file sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter); \
        return 0;                                                                                   \
    }


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                  BasicFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t BasicFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    ctx->LocalToString(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[2]);
    SourcePawn::IPluginFunction *openPre   = ctx->GetFunctionById(params[3]);
    SourcePawn::IPluginFunction *closePost = ctx->GetFunctionById(params[4]);

    file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_CALLBACK(openPre, "basic file open pre");
    handlers.after_close = FILE_EVENT_CALLBACK(closePost, "basic file close post");

    sink_ptr sink;
    try
    {
        sink = std::make_shared<basic_file_sink_st>(path, truncate, handlers);
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
        ctx->ReportError("SM error! Could not create basic file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t BasicFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_BASIC_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], basicFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t BasicFileSink_Truncate(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_BASIC_FILE_SINK_HANDLE_OR_ERROR(params[1]);

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

static cell_t BasicFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[3]);
    SourcePawn::IPluginFunction *openPre   = ctx->GetFunctionById(params[4]);
    SourcePawn::IPluginFunction *closePost = ctx->GetFunctionById(params[5]);

    file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_CALLBACK(openPre, "basic file open pre");
    handlers.after_close = FILE_EVENT_CALLBACK(closePost, "basic file close post");

    sink_ptr sink;
    try
    {
        sink = std::make_shared<basic_file_sink_st>(path, truncate, handlers);
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
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t BasicFileSinkNatives[] =
{
    {"BasicFileSink.BasicFileSink",             BasicFileSink},
    {"BasicFileSink.GetFilename",               BasicFileSink_GetFilename},
    {"BasicFileSink.Truncate",                  BasicFileSink_Truncate},

    {"BasicFileSink.CreateLogger",              BasicFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};
