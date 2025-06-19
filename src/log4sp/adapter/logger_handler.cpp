#include <cassert>
#include <string>

#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

using spdlog::sink_ptr;
using spdlog::sinks::stdout_sink_st;


[[nodiscard]] logger_handler &logger_handler::instance() noexcept {
    static logger_handler instance;
    return instance;
}

void logger_handler::initialize() {
    instance().initialize_();
}

void logger_handler::destroy() noexcept {
    instance().destroy_();
}


[[nodiscard]] SourceMod::HandleType_t logger_handler::handle_type() const noexcept {
    return handle_type_;
}

[[nodiscard]] SourceMod::Handle_t logger_handler::create_handle(std::shared_ptr<logger> object, const SourceMod::HandleSecurity *security, const SourceMod::HandleAccess *access, SourceMod::HandleError *error) noexcept {
    assert(handle_type_);

    SourceMod::Handle_t handle = handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error);
    if (!handle) {
        return BAD_HANDLE;
    }

    assert(handles_.find(object->name()) == handles_.end());
    assert(loggers_.find(object->name()) == loggers_.end());

    handles_[object->name()] = handle;
    loggers_[object->name()] = object;

    return handle;
}

[[nodiscard]] std::shared_ptr<logger> logger_handler::read_handle(const SourceMod::Handle_t handle, const SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept {
    assert(handle_type_);

    logger *object;
    SourceMod::HandleError err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);
    if (err != SourceMod::HandleError_None) {
        if (error) {
            *error = err;
        }
        return nullptr;
    }

    assert(loggers_.find(object->name()) != loggers_.end());
    return loggers_.find(object->name())->second;
}

[[nodiscard]] logger *logger_handler::read_handle_raw(const SourceMod::Handle_t handle, const SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept {
    assert(handle_type_);

    logger *object;
    SourceMod::HandleError err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);
    if (err != SourceMod::HandleError_None) {
        if (error) {
            *error = err;
        }
        return nullptr;
    }

    assert(loggers_.find(object->name()) != loggers_.end());
    return object;
}

[[nodiscard]] SourceMod::Handle_t logger_handler::find_handle(const std::string &name) const noexcept {
    auto found = handles_.find(name);
    return found == handles_.end() ? BAD_HANDLE : found->second;
}

[[nodiscard]] std::shared_ptr<logger> logger_handler::find_logger(const std::string &name) const noexcept {
    auto found = loggers_.find(name);
    return found == loggers_.end() ? BAD_HANDLE : found->second;
}

void logger_handler::apply_all(const std::function<void(const SourceMod::Handle_t)> &fun) {
    for (auto &h : handles_) {
        fun(h.second);
    }
}

void logger_handler::apply_all(const std::function<void(std::shared_ptr<logger>)> &fun) {
    for (auto &l : loggers_) {
        fun(l.second);
    }
}

void logger_handler::OnHandleDestroy(SourceMod::HandleType_t type, void *object) {
    auto logger = static_cast<log4sp::logger *>(object);

    assert(handles_.find(logger->name()) != handles_.end());
    assert(loggers_.find(logger->name()) != loggers_.end());

    handles_.erase(logger->name());
    loggers_.erase(logger->name());
}


void logger_handler::initialize_() {
    SourceMod::HandleAccess access;
    SourceMod::HandleError error;

    // 默认情况下，创建的 handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;

    handle_type_ = handlesys->CreateType("Logger", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (!handle_type_) {
        throw_log4sp_ex("SM error! Could not create Logger handle type (error: " + std::to_string(error) + ")");
    }

    // Init Global Logger
    // 全局 Logger 生命周期与拓展一致，不应该被任何插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] |= HANDLE_RESTRICT_IDENTITY;
    SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());

    sink_ptr sink;
    try {
        sink = std::make_shared<stdout_sink_st>();
    } catch (const std::exception &ex) {
        throw_log4sp_ex(std::string("Could not create global logger handle (reason: ") + ex.what() + ")");
    }

    auto logger = std::make_shared<log4sp::logger>(SMEXT_CONF_LOGTAG, sink);
    auto handle = logger_handler::instance().create_handle(logger, &security, &access, &error);
    if (!handle) {
        throw_log4sp_ex("SM error! Could not create global logger handle (error: " + std::to_string(error) + ")");
    }
}

void logger_handler::destroy_() noexcept {
    if (handle_type_) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}


}       // namespace log4sp
