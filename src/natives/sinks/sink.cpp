#include "spdlog/sinks/sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::sink_ptr;
using spdlog::source_loc;
using spdlog::details::log_msg;
using spdlog::details::os::now;
using spdlog::level::level_enum;


/**
 * 封装读取 sink handle 代码
 * 这会创建 1 个变量: sink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_SINK_HANDLE_OR_ERROR(handle)                                                           \
    sink_ptr sink;                                                                                  \
    do {                                                                                            \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);             \
        if (!sink) {                                                                                \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                       Sink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    return sink->level();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    sink->set_level(lvl);
    return 0;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    char *pattern;
    CTX_LOCAL_TO_STRING(params[2], &pattern);

    sink->set_pattern(pattern);
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    return sink->should_log(lvl);
}

static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    char *name, *msg, *file, *func;
    CTX_LOCAL_TO_STRING(params[2], &name);
    CTX_LOCAL_TO_STRING(params[4], &msg);
    CTX_LOCAL_TO_STRING_NULL(params[5], &file);
    CTX_LOCAL_TO_STRING_NULL(params[7], &func);

    auto lvl = log4sp::num_to_lvl(params[3]);
    int line = params[8];

    source_loc loc(file, line, func);

    using std::chrono::duration_cast;
    using std::chrono::system_clock;
    std::chrono::system_clock::time_point logTime = now();
    if (params[8] != -1)
    {
        // FIXME: Possible Year 2038 Problem
        auto seconds = std::chrono::seconds(params[8]);
        logTime = system_clock::time_point(duration_cast<system_clock::duration>(seconds));
    }

    try
    {
        sink->log(log_msg(logTime, loc, name, lvl, msg));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

static cell_t ToPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

    char *name, *msg, *file, *func;
    CTX_LOCAL_TO_STRING(params[4], &name);
    CTX_LOCAL_TO_STRING(params[6], &msg);
    CTX_LOCAL_TO_STRING_NULL(params[7], &file);
    CTX_LOCAL_TO_STRING_NULL(params[9], &func);

    auto lvl = log4sp::num_to_lvl(params[5]);
    int line = params[8];

    source_loc loc(file, line, func);

    using std::chrono::duration_cast;
    using std::chrono::system_clock;
    system_clock::time_point logTime = now();
    if (params[10] != -1)
    {
        // FIXME: Possible Year 2038 Problem
        auto seconds = std::chrono::seconds(params[10]);
        logTime = system_clock::time_point(duration_cast<system_clock::duration>(seconds));
    }

    std::string formatted;
    try
    {
        formatted = sink->to_pattern(log_msg(logTime, loc, name, lvl, msg));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return 0;
    }

    size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], formatted.c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_SINK_HANDLE_OR_ERROR(params[1]);

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
    {"Sink.GetLevel",                           GetLevel},
    {"Sink.SetLevel",                           SetLevel},
    {"Sink.SetPattern",                         SetPattern},
    {"Sink.ShouldLog",                          ShouldLog},
    {"Sink.Log",                                Log},
    {"Sink.ToPattern",                          ToPattern},
    {"Sink.Flush",                              Flush},

    {nullptr,                                   nullptr}
};
