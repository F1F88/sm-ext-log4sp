#include "test_sink.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


#define READ_TEST_SINK_HANDLE_OR_ERROR(handle)                                                      \
    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                             \
    SourceMod::HandleError error;                                                                   \
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);              \
    if (!sink)                                                                                      \
    {                                                                                               \
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);                      \
        return 0;                                                                                   \
    }                                                                                               \
    auto testSink = std::dynamic_pointer_cast<Log4sp::Sinks::TestSinkST>(sink);                     \
    if (!testSink)                                                                                  \
    {                                                                                               \
        ctx->ReportError("Invalid test sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);\
        return 0;                                                                                   \
    }



static cell_t TestSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto sink = std::make_shared<Log4sp::Sinks::TestSinkST>();

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
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

    return testSink->GetLogCounter();
}

static cell_t GetFlushCount(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    return testSink->GetFlushCounter();
}

static cell_t DrainMsgs(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
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
    FWD_ADD_FUNCTION(func);

    auto data = params[3];

    testSink->DrainMsgs(
        [fwd, data](const spdlog::details::log_msg_buffer &logMsg)
        {
            auto name = to_string(logMsg.logger_name);
            auto payload = to_string(logMsg.payload);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(logMsg.time.time_since_epoch());
            auto logTime = static_cast<cell_t>(seconds.count());    // FIXME: Possible Year 2038 Problem
            auto file    = logMsg.source.filename ? logMsg.source.filename : "";
            auto func    = logMsg.source.funcname ? logMsg.source.funcname : "";

            FWD_PUSH_STRING(name.c_str());                          // name
            FWD_PUSH_CELL(logMsg.level);                            // lvl
            FWD_PUSH_STRING(payload.c_str());                       // msg
            FWD_PUSH_STRING(file);                                  // file
            FWD_PUSH_CELL(logMsg.source.line);                      // line
            FWD_PUSH_STRING(func);                                  // func
            FWD_PUSH_CELL(logTime);                                 // logTime
            FWD_PUSH_CELL(data);                                    // data
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

static cell_t DrainLastMsg(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
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
    FWD_ADD_FUNCTION(func);

    auto data = params[3];

    testSink->DrainLastMsgs(
        [fwd, data](const spdlog::details::log_msg_buffer &log_msg)
        {
            auto name = to_string(log_msg.logger_name);
            auto payload = to_string(log_msg.payload);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());
            auto logTime = static_cast<cell_t>(seconds.count());    // FIXME: Possible Year 2038 Problem
            auto file    = log_msg.source.filename ? log_msg.source.filename : "";
            auto func    = log_msg.source.funcname ? log_msg.source.funcname : "";

            FWD_PUSH_STRING(name.c_str());                          // name
            FWD_PUSH_CELL(log_msg.level);                           // lvl
            FWD_PUSH_STRING(payload.c_str());                       // msg
            FWD_PUSH_STRING(file);                                  // file
            FWD_PUSH_CELL(log_msg.source.line);                     // line
            FWD_PUSH_STRING(func);                                  // func
            FWD_PUSH_CELL(logTime);                                 // logTime
            FWD_PUSH_CELL(data);                                    // data
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

static cell_t DrainLines(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] line, any data);
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    FWD_ADD_FUNCTION(func);

    auto data = params[3];

    testSink->DrainLines(
        [fwd, data](std::string_view line)
        {
            FWD_PUSH_STRING(line.data());
            FWD_PUSH_CELL(data);
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

static cell_t DrainLastLine(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] line, any data);
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    FWD_ADD_FUNCTION(func);

    auto data = params[3];

    testSink->DrainLastLines(
        [fwd, data](std::string_view line)
        {
            FWD_PUSH_STRING(line.data());
            FWD_PUSH_CELL(data);
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

static cell_t SetLogDelay(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->SetLogDelay(std::chrono::milliseconds(params[2]));
    return 0;
}

static cell_t SetFlushDelay(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->SetFlushDelay(std::chrono::milliseconds(params[2]));
    return 0;
}

static cell_t SetLogException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    testSink->SetLogException(std::runtime_error(msg));
    return 0;
}

static cell_t ClearLogException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->ClearLogException();
    return 0;
}

static cell_t SetFlushException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    testSink->SetFlushException(std::runtime_error(msg));
    return 0;
}

static cell_t ClearFlushException(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_TEST_SINK_HANDLE_OR_ERROR(params[1]);

    testSink->ClearFlushException();
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
