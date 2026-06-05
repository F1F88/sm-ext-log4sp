#include <cassert>

#include "spdlog/sinks/daily_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


/**
 * 封装读取 daily file sink handle 代码
 * 这会创建 1 个变量: dailyFileSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_DAILY_FILE_SINK_HANDLE_OR_ERROR(handle)                                                \
    std::shared_ptr<spdlog::sinks::daily_file_sink_st> dailyFileSink;                               \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);          \
        if (!sink)                                                                                  \
        {                                                                                           \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        dailyFileSink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_st>(sink);         \
        if (!dailyFileSink)                                                                         \
        {                                                                                           \
            ctx->ReportError("Invalid DailyFileSink Handle %x.", handle);                           \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


#define DAILY_FILE_DEFAULT_CALCULATOR()                                                             \
    [](const spdlog::filename_t &filename, const tm &now_tm)                                        \
    {                                                                                               \
        spdlog::filename_t basename, ext;                                                           \
        std::tie(basename, ext) = spdlog::details::file_helper::split_by_extension(filename);       \
        auto relPath = spdlog::fmt_lib::format(                                                     \
            SPDLOG_FMT_STRING(                                                                      \
                SPDLOG_FILENAME_T("{}_{:04d}{:02d}{:02d}{}")),                                      \
                    basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, ext);       \
                                                                                                    \
        char absPath[PLATFORM_MAX_PATH];                                                            \
        smutils->BuildPath(Path_Game, absPath, sizeof(absPath), "%s", relPath.c_str());             \
        return spdlog::filename_t(absPath);                                                         \
    }


#define DAILY_FILE_CUSTOM_CALCULATOR(func)                                                          \
    [func](const spdlog::filename_t &filename, const tm &now_tm)                                    \
    {                                                                                               \
        char relPath[PLATFORM_MAX_PATH];                                                            \
        ke::SafeStrcpy(relPath, sizeof(relPath), filename.data());                                  \
                                                                                                    \
        tm tmp = now_tm;                                                                            \
        auto timestamp = static_cast<cell_t>(mktime(&tmp)); /* FIXME: Possible Year 2038 Problem */ \
                                                                                                    \
        /* void (char[] filename, int maxlen, int sec); */                                          \
        FWDS_CREATE_EX(nullptr, ET_Ignore, 3, nullptr, Param_String, Param_Cell, Param_Cell);       \
        FWD_ADD_FUNCTION(func);                                                                     \
        FWD_PUSH_STRING_EX(relPath, sizeof(relPath), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); \
        FWD_PUSH_CELL(sizeof(relPath));                                                             \
        FWD_PUSH_CELL(timestamp);                                                                   \
        FWD_EXECUTE();                                                                              \
        forwards->ReleaseForward(fwd);                                                              \
                                                                                                    \
        char absPath[PLATFORM_MAX_PATH];                                                            \
        smutils->BuildPath(Path_Game, absPath, sizeof(absPath), "%s", relPath);                     \
        return spdlog::filename_t(absPath);                                                         \
    }


static cell_t DailyFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[1], &file);

    int hour      = params[2];
    int minute    = params[3];
    auto truncate = static_cast<bool>(params[4]);
    auto maxFiles = static_cast<uint16_t>(params[5]);
    auto calcFunc = ctx->GetFunctionById(params[6]);
    auto openFunc = ctx->GetFunctionById(params[7]);
    auto closeFunc= ctx->GetFunctionById(params[8]);

    if (params[5] < 0 || params[5] > UINT16_MAX)
    {
        ctx->ReportError("Invalid maxFiles %d. (0-%d)", params[5], UINT16_MAX);
        return BAD_HANDLE;
    }

    spdlog::sinks::log4sp_daily_filename_calculator calculator = DAILY_FILE_DEFAULT_CALCULATOR();
    if (calcFunc)
    {
        calculator = DAILY_FILE_CUSTOM_CALCULATOR(calcFunc);
    }

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    std::shared_ptr<spdlog::sinks::daily_file_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(file, hour, minute, truncate, maxFiles, handlers, calculator);
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
        ctx->ReportError("Failed to creates a DailyFileSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_DAILY_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], dailyFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t GetFilenameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_DAILY_FILE_SINK_HANDLE_OR_ERROR(params[1]);

    return static_cast<cell_t>(dailyFileSink->filename().length());
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

    int hour      = params[3];
    int minute    = params[4];
    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = static_cast<uint16_t>(params[6]);
    auto calcFunc = ctx->GetFunctionById(params[7]);
    auto openFunc = ctx->GetFunctionById(params[8]);
    auto closeFunc= ctx->GetFunctionById(params[9]);

    if (params[6] < 0 || params[6] > UINT16_MAX)
    {
        ctx->ReportError("Invalid maxFiles %d. (0-%d)", params[6], UINT16_MAX);
        return BAD_HANDLE;
    }

    spdlog::sinks::log4sp_daily_filename_calculator calculator = DAILY_FILE_DEFAULT_CALCULATOR();
    if (calcFunc)
    {
        calculator = DAILY_FILE_CUSTOM_CALCULATOR(calcFunc);
    }

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closeFunc);

    std::shared_ptr<spdlog::sinks::daily_file_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(file, hour, minute, truncate, maxFiles, handlers, calculator);
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

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSink.DailyFileSink",             DailyFileSink},
    {"DailyFileSink.GetFilename",               GetFilename},
    {"DailyFileSink.GetFilenameLength",         GetFilenameLength},

    {"DailyFileSink.CreateLogger",              CreateLogger},

    {nullptr,                                   nullptr}
};
