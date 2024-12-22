#ifndef _LOG4SP_ADAPTER_LOGGER_HANDLER_INL_H_
#define _LOG4SP_ADAPTER_LOGGER_HANDLER_INL_H_

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
    #include "spdlog/spdlog.h"
    // #include "spdlog/fmt/xchar.h"
#endif

#include "log4sp/proxy/logger_proxy.h"

#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

class logger_proxy;

// inline std::string debugLoggers(std::unordered_map<std::string, std::shared_ptr<logger_proxy>> map) {
//     std::string result = "[";
//     for (const auto& [key, value] : map) {
//         result += fmt::format("({}: {}), ", key, fmt::ptr(value.get()));
//     }
//     if (!map.empty()) {
//         result.pop_back();
//         result.pop_back();
//     }
//     result += "]";
//     return result;
// }

// inline std::string debugHandlers(std::unordered_map<std::string, Handle_t> map){
//     std::string result = "[";
//     for (const auto& [key, value] : map) {
//         result += fmt::format("({}: {}), ", key, static_cast<int>(value));
//     }
//     if (!map.empty()) {
//         result.pop_back();
//         result.pop_back();
//     }
//     result += "]";
//     return result;
// }

inline logger_handler& logger_handler::instance() {
    static logger_handler instance;
    return instance;
}

inline HandleError logger_handler::create_handle_type() {
    HandleAccess access;
    HandleError error;

    // 默认情况下，创建的 Logger handle 可以被非拥有者以外的人删除
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[HandleAccess_Delete] = 0;

    handle_type_ = handlesys->CreateType("Logger", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (handle_type_ == NO_HANDLE_TYPE) {
        SPDLOG_CRITICAL("Internal Error! Logger handle type is not registered.");
        return error;
    }
    return HandleError_None;
}

inline HandleType_t logger_handler::handle_type() const {
    return handle_type_;
}

inline Handle_t logger_handler::create_handle(std::shared_ptr<logger_proxy> object, const HandleSecurity *security, const HandleAccess *access, HandleError *error) {
    Handle_t handle = handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return BAD_HANDLE;
    }

    handles_[object->name()] = handle;
    loggers_[object->name()] = object;

    // SPDLOG_TRACE("Logger handle created. (name: {}, hdl: {}, obj: {})", object->name(), handle, fmt::ptr(object.get()));
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

    auto found = loggers_.find(object->name());
    if (found == loggers_.end()) {
        SPDLOG_CRITICAL("Internal Error! handle is valid, but logger is not found. (name: {}, hdl: {})", object->name(), handle);
        if (error) {
            *error = HandleError_Index;
        }
        return nullptr;
    }

    return found->second;
}

inline Handle_t logger_handler::find_handle(const std::string &name) {
    auto found = handles_.find(name);
    return found == handles_.end() ? BAD_HANDLE : found->second;
}

inline logger_handler::~logger_handler() {
    remove_handle_type();
}

inline void logger_handler::OnHandleDestroy(HandleType_t type, void *object) {
    auto logger = static_cast<log4sp::logger_proxy *>(object);

    // SPDLOG_TRACE("Logger handle destroyed. (name: {}, obj: {})", logger->name(), fmt::ptr(logger));

    auto found_handle = handles_.find(logger->name());
    if (found_handle == handles_.end()) {
        SPDLOG_CRITICAL("Unknown handle destroyed. (name: {})", logger->name());
    } else {
        handles_.erase(found_handle);
    }

    auto found_logger = loggers_.find(logger->name());
    if (found_logger == loggers_.end()) {
        SPDLOG_CRITICAL("Unknown handle destroyed. (name: {})", logger->name());
    } else {
        loggers_.erase(found_logger);
    }
}

inline void logger_handler::remove_handle_type() {
    if (handle_type_ != NO_HANDLE_TYPE) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}

inline std::vector<std::string> logger_handler::get_all_logger_names() {
    std::vector<std::string> names;
    // names.reserve(loggers_.size());
    for (const auto& pair : loggers_) {
        names.push_back(pair.first);
    }
    return names;
}


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_LOGGER_HANDLER_INL_H_
