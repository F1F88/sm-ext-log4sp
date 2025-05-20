#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::file_event_handlers;
using spdlog::filename_t;
using spdlog::sink_ptr;
using spdlog::sinks::rotating_file_sink_mt;
using spdlog::sinks::rotating_file_sink_st;


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RotatingFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t RotatingFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize  = static_cast<size_t>(params[2]);
    auto maxFiles     = static_cast<size_t>(params[3]);
    auto rotateOnOpen = static_cast<bool>(params[4]);
    SourcePawn::IPluginFunction *openPre = ctx->GetFunctionById(params[5]);
    SourcePawn::IPluginFunction *closePost = ctx->GetFunctionById(params[6]);

    file_event_handlers handlers;
    handlers.before_open = [openPre](const filename_t &filename)
    {
        if (!openPre)
            return;

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create rotating file open pre forward.");

        if (!forward->AddFunction(openPre))
            log4sp::throw_log4sp_ex("SM error! Could not add rotating file open pre function.");

        auto path = log4sp::unbuild_path(SourceMod::PathType::Path_Game, filename);

        forward->PushString(path.c_str());
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif
        forwards->ReleaseForward(forward);
    };

    handlers.after_close = [closePost](const filename_t &filename)
    {
        if (!closePost)
            return;

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create rotating file close post forward.");

        if (!forward->AddFunction(closePost))
            log4sp::throw_log4sp_ex("SM error! Could not add rotating file close post function.");

        auto path = log4sp::unbuild_path(SourceMod::PathType::Path_Game, filename);

        forward->PushString(path.c_str());
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif
        forwards->ReleaseForward(forward);
    };

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

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create rotating file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t RotatingFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<rotating_file_sink_st>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid rotating file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t RotatingFileSink_GetFilenameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<rotating_file_sink_st>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid rotating file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    return static_cast<cell_t>(realSink->filename().length());
}

static cell_t RotatingFileSink_RotateNow(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<rotating_file_sink_st>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid rotating file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    try
    {
        realSink->rotate_now();
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
    ctx->LocalToString(params[3], &file);
    auto index = static_cast<size_t>(params[4]);

    auto filename = rotating_file_sink_st::calc_filename(file, index);

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[1], params[2], filename.c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t RotatingFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    auto maxFileSize  = static_cast<size_t>(params[3]);
    auto maxFiles     = static_cast<size_t>(params[4]);
    auto rotateOnOpen = static_cast<bool>(params[5]);
    SourcePawn::IPluginFunction *openPre = ctx->GetFunctionById(params[6]);
    SourcePawn::IPluginFunction *closePost = ctx->GetFunctionById(params[7]);

    file_event_handlers handlers;
    handlers.before_open = [openPre](const filename_t &filename)
    {
        if (!openPre)
            return;

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create rotating file open pre forward.");

        if (!forward->AddFunction(openPre))
            log4sp::throw_log4sp_ex("SM error! Could not add rotating file open pre function.");

        auto path = log4sp::unbuild_path(SourceMod::PathType::Path_Game, filename);

        forward->PushString(path.c_str());
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif
        forwards->ReleaseForward(forward);
    };

    handlers.after_close = [closePost](const filename_t &filename)
    {
        if (!closePost)
            return;

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create rotating file close post forward.");

        if (!forward->AddFunction(closePost))
            log4sp::throw_log4sp_ex("SM error! Could not add rotating file close post function.");

        auto path = log4sp::unbuild_path(SourceMod::PathType::Path_Game, filename);

        forward->PushString(path.c_str());
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif
        forwards->ReleaseForward(forward);
    };

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

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
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
