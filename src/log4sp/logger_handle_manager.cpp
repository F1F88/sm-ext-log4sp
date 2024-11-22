#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <log4sp/logger_handle_manager.h>


namespace log4sp {

logger_handle_manager &logger_handle_manager::instance()
{
    static logger_handle_manager singleInstance;
    return singleInstance;
}

logger_handle_data* logger_handle_manager::get_data(const std::string &logger_name)
{
    auto found = logger_datas_.find(logger_name);
    return found != logger_datas_.end() ? found->second : nullptr;
}

HandleType_t logger_handle_manager::get_handle_type(Handle_t handle)
{
    return g_LoggerHandleType;
}

spdlog::logger* logger_handle_manager::read_handle(IPluginContext *ctx, Handle_t handle)
{
    auto type = get_handle_type(handle);
    if (type == NO_HANDLE_TYPE)
    {
        ctx->ReportError("Unable to identify the type of logger handle. (hdl=0x%x)", handle);
        return nullptr;
    }

    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    spdlog::logger *logger;

    auto error = handlesys->ReadHandle(handle, type, &sec, (void **)&logger);
    if (error != HandleError_None)
    {
        ctx->ReportError("Invalid logger handle. (hdl=0x%x, err=%d)", handle, error);
        return nullptr;
    }

    return logger;
}

void logger_handle_manager::drop(const std::string &logger_name)
{
    auto found = logger_datas_.find(logger_name);
    if (found != logger_datas_.end())
    {
        delete found->second;
        logger_datas_.erase(found);
    }
}

void logger_handle_manager::drop_all()
{
    for (auto it = logger_datas_.begin(); it != logger_datas_.end(); ++it)
    {
        delete it->second;
        logger_datas_.erase(it);
    }
}

void logger_handle_manager::shutdown()
{
    drop_all();
}


logger_handle_data* logger_handle_manager::create_logger_st(IPluginContext *ctx, std::string name, std::vector<spdlog::sink_ptr> sinks)
{
    // note: logger name 不可重复

    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, false, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded logger handle created successfully.");
    return data;
}

logger_handle_data*  logger_handle_manager::create_logger_mt(IPluginContext *ctx, std::string name, std::vector<spdlog::sink_ptr> sinks, spdlog::async_overflow_policy policy)
{
    // note: logger name 不可重复
    // note: policy 越界不会异常，但保证参数合法是应该的

    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinks);
    auto logger = std::make_shared<spdlog::async_logger>(name, dist_sink, spdlog::thread_pool(), policy);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, true, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded logger handle created successfully.");
    return data;
}


logger_handle_data* logger_handle_manager::create_server_console_logger_st(IPluginContext *ctx, std::string name)
{
    // note: logger name 不可重复

    auto sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    auto logger = std::make_shared<spdlog::logger>(name, sink);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded stdout logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, false, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded stdout logger handle created successfully.");
    return data;
}

logger_handle_data* logger_handle_manager::create_server_console_logger_mt(IPluginContext *ctx, std::string name, spdlog::async_overflow_policy policy)
{
    // note: logger name 不可重复
    // note: policy 越界不会异常，但保证参数合法是应该的

    auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    auto sinks = std::vector<spdlog::sink_ptr>{sink};
    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinks);
    auto logger = std::make_shared<spdlog::async_logger>(name, dist_sink, spdlog::thread_pool(), policy);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded stdout logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, true, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded stdout logger handle created successfully.");
    return data;
}


logger_handle_data* logger_handle_manager::create_base_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, bool truncate)
{
    // note: logger name 不可重复

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filename, truncate);
    auto logger = std::make_shared<spdlog::logger>(name, sink);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded base file logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, false, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded base file logger handle created successfully.");
    return data;
}

logger_handle_data* logger_handle_manager::create_base_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, bool truncate, spdlog::async_overflow_policy policy)
{
    // note: logger name 不可重复
    // note: policy 越界不会异常，但保证参数合法是应该的

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);
    auto sinks = std::vector<spdlog::sink_ptr>{sink};
    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinks);
    auto logger = std::make_shared<spdlog::async_logger>(name, dist_sink, spdlog::thread_pool(), policy);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded base file logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, true, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded base file logger handle created successfully.");
    return data;
}


logger_handle_data* logger_handle_manager::create_rotating_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, size_t max_file_size, size_t max_files, bool rotate_on_open)
{
    // note: logger name 不可重复

    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(filename, max_file_size, max_files, rotate_on_open);
    auto logger = std::make_shared<spdlog::logger>(name, sink);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded rotating file logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, false, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded rotating file logger handle created successfully.");
    return data;
}

logger_handle_data* logger_handle_manager::create_rotating_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, size_t max_file_size, size_t max_files, bool rotate_on_open, spdlog::async_overflow_policy policy)
{
    // note: logger name 不可重复
    // note: policy 越界不会异常，但保证参数合法是应该的

    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, max_file_size, max_files, rotate_on_open);
    auto sinks = std::vector<spdlog::sink_ptr>{sink};
    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinks);
    auto logger = std::make_shared<spdlog::async_logger>(name, dist_sink, spdlog::thread_pool(), policy);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded rotating file logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, true, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded rotating file logger handle created successfully.");
    return data;
}


logger_handle_data* logger_handle_manager::create_daily_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, int hour, int minute, bool truncate, uint16_t max_files)
{
    // note: logger name 不可重复

    auto sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(filename, hour, minute, truncate, max_files);
    auto logger = std::make_shared<spdlog::logger>(name, sink);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded daily file logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, false, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded daily file logger handle created successfully.");
    return data;
}

logger_handle_data* logger_handle_manager::create_daily_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, int hour, int minute, bool truncate, uint16_t max_files, spdlog::async_overflow_policy policy)
{
    // note: logger name 不可重复
    // note: policy 越界不会异常，但保证参数合法是应该的

    auto sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, hour, minute, truncate, max_files);
    auto sinks = std::vector<spdlog::sink_ptr>{sink};
    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinks);
    auto logger = std::make_shared<spdlog::async_logger>(name, dist_sink, spdlog::thread_pool(), policy);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded daily file logger handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new logger_handle_data{logger, true, handle, type, nullptr};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded daily file logger handle created successfully.");
    return data;
}


void logger_handle_manager::register_logger_handle_(const std::string &key, log4sp::logger_handle_data *data)
{
    logger_datas_.insert(std::make_pair(key, data));
}


} // namespace log4sp
