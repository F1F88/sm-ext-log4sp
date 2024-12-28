#ifndef _LOG4SP_ADAPTER_SINK_HANDLER_INL_H_
#define _LOG4SP_ADAPTER_SINK_HANDLER_INL_H_


#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
    #include "spdlog/spdlog.h"
    // #include "spdlog/fmt/xchar.h"
#else
    #include "spdlog/sinks/sink.h"
#endif

#include "log4sp/adapter/sink_hanlder.h"


namespace log4sp {

inline sink_handler& sink_handler::instance() {
    static sink_handler instance;
    return instance;
}

inline HandleError sink_handler::create_handle_type() {
    HandleAccess access;
    HandleError error;

    // 默认情况下，创建的 Sink handle 可以被非拥有者以外的人删除
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[HandleAccess_Delete] = 0;

   handle_type_ = handlesys->CreateType("Sink", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (handle_type_ == NO_HANDLE_TYPE) {
        SPDLOG_CRITICAL("Internal Error! Sink handle type is not registered.");
        return error;
    }
    return HandleError_None;
}

inline HandleType_t sink_handler::handle_type() const {
    return handle_type_;
}

inline Handle_t sink_handler::create_handle(std::shared_ptr<spdlog::sinks::sink> object, const HandleSecurity *security, const HandleAccess *access, HandleError *error) {
    Handle_t handle = handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return BAD_HANDLE;
    }

    handles_[object.get()] = handle;
    sinks_[object.get()] = object;

    SPDLOG_TRACE("Sink handle created. (obj: {}, hdl: {})", spdlog::fmt_lib::ptr(object.get()), handle);
    return handle;
}

inline std::shared_ptr<spdlog::sinks::sink> sink_handler::read_handle(Handle_t handle, HandleSecurity *security, HandleError *error) {
    spdlog::sinks::sink *object;
    HandleError err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);

    if (err != HandleError_None) {
        if (error) {
            *error = static_cast<HandleError>(err);
        }
        return nullptr;
    }

    auto found = sinks_.find(object);
    if (found == sinks_.end()) {
        SPDLOG_CRITICAL("Internal Error! handle is valid, but sink is not found. (obj: {}, hdl: {})", spdlog::fmt_lib::ptr(object), handle);
        if (error) {
            *error = HandleError_Index;
        }
        return nullptr;
    }

    return found->second;
}

inline sink_handler::~sink_handler() {
    remove_handle_type();
}

inline void sink_handler::OnHandleDestroy(HandleType_t type, void *object) {
    auto sink = static_cast<spdlog::sinks::sink *>(object);

    SPDLOG_TRACE("Sink handle destroyed. (obj: {})", spdlog::fmt_lib::ptr(object));

    auto found_handle = handles_.find(sink);
    if (found_handle == handles_.end()) {
        SPDLOG_CRITICAL("Unknown handle destroyed. (obj: {})", spdlog::fmt_lib::ptr(object));
    } else {
        handles_.erase(found_handle);
    }

    auto found_sink = sinks_.find(sink);
    if (found_sink == sinks_.end()) {
        SPDLOG_CRITICAL("Unknown handle destroyed. (obj: {})", spdlog::fmt_lib::ptr(object));
    } else {
        sinks_.erase(found_sink);
    }
}

inline void sink_handler::remove_handle_type() {
    if (handle_type_ != NO_HANDLE_TYPE) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_SINK_HANDLER_INL_H_
