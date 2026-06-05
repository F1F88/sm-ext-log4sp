#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


/**
 * 封装读取 basic file sink handle 代码
 * 这会创建 1 个变量: basicFileSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_BASIC_FILE_SINK_HANDLE_OR_ERROR(handle)                                                \
    std::shared_ptr<spdlog::sinks::basic_file_sink_st> basicFileSink;                               \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);          \
        if (!sink)                                                                                  \
        {                                                                                           \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        basicFileSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sink);         \
        if (!basicFileSink)                                                                         \
        {                                                                                           \
            ctx->ReportError("Invalid BasicFileSink Handle %x.", handle);                           \
            return 0;                                                                               \
        }                                                                                           \
    }


static cell_t BasicFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[1], &file);

    char absPath[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, absPath, sizeof(absPath), "%s", file);

    auto truncate = static_cast<bool>(params[2]);
    SourcePawn::IPluginFunction *openFunc  = ctx->GetFunctionById(params[3]);
    SourcePawn::IPluginFunction *closeFunc = ctx->GetFunctionById(params[4]);

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    std::shared_ptr<spdlog::sinks::basic_file_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(absPath, truncate, handlers);
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
        ctx->ReportError("Failed to creates a BasicFileSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t BasicFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_BASIC_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], basicFileSink->filename().c_str(), &bytes);
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

    auto truncate = static_cast<bool>(params[3]);
    SourcePawn::IPluginFunction *openFunc  = ctx->GetFunctionById(params[4]);
    SourcePawn::IPluginFunction *closeFunc = ctx->GetFunctionById(params[5]);

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    std::shared_ptr<spdlog::sinks::basic_file_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(absPath, truncate, handlers);
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

const sp_nativeinfo_t BasicFileSinkNatives[] =
{
    {"BasicFileSink.BasicFileSink",             BasicFileSink},
    {"BasicFileSink.GetFilename",               BasicFileSink_GetFilename},
    {"BasicFileSink.Truncate",                  BasicFileSink_Truncate},

    {"BasicFileSink.CreateLogger",              BasicFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};
