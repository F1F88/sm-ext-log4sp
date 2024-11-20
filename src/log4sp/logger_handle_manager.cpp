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

logger_handle_data logger_handle_manager::get_data(const std::string &logger_name)
{
    auto found = logger_datas_.find(logger_name);
    return found != logger_datas_.end() ? found->second : logger_handle_data{};
}

// logger_handle_data logger_handle_manager::get_data(IPluginContext *ctx, Handle_t handle)
// {
//     auto logger = read_handle(ctx, handle);
//     return logger != nullptr ? get_data(logger->name()) : logger_handle_data{};
// }

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
    if (logger_datas_.erase(logger_name))
    {
        SPDLOG_ERROR("The handle data of logger name does not exist! (name='{}')", logger_name);
        return;
    }
}

// void logger_handle_manager::drop(IPluginContext *ctx, Handle_t handle)
// {
//     auto logger = read_handle(ctx, handle);
//     if (logger == nullptr)
//     {
//         SPDLOG_ERROR("The logger data of handle does not exist! (hdl={})", handle);
//         return;
//     }
//     drop(logger->name());
// }

void logger_handle_manager::drop_all()
{
    logger_datas_.clear();
}

void logger_handle_manager::shutdown()
{
    drop_all();
}


logger_handle_data logger_handle_manager::create_logger_st(IPluginContext *ctx, std::string name, std::vector<spdlog::sink_ptr> sinks)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of single threaded logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, false, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded logger handle created successfully.");
    return data;
}

logger_handle_data logger_handle_manager::create_logger_mt(IPluginContext *ctx, std::string name, std::vector<spdlog::sink_ptr> sinks, spdlog::async_overflow_policy policy)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    auto logger = std::make_shared<spdlog::async_logger>(name, sinks.begin(), sinks.end(), spdlog::thread_pool(), policy);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of multi threaded logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, true, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded logger handle created successfully.");
    return data;
}


logger_handle_data logger_handle_manager::create_server_console_logger_st(IPluginContext *ctx, std::string name)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    auto logger = spdlog::stdout_logger_st(name);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of single threaded stdout logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, false, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded stdout logger handle created successfully.");
    return data;
}

logger_handle_data logger_handle_manager::create_server_console_logger_mt(IPluginContext *ctx, std::string name, spdlog::async_overflow_policy policy)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    std::shared_ptr<spdlog::logger> logger;
    switch (policy)
    {
    case spdlog::async_overflow_policy::overrun_oldest:
        logger = spdlog::stdout_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::overrun_oldest>>(name);
        break;
    case spdlog::async_overflow_policy::discard_new:
        logger = spdlog::stdout_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::discard_new>>(name);
        break;
    default:
        logger = spdlog::stdout_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::block>>(name);
        break;
    }

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    auto handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of multi threaded stdout logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, true, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded stdout logger handle created successfully.");
    return data;
}


logger_handle_data logger_handle_manager::create_base_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, bool truncate)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    auto logger = spdlog::basic_logger_st(name, filename, truncate);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of single threaded base file logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, false, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded base file logger handle created successfully.");
    return data;
}

logger_handle_data logger_handle_manager::create_base_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, bool truncate, spdlog::async_overflow_policy policy)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    std::shared_ptr<spdlog::logger> logger;
    switch (policy)
    {
    case spdlog::async_overflow_policy::overrun_oldest:
        logger = spdlog::basic_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::overrun_oldest>>(name, filename, truncate);
        break;
    case spdlog::async_overflow_policy::discard_new:
        logger = spdlog::basic_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::discard_new>>(name, filename, truncate);
        break;
    default:
        logger = spdlog::basic_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::block>>(name, filename, truncate);
        break;
    }

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of multi threaded base file logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, true, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded base file logger handle created successfully.");
    return data;
}


logger_handle_data logger_handle_manager::create_rotating_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, size_t max_file_size, size_t max_files, bool rotate_on_open)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    auto logger = spdlog::rotating_logger_st(name, filename, max_file_size, max_files, rotate_on_open);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of single threaded rotating file logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, false, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded rotating file logger handle created successfully.");
    return data;
}

logger_handle_data logger_handle_manager::create_rotating_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, size_t max_file_size, size_t max_files, bool rotate_on_open, spdlog::async_overflow_policy policy)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    std::shared_ptr<spdlog::logger> logger;
    switch (policy)
    {
    case spdlog::async_overflow_policy::overrun_oldest:
        logger = spdlog::rotating_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::overrun_oldest>>(name, filename, max_file_size, max_files, rotate_on_open);
        break;
    case spdlog::async_overflow_policy::discard_new:
        logger = spdlog::rotating_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::discard_new>>(name, filename, max_file_size, max_files, rotate_on_open);
        break;
    default:
        logger = spdlog::rotating_logger_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::block>>(name, filename, max_file_size, max_files, rotate_on_open);
        break;
    }

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of multi threaded rotating file logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, true, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded rotating file logger handle created successfully.");
    return data;
}


logger_handle_data logger_handle_manager::create_daily_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, int hour, int minute, bool truncate, uint16_t max_files)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    auto logger = spdlog::daily_logger_format_st(name, filename, hour, minute, truncate, max_files);

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of single threaded daily file logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, false, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Single threaded daily file logger handle created successfully.");
    return data;
}

logger_handle_data logger_handle_manager::create_daily_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, int hour, int minute, bool truncate, uint16_t max_files, spdlog::async_overflow_policy policy)
{
    // 把基本的检查前置到 native 里以提高性能
    // if (logger_datas_.find(name) != logger_datas_.end())
    // {
    //     ctx->ReportError("Logger with name '%s' already exists.", name);
    //     return logger_handle_data{};
    // }

    std::shared_ptr<spdlog::logger> logger;
    switch (policy)
    {
    case spdlog::async_overflow_policy::overrun_oldest:
        logger = spdlog::daily_logger_format_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::overrun_oldest>>(name, filename, hour, minute, truncate, max_files);
        break;
    case spdlog::async_overflow_policy::discard_new:
        logger = spdlog::daily_logger_format_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::discard_new>>(name, filename, hour, minute, truncate, max_files);
        break;
    default:
        logger = spdlog::daily_logger_format_mt<spdlog::async_factory_impl<spdlog::async_overflow_policy::block>>(name, filename, hour, minute, truncate, max_files);
        break;
    }

    auto type = g_LoggerHandleType;
    auto obj = logger.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (error != HandleError_None)
    {
        ctx->ReportError("Allocation of multi threaded daily file logger handle failed. (error %d)", error);
        return logger_handle_data{};
    }

    auto data = logger_handle_data{logger, true, handle, type, NULL};
    register_logger_handle_(logger->name(), data);

    SPDLOG_TRACE("Multi threaded daily file logger handle created successfully.");
    return data;
}


void logger_handle_manager::register_logger_handle_(const std::string &key, logger_handle_data data)
{
    // if (logger_datas_.find(key) != logger_datas_.end())
    // {
    //     SPDLOG_CRITICAL("logger with name '{}' already exists.", key);
    // }
    // else if (key == SMEXT_CONF_LOGTAG)
    // {
    //     SPDLOG_CRITICAL("'" SMEXT_CONF_LOGTAG "' is a reserved dedicated logger name.");
    // }

    logger_datas_.insert({key, data});
}


} // namespace log4sp
