#include <cassert>
#include <string>

#include "spdlog/spdlog.h"  // SPDLOG_TRACE 和 SPDLOG_DEBUG 需要
// #include "spdlog/sinks/sink.h"

#include "log4sp/adapter/sink_hanlder.h"


namespace log4sp {

sink_handler &sink_handler::instance() {
    static sink_handler instance;
    return instance;
}

void sink_handler::initialize() {
    instance().initialize_();
}

void sink_handler::destroy() {
    instance().destroy_();
}


HandleType_t sink_handler::handle_type() const {
    return handle_type_;
}

Handle_t sink_handler::create_handle(std::shared_ptr<spdlog::sinks::sink> object, const HandleSecurity *security, const HandleAccess *access, HandleError *error) {
    Handle_t handle = handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return BAD_HANDLE;
    }

    assert(handles_.find(object.get()) == handles_.end());
    assert(sinks_.find(object.get()) == sinks_.end());

    handles_[object.get()] = handle;
    sinks_[object.get()] = object;

    SPDLOG_TRACE("A sink handle created. (obj: {}, hdl: {})", spdlog::fmt_lib::ptr(object.get()), handle);
    return handle;
}

std::shared_ptr<spdlog::sinks::sink> sink_handler::read_handle(Handle_t handle, HandleSecurity *security, HandleError *error) {
    spdlog::sinks::sink *object;
    HandleError err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);

    if (err != HandleError_None) {
        if (error) {
            *error = static_cast<HandleError>(err);
        }
        return nullptr;
    }

    assert(sinks_.find(object) != sinks_.end());

    auto found = sinks_.find(object);
    return found->second;
}

void sink_handler::OnHandleDestroy(HandleType_t type, void *object) {
    SPDLOG_TRACE("A sink handle destroyed. (obj: {})", spdlog::fmt_lib::ptr(object));

    auto sink = static_cast<spdlog::sinks::sink *>(object);

    assert(handles_.find(sink) != handles_.end());
    assert(sinks_.find(sink) != sinks_.end());

    handles_.erase(sink);
    sinks_.erase(sink);
}


sink_handler::~sink_handler() {
    assert(handle_type_ == NO_HANDLE_TYPE);
}

void sink_handler::initialize_() {
    HandleAccess access;
    HandleError error;

    // 默认情况下，创建的 handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[HandleAccess_Delete] = 0;

    handle_type_ = handlesys->CreateType("Sink", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (handle_type_ == NO_HANDLE_TYPE) {
        throw std::runtime_error("Failed to create Sink handle type. (error: " + std::to_string(static_cast<int>(error)) + ")");
    }
}

void sink_handler::destroy_() {
    assert(handle_type_ != NO_HANDLE_TYPE);

    if (handle_type_ != NO_HANDLE_TYPE) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}


}       // namespace log4sp
