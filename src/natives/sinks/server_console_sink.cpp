#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::sink_ptr;
using spdlog::sinks::stdout_sink_mt;
using spdlog::sinks::stdout_sink_st;


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t ServerConsoleSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    sink_ptr sink;
    try
    {
        sink = std::make_shared<stdout_sink_st>();
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

    ctx->ReportError("SM error! Could not create server console sink handle (error: %d)", error);
    return BAD_HANDLE;
}

static cell_t ServerConsoleSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    sink_ptr sink;
    try
    {
        sink = std::make_shared<stdout_sink_st>();
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

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {"ServerConsoleSink.CreateLogger",              ServerConsoleSink_CreateLogger},

    {nullptr,                                       nullptr}
};
