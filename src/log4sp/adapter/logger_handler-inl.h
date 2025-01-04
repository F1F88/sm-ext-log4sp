#ifndef _LOG4SP_ADAPTER_LOGGER_HANDLER_INL_H_
#define _LOG4SP_ADAPTER_LOGGER_HANDLER_INL_H_

#include <cassert>
#include <string>

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_INFO
    #include "spdlog/spdlog.h"  // SPDLOG_TRACE 和 SPDLOG_DEBUG 需要
#endif

#include "log4sp/proxy/logger_proxy.h"

#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

inline logger_handler& logger_handler::instance() {
    static logger_handler instance;
    return instance;
}

inline void logger_handler::initialize() {
    instance().initialize_();
}

inline void logger_handler::destroy() {
    instance().destroy_();
}


inline HandleType_t logger_handler::handle_type() const {
    return handle_type_;
}

inline Handle_t logger_handler::create_handle(std::shared_ptr<logger_proxy> object, const HandleSecurity *security, const HandleAccess *access, HandleError *error) {
    Handle_t handle = handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return BAD_HANDLE;
    }

    assert(handles_.find(object->name()) == handles_.end());
    assert(loggers_.find(object->name()) == loggers_.end());

    handles_[object->name()] = handle;
    loggers_[object->name()] = object;

    SPDLOG_TRACE("Logger handle created. (name: {}, hdl: {})", object->name(), handle);
    return handle;
}

inline std::shared_ptr<logger_proxy> logger_handler::read_handle(Handle_t handle, HandleSecurity *security, HandleError *error) {
    logger_proxy *object;
    HandleError err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);

    if (err != HandleError_None) {
        if (error) {
            *error = static_cast<HandleError>(err);
        }
        return nullptr;
    }

    assert(loggers_.find(object->name()) != loggers_.end());

    auto found = loggers_.find(object->name());
    return found->second;
}

inline Handle_t logger_handler::find_handle(const std::string &name) {
    auto found = handles_.find(name);
    return found == handles_.end() ? BAD_HANDLE : found->second;
}

inline std::shared_ptr<logger_proxy> logger_handler::find_logger(const std::string &name) {
    auto found = loggers_.find(name);
    return found == loggers_.end() ? BAD_HANDLE : found->second;
}

inline std::vector<std::string> logger_handler::get_all_logger_names() {
    std::vector<std::string> names;
    for (const auto &pair : loggers_) {
        names.push_back(pair.first);
    }
    return names;
}

inline void logger_handler::apply_all(const std::function<void(const std::shared_ptr<logger_proxy>)> &fun) {
    for (auto &l : loggers_) {
        fun(l.second);
    }
}

inline void logger_handler::OnHandleDestroy(HandleType_t type, void *object) {
    auto logger = static_cast<log4sp::logger_proxy *>(object);

    SPDLOG_TRACE("Logger handle destroyed. (name: {})", logger->name());

    assert(handles_.find(logger->name()) != handles_.end());
    assert(loggers_.find(logger->name()) != loggers_.end());

    handles_.erase(logger->name());
    loggers_.erase(logger->name());
}


inline logger_handler::~logger_handler() {
    destroy_();
}

inline void logger_handler::initialize_() {
    if (handlesys->FindHandleType("Logger", &handle_type_)) {
        throw std::runtime_error("Logger handle type already exists");
    }

    HandleAccess access;
    HandleError error;

    // 默认情况下，创建的 handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[HandleAccess_Delete] = 0;

    handle_type_ = handlesys->CreateType("Logger", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (handle_type_ == NO_HANDLE_TYPE) {
        throw std::runtime_error("Handle error code " + std::to_string(static_cast<int>(error)));
    }
}

inline void logger_handler::destroy_() {
    assert(handle_type_ != NO_HANDLE_TYPE);

    if (handle_type_ != NO_HANDLE_TYPE) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_LOGGER_HANDLER_INL_H_
