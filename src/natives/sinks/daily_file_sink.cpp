
#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/daily_file_sink.h"


static cell_t DailyFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    if (auto err = ctx->LocalToString(params[1], &file))
    {
        ctx->ReportError("Invalid file (error %d)", err);
        return BAD_HANDLE;
    }

    int hour      = params[2];
    int minute    = params[3];
    auto truncate = static_cast<bool>(params[4]);
    auto maxFiles = static_cast<uint16_t>(params[5]);

    if (hour < 0 || hour > 23)
    {
        ctx->ReportError("Invalid rotation hour %d. [0-23]", hour);
        return BAD_HANDLE;
    }

    if (minute < 0 || minute > 59)
    {
        ctx->ReportError("Invalid rotation minute %d. [0-59]", minute);
        return BAD_HANDLE;
    }

    if (params[5] < 0 || params[5] > UINT16_MAX)
    {
        ctx->ReportError("Invalid maxFiles %u. [0-%u]", params[5], UINT16_MAX);
        return BAD_HANDLE;
    }

    SourceMod::IPlugin *calcPlugin;
    if (!params[6])
    {
        calcPlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        calcPlugin = plsys->PluginFromHandle(params[6], &error);
        if (!calcPlugin)
        {
            ctx->ReportError("Invalid calc Plugin Handle %x (error %d)", params[6], error);
            return BAD_HANDLE;
        }
    }

    SourcePawn::IPluginFunction *calcFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[7]))
    {
        if (!calcPlugin->GetBaseContext()->GetFunctionByIdOrNull(params[7], &calcFunc))
        {
            ctx->ReportError("Invalid calc function id %x.", params[7]);
            return BAD_HANDLE;
        }
    }

    SourceMod::Handle_t calcPluginHandle = calcPlugin->GetMyHandle();
    Log4sp::Sinks::DailyFileSink::Calculator calculator = nullptr;
    if (calcFunc)
    {
        calculator = [calcPluginHandle, calcFunc](const spdlog::filename_t &filename, const tm &nowTime)
        {
            if (!plsys->PluginFromHandle(calcPluginHandle, nullptr))
            {
                smutils->LogError(myself, "Daily filename calculator function is no longer valid; revert to the default calculator.");
                return Log4sp::Sinks::DailyFileSink::DefaultCalc(filename, nowTime);
            }

            std::array<char, sizeof(Log4sp::CellSourceLoc::filename)> relPath;
            ke::SafeStrcpy(relPath.data(), sizeof(relPath), filename.data());

            tm tmp = nowTime;
            auto timestamp = Log4sp::CellLong(static_cast<std::int64_t>(mktime(&tmp)));

            /* void (char[] filename, int maxlen, LOG4SP_CONST_ES Long sec); */
            auto fwd = forwards->CreateForwardEx(nullptr,
                        SourceMod::ExecType::ET_Ignore,
                        3,
                        nullptr,
                        SourceMod::ParamType::Param_String,
                        SourceMod::ParamType::Param_Cell,
                        SourceMod::ParamType::Param_Array);
            using spdlog::throw_spdlog_ex;
            using spdlog::fmt_lib::format;

            if (!fwd)
                throw_spdlog_ex("Failed to create custom calculator forward.");

            if (!fwd->AddFunction(calcFunc))
            {
                forwards->ReleaseForward(fwd);
                throw_spdlog_ex("Failed to add calculator function.");
            }

            if (auto err = fwd->PushStringEx(relPath.data(), sizeof(relPath), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK))
            {
                forwards->ReleaseForward(fwd);
                throw_spdlog_ex(format("Failed to push filename into calculator forward (error {})", err));
            }

            if (auto err = fwd->PushCell(sizeof(relPath)))
            {
                forwards->ReleaseForward(fwd);
                throw_spdlog_ex(format("Failed to push maxlen into calculator forward (error {})", err));
            }

            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&timestamp), sizeof(timestamp) / sizeof(cell_t)))
            {
                forwards->ReleaseForward(fwd);
                throw_spdlog_ex(format("Failed to push timestamp into calculator forward (error {})", err));
            }

            if (auto err = fwd->Execute())
            {
                forwards->ReleaseForward(fwd);
                throw_spdlog_ex(format("Failed to execute file event forward (error {})", err));
            }
            forwards->ReleaseForward(fwd);

            std::array<char, PLATFORM_MAX_PATH> absPath;
            smutils->BuildPath(Path_Game, absPath.data(), sizeof(absPath), "%s", relPath.data());
            return spdlog::filename_t(absPath.data());
        };
    }

    SourceMod::IPlugin *openPlugin;
    if (!params[8])
    {
        openPlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        openPlugin = plsys->PluginFromHandle(params[8], &error);
        if (!openPlugin)
        {
            ctx->ReportError("Invalid open Plugin Handle %x (error %d)", params[8], error);
            return BAD_HANDLE;
        }
    }

    SourceMod::Handle_t openPluginHandle = openPlugin->GetMyHandle();
    SourcePawn::IPluginFunction *openFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[9]))
    {
        if (!openPlugin->GetBaseContext()->GetFunctionByIdOrNull(params[9], &openFunc))
        {
            ctx->ReportError("Invalid open function id %x.", params[9]);
            return BAD_HANDLE;
        }
    }

    SourceMod::IPlugin *closePlugin;
    if (!params[10])
    {
        closePlugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        closePlugin = plsys->PluginFromHandle(params[10], &error);
        if (!closePlugin)
        {
            ctx->ReportError("Invalid close Plugin Handle %x (error %d)", params[10], error);
            return BAD_HANDLE;
        }
    }

    SourceMod::Handle_t closePluginHandle = closePlugin->GetMyHandle();
    SourcePawn::IPluginFunction *closeFunc = nullptr;
    if (!ctx->IsNullFunctionId(params[11]))
    {
        if (!closePlugin->GetBaseContext()->GetFunctionByIdOrNull(params[11], &closeFunc))
        {
            ctx->ReportError("Invalid close function id %x.", params[11]);
            return BAD_HANDLE;
        }
    }

    spdlog::file_event_handlers handlers;
    handlers.before_open = FILE_EVENT_FUNCTION(openPluginHandle, openFunc);
    handlers.after_close = FILE_EVENT_FUNCTION(closePluginHandle, closeFunc);

    Log4sp::Sinks::DailyFileSink *sink;
    try
    {
        sink = new Log4sp::Sinks::DailyFileSink(file, hour, minute, truncate, maxFiles, handlers, std::move(calculator));
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
        delete sink;
        ctx->ReportError("Failed to creates a DailyFileSink Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto dailyFileSink = dynamic_cast<Log4sp::Sinks::DailyFileSink*>(sink);
    if (!dailyFileSink)
    {
        ctx->ReportError("Invalid DailyFileSink Handle %x.", params[1]);
        return 0;
    }

    auto filename = Log4sp::UnbuildPath<SourceMod::PathType::Path_Game>(dailyFileSink->Filename());

    std::size_t bytes = 0;
    if (auto err = ctx->StringToLocalUTF8(params[2], params[3], filename.c_str(), &bytes))
    {
        ctx->ReportError("Failed to write filename to buffer (error %d)", err);
        return 0;
    }
    return static_cast<cell_t>(bytes);
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);
    return !!sink && !!dynamic_cast<Log4sp::Sinks::DailyFileSink*>(sink);
}

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSink.DailyFileSink",             DailyFileSink},
    {"DailyFileSink.GetFilename",               GetFilename},
    {"DailyFileSink.IsValid",                   IsValid},

    {nullptr,                                   nullptr}
};
