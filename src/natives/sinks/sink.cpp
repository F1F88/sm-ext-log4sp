#include "spdlog/sinks/sink.h"
#include "spdlog/pattern_formatter.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"


/**
 * 封装读取 sink handle 代码
 * 这会创建 1 个变量: sink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_SINK_HANDLE_OR_ERROR(handle)                                                           \
    spdlog::sink_ptr sink;                                                                          \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);               \
        if (!sink)                                                                                  \
        {                                                                                           \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
    }


static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    char *logTime;
    if (auto err = ctx->LocalToString(params[2], &logTime))
    {
        ctx->ReportError("Invalid log time (error %d)", err);
        return 0;
    }

    Log4sp::CellSourceLoc *loc;
    if (auto err = ctx->LocalToPhysAddr(params[3], reinterpret_cast<cell_t**>(&loc)))
    {
        ctx->ReportError("Invalid loc (error %d)", err);
        return 0;
    }

    char *name;
    if (auto err = ctx->LocalToString(params[4], &name))
    {
        ctx->ReportError("Invalid name (error %d)", err);
        return 0;
    }

    auto lvl = Log4sp::NumToLvl(params[5]);

    char *msg;
    if (auto err = ctx->LocalToString(params[6], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    using std::chrono::duration_cast;
    using std::chrono::system_clock;
    using spdlog::details::os::now;
    system_clock::time_point logTimePoint;
    try
    {
        auto nanoseconds = std::chrono::nanoseconds(std::stoull(logTime));
        logTimePoint = system_clock::time_point(duration_cast<system_clock::duration>(nanoseconds));
    }
    catch (const std::exception &)
    {
        logTimePoint = now();
    }

    try
    {
        using spdlog::details::log_msg;
        sink->log(log_msg(logTimePoint, loc->ToSourceLoc(), name, lvl, msg));
    }
    catch (const std::exception &ex)
    {
        if (auto err = ctx->StringToLocalUTF8(params[7], params[8], ex.what(), nullptr))
            ctx->ReportError("Failed to write error to buffer (error %d)", err);
        return false;
    }
    return true;
}

static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    return sink->level();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    sink->set_level(lvl);
    return 0;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    char *pattern;
    CTX_LOCAL_TO_STRING(params[2], &pattern);

    auto type = Log4sp::NumToPatternTimeType(params[3]);

    using spdlog::pattern_formatter;
    sink->set_formatter(std::make_unique<pattern_formatter>(pattern, type));
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    return sink->should_log(lvl);
}

/**
 * public native void Flush();
 */
static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandleRaw(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    try
    {
        sink->flush();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}


const sp_nativeinfo_t SinkNatives[] =
{
    {"Sink.Log",                                Log},
    {"Sink.GetLevel",                           GetLevel},
    {"Sink.SetLevel",                           SetLevel},
    {"Sink.SetPattern",                         SetPattern},
    {"Sink.ShouldLog",                          ShouldLog},
    {"Sink.Flush",                              Flush},

    {nullptr,                                   nullptr}
};
