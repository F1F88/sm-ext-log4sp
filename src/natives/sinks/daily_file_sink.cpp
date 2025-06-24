#include <cassert>

#include "spdlog/sinks/daily_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::file_event_handlers;
using spdlog::filename_t;
using spdlog::sink_ptr;
using spdlog::details::file_helper;
using spdlog::fmt_lib::format;
using spdlog::sinks::daily_file_sink_mt;
using spdlog::sinks::daily_file_sink_st;
using spdlog::sinks::log4sp_daily_filename_calculator;


/**
 * 封装读取 daily file sink handle 代码
 * 这会创建 1 个变量: dailyFileSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_DAILY_FIEL_SINK_HANDLE_OR_ERROR(handle)                                                \
    std::shared_ptr<daily_file_sink_st> dailyFileSink;                                              \
    do {                                                                                            \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);        \
        if (!sink) {                                                                                \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        dailyFileSink = std::dynamic_pointer_cast<daily_file_sink_st>(sink);                        \
        if (!dailyFileSink) {                                                                       \
            ctx->ReportError("Invalid DailyFileSink Handle %x.", handle);                           \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


#define DAILY_FILE_DEFAULT_CALCULATOR()                                                             \
    [](const filename_t &filename, const tm &now_tm) {                                              \
        filename_t basename, ext;                                                                   \
        std::tie(basename, ext) = file_helper::split_by_extension(filename);                        \
        auto buffer = format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}_{:04d}{:02d}{:02d}{}")),       \
                    basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, ext);       \
        char path[PLATFORM_MAX_PATH];                                                               \
        smutils->BuildPath(Path_Game, path, sizeof(path), buffer.c_str());                          \
        return filename_t(path);                                                                    \
    }


#define DAILY_FILE_CUSTOM_CALCULATOR(function)                                                      \
    [function](const filename_t &filename, const tm &now_tm) {                                      \
        char buffer[PLATFORM_MAX_PATH];                                                             \
        size_t size = filename.size();                                                              \
        if (size > sizeof(buffer) - 1) {                                                            \
            size = sizeof(buffer) - 1;                                                              \
        }                                                                                           \
        memcpy(buffer, filename.data(), size);                                                      \
        buffer[size] = '\0';                                                                        \
                                                                                                    \
        tm tmp = now_tm;                                                                            \
        auto stamp = static_cast<cell_t>(mktime(&tmp));                                             \
                                                                                                    \
        /* void (char[] filename, int maxlen, int sec); */                                          \
        FWDS_CREATE_EX(nullptr, ET_Ignore, 3, nullptr, Param_String, Param_Cell, Param_Cell);       \
        FWD_ADD_FUNCTION(function);                                                                 \
        FWD_PUSH_STRING_EX(buffer, sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); \
        FWD_PUSH_CELL(sizeof(buffer));                                                              \
        FWD_PUSH_CELL(stamp);                                                                       \
        FWD_EXECUTE();                                                                              \
        forwards->ReleaseForward(forward);                                                          \
                                                                                                    \
        smutils->BuildPath(Path_Game, buffer, sizeof(buffer), buffer);                              \
        return filename_t(buffer);                                                                  \
    }


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   DailyFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t DailyFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    CTX_LOCAL_TO_STRING(params[1], &file);

    int hour      = params[2];
    int minute    = params[3];
    auto truncate = static_cast<bool>(params[4]);
    auto maxFiles = params[5];
    auto function = ctx->GetFunctionById(params[6]);
    auto openPre  = ctx->GetFunctionById(params[7]);
    auto closePost= ctx->GetFunctionById(params[8]);

    if (maxFiles < 0 || maxFiles > UINT16_MAX)
    {
        ctx->ReportError("Invalid maxFiles %d. (0-%d)", maxFiles, UINT16_MAX);
        return BAD_HANDLE;
    }

    log4sp_daily_filename_calculator calculator = DAILY_FILE_DEFAULT_CALCULATOR();
    if (function)
    {
        calculator = DAILY_FILE_CUSTOM_CALCULATOR(function);
    }

    file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_CALLBACK(openPre);
    handlers.after_close = FILE_EVENT_CALLBACK(closePost);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<daily_file_sink_st>(file, hour, minute, truncate, static_cast<uint16_t>(maxFiles), handlers, calculator);
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
        ctx->ReportError("Failed to creates a DailyFileSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t DailyFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_DAILY_FIEL_SINK_HANDLE_OR_ERROR(params[1]);

    size_t bytes{0};
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], dailyFileSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t DailyFileSink_GetFilenameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_DAILY_FIEL_SINK_HANDLE_OR_ERROR(params[1]);

    return static_cast<cell_t>(dailyFileSink->filename().length());
}

static cell_t DailyFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    int hour      = params[3];
    int minute    = params[4];
    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = params[6];
    auto function = ctx->GetFunctionById(params[7]);
    auto openPre  = ctx->GetFunctionById(params[8]);
    auto closePost= ctx->GetFunctionById(params[9]);

    if (maxFiles < 0 || maxFiles > UINT16_MAX)
    {
        ctx->ReportError("Invalid maxFiles %d. (0-%d)", maxFiles, UINT16_MAX);
        return BAD_HANDLE;
    }

    log4sp_daily_filename_calculator calculator = DAILY_FILE_DEFAULT_CALCULATOR();
    if (function)
    {
        calculator = DAILY_FILE_CUSTOM_CALCULATOR(function);
    }

    file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_CALLBACK(openPre);
    handlers.after_close = FILE_EVENT_CALLBACK(closePost);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<daily_file_sink_st>(file, hour, minute, truncate, static_cast<uint16_t>(maxFiles), handlers, calculator);
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

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSink.DailyFileSink",             DailyFileSink},
    {"DailyFileSink.GetFilename",               DailyFileSink_GetFilename},
    {"DailyFileSink.GetFilenameLength",         DailyFileSink_GetFilenameLength},

    {"DailyFileSink.CreateLogger",              DailyFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};
