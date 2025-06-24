#include "test_sink.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"


using log4sp::sinks::test_sink_st;
using spdlog::sink_ptr;
using spdlog::details::log_msg_buffer;


#define READ_TEST_SINK_HANDLE_OR_ERROR(handle)                                                      \
    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                             \
    SourceMod::HandleError error;                                                                   \
    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);            \
    if (!sink) {                                                                                    \
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);                      \
        return 0;                                                                                   \
    }                                                                                               \
    auto testSink = std::dynamic_pointer_cast<test_sink_st>(sink);                                  \
    if (!testSink) {                                                                                \
        ctx->ReportError("Invalid test sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter); \
        return 0;                                                                                   \
    }



///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                      TestSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t TestSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    sink_ptr sink = std::make_shared<test_sink_st>();

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create test sink handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t GetLogCount(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    return testSink->get_log_counter();
}

static cell_t GetFlushCount(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    return testSink->get_flush_counter();
}

static cell_t DrainMsgs(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] name, LogLevel lvl, const char[] msg, const char[] file,
    //       int line, const char[] func, int logTime, any data);
    FWDS_CREATE_EX(nullptr, ET_Ignore, 8, nullptr,
                   Param_String,    // name
                   Param_Cell,      // lvl
                   Param_String,    // msg
                   Param_String,    // file
                   Param_Cell,      // line
                   Param_String,    // func
                   Param_Cell,      // logTime
                   Param_Cell);     // data
    FWD_ADD_FUNCTION(function);

    auto data = params[3];

    testSink->drain_msgs(
        [forward, data](const log_msg_buffer &log_msg) {
            auto name = to_string(log_msg.logger_name);
            auto payload = to_string(log_msg.payload);
            auto logTime = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());

            FWD_PUSH_STRING(name.c_str());                          // name
            FWD_PUSH_CELL(log_msg.level);                           // lvl
            FWD_PUSH_STRING(payload.c_str());                       // msg
            FWD_PUSH_STRING(log_msg.source.filename);               // file
            FWD_PUSH_CELL(log_msg.source.line);                     // line
            FWD_PUSH_STRING(log_msg.source.funcname);               // func
            FWD_PUSH_CELL(static_cast<cell_t>(logTime.count()));    // logTime
            FWD_PUSH_CELL(data);                                    // data
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t DrainLastMsg(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] name, LogLevel lvl, const char[] msg, const char[] file,
    //       int line, const char[] func, int logTime, any data);
    FWDS_CREATE_EX(nullptr, ET_Ignore, 8, nullptr,
                   Param_String,    // name
                   Param_Cell,      // lvl
                   Param_String,    // msg
                   Param_String,    // file
                   Param_Cell,      // line
                   Param_String,    // func
                   Param_Cell,      // logTime
                   Param_Cell);     // data
    FWD_ADD_FUNCTION(function);

    auto data = params[3];

    testSink->drain_last_msg(
        [forward, data](const log_msg_buffer &log_msg) {
            auto name = to_string(log_msg.logger_name);
            auto payload = to_string(log_msg.payload);
            auto logTime = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());

            FWD_PUSH_STRING(name.c_str());                          // name
            FWD_PUSH_CELL(log_msg.level);                           // lvl
            FWD_PUSH_STRING(payload.c_str());                       // msg
            FWD_PUSH_STRING(log_msg.source.filename);               // file
            FWD_PUSH_CELL(log_msg.source.line);                     // line
            FWD_PUSH_STRING(log_msg.source.funcname);               // func
            FWD_PUSH_CELL(static_cast<cell_t>(logTime.count()));    // logTime
            FWD_PUSH_CELL(data);                                    // data
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t DrainLines(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] line, any data);
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    FWD_ADD_FUNCTION(function);

    auto data = params[3];

    testSink->drain_lines(
        [forward, data](std::string_view line) {
            FWD_PUSH_STRING(line.data());
            FWD_PUSH_CELL(data);
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t DrainLastLine(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] line, any data);
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    FWD_ADD_FUNCTION(function);

    auto data = params[3];

    testSink->drain_last_line(
        [forward, data](std::string_view line) {
            FWD_PUSH_STRING(line.data());
            FWD_PUSH_CELL(data);
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t SetLogDelay(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->set_log_delay(std::chrono::milliseconds(params[2]));
    return 0;
}

static cell_t SetFlushDelay(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->set_flush_delay(std::chrono::milliseconds(params[2]));
    return 0;
}

static cell_t SetLogException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    testSink->set_log_exception(std::runtime_error(msg));
    return 0;
}

static cell_t ClearLogException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->clear_log_exception();
    return 0;
}

static cell_t SetFlushException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    testSink->set_flush_exception(std::runtime_error(msg));
    return 0;
}

static cell_t ClearFlushException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->clear_flush_exception();
    return 0;
}

const sp_nativeinfo_t TestSinkNatives[] =
{
    {"TestSink.TestSink",                           TestSink},

    {"TestSink.GetLogCount",                        GetLogCount},
    {"TestSink.GetFlushCount",                      GetFlushCount},

    {"TestSink.DrainMsgs",                          DrainMsgs},
    {"TestSink.DrainLastMsg",                       DrainLastMsg},

    {"TestSink.DrainLines",                         DrainLines},
    {"TestSink.DrainLastLine",                      DrainLastLine},

    {"TestSink.SetLogDelay",                        SetLogDelay},
    {"TestSink.SetFlushDelay",                      SetFlushDelay},

    {"TestSink.SetLogException",                    SetLogException},
    {"TestSink.ClearLogException",                  ClearLogException},

    {"TestSink.SetFlushException",                  SetFlushException},
    {"TestSink.ClearFlushException",                ClearFlushException},

    {nullptr,                                       nullptr}
};
