#include <cassert>

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/daily_file_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   DailyFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native DailyFileSink(const char[] file,
 *                             int hour = 0,
 *                             int minute = 0,
 *                             bool truncate = false,
 *                             int maxFiles = 0,
 *                             DailyFileCalculator callback = INVALID_FUNCTION);
 */
static cell_t DailyFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    ctx->LocalToString(params[1], &file);

    auto hour     = static_cast<int>(params[2]);
    auto minute   = static_cast<int>(params[3]);
    auto truncate = static_cast<bool>(params[4]);
    auto maxFiles = static_cast<uint16_t>(params[5]);
    auto function = ctx->GetFunctionById(static_cast<funcid_t>(params[6]));

    auto calculator = [function](const log4sp::filename_t &filename, const tm &now_tm)
    {
        if (!function)
        {
            auto buffer = log4sp::sinks::daily_filename_default_calculator(filename, now_tm);

            char path[PLATFORM_MAX_PATH];
            smutils->BuildPath(Path_Game, path, sizeof(path), buffer.c_str());
            return log4sp::filename_t{path};
        }

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 3, nullptr, Param_String, Param_Cell, Param_Cell);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create daily file calculator forward.");

        if (!forward->AddFunction(function))
            log4sp::throw_log4sp_ex("SM error! Could not add daily file calculator function.");

        char buffer[PLATFORM_MAX_PATH];
        size_t len{(std::min)(sizeof(buffer) - 1, filename.size())};
        memcpy(buffer, filename.data(), len);
        buffer[len] = '\0';

        tm tmp{now_tm};
        auto stamp = static_cast<cell_t>(mktime(&tmp));

        forward->PushStringEx(buffer, sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK);
        forward->PushCell(sizeof(buffer));
        forward->PushCell(stamp);
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif

        forwards->ReleaseForward(forward);
        smutils->BuildPath(Path_Game, buffer, sizeof(buffer), buffer);
        return log4sp::filename_t{buffer};
    };

    log4sp::sink_ptr sink;
    try
    {
        sink = std::make_shared<log4sp::sinks::daily_file_sink>(file, hour, minute, truncate, maxFiles, calculator);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create daily file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::daily_file_sink>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid daily file sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

/**
 * public static native Logger CreateLogger(const char[] name, const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0);
 */
static cell_t DailyFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    auto hour     = static_cast<int>(params[3]);
    auto minute   = static_cast<int>(params[4]);
    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = static_cast<uint16_t>(params[6]);
    auto function = ctx->GetFunctionById(static_cast<funcid_t>(params[7]));

    auto calculator = [function](const log4sp::filename_t &filename, const tm &now_tm)
    {
        if (!function)
        {
            auto buffer = log4sp::sinks::daily_filename_default_calculator(filename, now_tm);

            char path[PLATFORM_MAX_PATH];
            smutils->BuildPath(Path_Game, path, sizeof(path), buffer.c_str());
            return log4sp::filename_t{path};
        }

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 3, nullptr, Param_String, Param_Cell, Param_Cell);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create daily file calculator forward.");

        if (!forward->AddFunction(function))
            log4sp::throw_log4sp_ex("SM error! Could not add daily file calculator function.");

        char buffer[PLATFORM_MAX_PATH];
        auto len = (std::min)(sizeof(buffer) - 1, filename.size());
        memcpy(buffer, filename.data(), len);
        buffer[len] = '\0';

        tm tmp{now_tm};
        auto stamp = static_cast<cell_t>(mktime(&tmp));

        forward->PushStringEx(buffer, sizeof(buffer), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK);
        forward->PushCell(sizeof(buffer));
        forward->PushCell(stamp);
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif

        forwards->ReleaseForward(forward);
        smutils->BuildPath(Path_Game, buffer, sizeof(buffer), buffer);
        return log4sp::filename_t{buffer};
    };

    log4sp::sink_ptr sink;
    try
    {
        sink = std::make_shared<log4sp::sinks::daily_file_sink>(file, hour, minute, truncate, maxFiles, calculator);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSink.DailyFileSink",             DailyFileSink},
    {"DailyFileSink.GetFilename",               DailyFileSink_GetFilename},

    {"DailyFileSink.CreateLogger",              DailyFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};
