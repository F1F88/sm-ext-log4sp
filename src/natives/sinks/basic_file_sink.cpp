#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::file_event_handlers;
using spdlog::filename_t;
using spdlog::sink_ptr;
using spdlog::sinks::basic_file_sink_mt;
using spdlog::sinks::basic_file_sink_st;


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
    handlers.before_open = [openPre](const filename_t &filename)
    {
        if (!openPre)
            return;

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create basic file open pre forward.");

        if (!forward->AddFunction(openPre))
            log4sp::throw_log4sp_ex("SM error! Could not add basic file open pre function.");

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
            log4sp::throw_log4sp_ex("SM error! Could not create basic file close post forward.");

        if (!forward->AddFunction(closePost))
            log4sp::throw_log4sp_ex("SM error! Could not add basic file close post function.");

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
        sink = std::make_shared<basic_file_sink_st>(path, truncate, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    if (auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error))
        return handle;

    ctx->ReportError("SM error! Could not create base file sink handle (error: %d)", error);
    return BAD_HANDLE;
}

static cell_t BasicFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    size_t bytes{0};
    if (auto realSink = std::dynamic_pointer_cast<basic_file_sink_st>(sink))
    {
        ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
        return static_cast<cell_t>(bytes);
    }

    if (auto realSink = std::dynamic_pointer_cast<basic_file_sink_mt>(sink))
    {
        ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
        return static_cast<cell_t>(bytes);
    }

    ctx->ReportError("Invalid basic file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

static cell_t BasicFileSink_Truncate(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<basic_file_sink_st>(sink))
    {
        try
        {
            realSink->truncate();
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<basic_file_sink_mt>(sink))
    {
        try
        {
            realSink->truncate();
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    ctx->ReportError("Invalid basic file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
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
    handlers.before_open = [openPre](const filename_t &filename)
    {
        if (!openPre)
            return;

        auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
        if (!forward)
            log4sp::throw_log4sp_ex("SM error! Could not create basic file open pre forward.");

        if (!forward->AddFunction(openPre))
            log4sp::throw_log4sp_ex("SM error! Could not add basic file open pre function.");

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
            log4sp::throw_log4sp_ex("SM error! Could not create basic file close post forward.");

        if (!forward->AddFunction(closePost))
            log4sp::throw_log4sp_ex("SM error! Could not add basic file close post function.");

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
        sink = std::make_shared<basic_file_sink_st>(path, truncate, handlers);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    if (auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error))
        return handle;

    ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
    return BAD_HANDLE;

}

const sp_nativeinfo_t BasicFileSinkNatives[] =
{
    {"BasicFileSink.BasicFileSink",             BasicFileSink},
    {"BasicFileSink.GetFilename",               BasicFileSink_GetFilename},
    {"BasicFileSink.Truncate",                  BasicFileSink_Truncate},

    {"BasicFileSink.CreateLogger",              BasicFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};
