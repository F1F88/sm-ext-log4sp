#include <log4sp/sink_handle_manager.h>


namespace log4sp {

sink_handle_manager &sink_handle_manager::instance()
{
    static sink_handle_manager singleInstance;
    return singleInstance;
}

sink_handle_data* sink_handle_manager::get_data(spdlog::sinks::sink *sink)
{
    auto found = sink_datas_.find(sink);
    return found != sink_datas_.end() ? found->second : nullptr;
}

HandleType_t sink_handle_manager::get_handle_type(Handle_t handle)
{
    auto found = handle_types_.find(handle);
    return found != handle_types_.end() ? found->second : NO_HANDLE_TYPE;
}

spdlog::sinks::sink* sink_handle_manager::read_handle(IPluginContext *ctx, Handle_t handle)
{
    auto type = get_handle_type(handle);
    if (type == NO_HANDLE_TYPE)
    {
        ctx->ReportError("Unable to identify the type of sink handle. (handle=0x%x)", handle);
        return nullptr;
    }

    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    spdlog::sinks::sink *sink;

    auto error = handlesys->ReadHandle(handle, type, &sec, (void **)&sink);
    if (error != HandleError_None)
    {
        ctx->ReportError("Invalid sink handle. (handle=0x%x, error=%d)", handle, error);
        return nullptr;
    }

    return sink;
}

void sink_handle_manager::drop(spdlog::sinks::sink *sink)
{
    auto it = sink_datas_.find(sink);
    if (it != sink_datas_.end())
    {
        handle_types_.erase(it->second->handle());
        sink_datas_.erase(it);
    }
}

void sink_handle_manager::drop_all()
{
    sink_datas_.clear();
    handle_types_.clear();
}

void sink_handle_manager::shutdown()
{
    drop_all();
}


sink_handle_data* sink_handle_manager::create_server_console_sink_st(IPluginContext *ctx)
{
    auto sink = std::make_shared<spdlog::sinks::stdout_sink_st>();

    auto type = g_ServerConsoleSinkSTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded server console sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, false, handle, type};
    register_sink_handle_(obj, data);
    return data;
}

sink_handle_data* sink_handle_manager::create_server_console_sink_mt(IPluginContext *ctx)
{
    auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();

    auto type = g_ServerConsoleSinkMTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded server console sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, true, handle, type};
    register_sink_handle_(obj, data);
    return data;
}


sink_handle_data* sink_handle_manager::create_base_file_sink_st(IPluginContext *ctx, const char *filename, bool truncate)
{
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filename, truncate);

    auto type = g_BaseFileSinkSTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded base file sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, false, handle, type};
    register_sink_handle_(obj, data);
    return data;
}

sink_handle_data* sink_handle_manager::create_base_file_sink_mt(IPluginContext *ctx, const char *filename, bool truncate)
{
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);

    auto type = g_BaseFileSinkMTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded base file sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, true, handle, type};
    register_sink_handle_(obj, data);
    return data;
}


sink_handle_data* sink_handle_manager::create_rotating_file_sink_st(IPluginContext *ctx, const char *base_filename, size_t max_size, size_t max_files, bool rotate_on_open)
{
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(base_filename, max_size, max_files, rotate_on_open);

    auto type = g_RotatingFileSinkSTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded rotating file sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, false, handle, type};
    register_sink_handle_(obj, data);
    return data;
}

sink_handle_data* sink_handle_manager::create_rotating_file_sink_mt(IPluginContext *ctx, const char *base_filename, size_t max_size, size_t max_files, bool rotate_on_open)
{
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(base_filename, max_size, max_files, rotate_on_open);

    auto type = g_RotatingFileSinkMTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded rotating file sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, true, handle, type};
    register_sink_handle_(obj, data);
    return data;
}


sink_handle_data* sink_handle_manager::create_daily_file_sink_st(IPluginContext *ctx, const char *base_filename, int rotation_hour, int rotation_minute, bool truncate, uint16_t max_files)
{
    auto sink = std::make_shared<spdlog::sinks::daily_file_format_sink_st>(base_filename, rotation_hour, rotation_minute, truncate, max_files);

    auto type = g_DailyFileSinkSTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded daily file sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, false, handle, type};
    register_sink_handle_(obj, data);
    return data;
}

sink_handle_data* sink_handle_manager::create_daily_file_sink_mt(IPluginContext *ctx, const char *base_filename, int rotation_hour, int rotation_minute, bool truncate, uint16_t max_files)
{
    auto sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(base_filename, rotation_hour, rotation_minute, truncate, max_files);

    auto type = g_DailyFileSinkMTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded daily file sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, true, handle, type};
    register_sink_handle_(obj, data);
    return data;
}

sink_handle_data* sink_handle_manager::create_client_console_sink_st(IPluginContext *ctx)
{
    auto sink = std::make_shared<log4sp::sinks::client_console_sink_st>();

    auto type = g_ClientConsoleSinkSTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of single threaded client console sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, false, handle, type};
    register_sink_handle_(obj, data);
    return data;
}

sink_handle_data* sink_handle_manager::create_client_console_sink_mt(IPluginContext *ctx)
{
    auto sink = std::make_shared<log4sp::sinks::client_console_sink_mt>();

    auto type = g_ClientConsoleSinkMTHandleType;
    auto obj = sink.get();
    auto sec = HandleSecurity(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error;

    Handle_t handle = handlesys->CreateHandleEx(type, obj, &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of multi threaded client console sink handle failed. (error %d)", error);
        return nullptr;
    }

    auto data = new sink_handle_data{sink, true, handle, type};
    register_sink_handle_(obj, data);
    return data;
}


void sink_handle_manager::register_sink_handle_(spdlog::sinks::sink *key, sink_handle_data* data)
{
    sink_datas_.insert(std::pair{key, data});
    handle_types_.insert(std::pair{data->handle(), data->handle_type()});
}


} // namespace log4sp
