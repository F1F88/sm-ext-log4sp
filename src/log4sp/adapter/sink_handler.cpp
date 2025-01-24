#include <cassert>
#include <string>

#include "spdlog/sinks/sink.h"

#include "log4sp/adapter/sink_hanlder.h"


namespace log4sp {

[[nodiscard]] sink_handler &sink_handler::instance() noexcept {
    static sink_handler instance;
    return instance;
}

void sink_handler::initialize() {
    instance().initialize_();
}

void sink_handler::destroy() noexcept {
    instance().destroy_();
}


[[nodiscard]] SourceMod::HandleType_t sink_handler::handle_type() const noexcept {
    return handle_type_;
}

[[nodiscard]] SourceMod::Handle_t sink_handler::create_handle(std::shared_ptr<spdlog::sinks::sink> object, const SourceMod::HandleSecurity *security, const SourceMod::HandleAccess *access, SourceMod::HandleError *error) noexcept {
    assert(handle_type_ != NO_HANDLE_TYPE);

    auto handle = handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return BAD_HANDLE;
    }

    assert(handles_.find(object.get()) == handles_.end());
    assert(sinks_.find(object.get()) == sinks_.end());

    handles_[object.get()] = handle;
    sinks_[object.get()] = object;

    return handle;
}

[[nodiscard]] std::shared_ptr<spdlog::sinks::sink> sink_handler::read_handle(SourceMod::Handle_t handle, SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept {
    assert(handle_type_ != NO_HANDLE_TYPE);

    spdlog::sinks::sink *object;
    auto err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);
    if (err != SourceMod::HandleError_None) {
        if (error) {
            *error = static_cast<SourceMod::HandleError>(err);
        }
        return nullptr;
    }

    assert(sinks_.find(object) != sinks_.end());

    auto found = sinks_.find(object);
    return found->second;
}

[[nodiscard]] spdlog::sinks::sink *sink_handler::read_handle_raw(SourceMod::Handle_t handle, SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept {
    assert(handle_type_ != NO_HANDLE_TYPE);

    spdlog::sinks::sink *object;
    auto err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&object);
    if (err != SourceMod::HandleError_None) {
        if (error) {
            *error = static_cast<SourceMod::HandleError>(err);
        }
        return nullptr;
    }

    assert(sinks_.find(object) != sinks_.end());

    return object;
}

void sink_handler::OnHandleDestroy(SourceMod::HandleType_t type, void *object) {
    auto sink = static_cast<spdlog::sinks::sink *>(object);

    assert(handles_.find(sink) != handles_.end());
    assert(sinks_.find(sink) != sinks_.end());

    handles_.erase(sink);
    sinks_.erase(sink);
}


void sink_handler::initialize_() {
    SourceMod::HandleAccess access;
    SourceMod::HandleError error;

    // 默认情况下，创建的 handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;

    handle_type_ = handlesys->CreateType("Sink", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (handle_type_ == NO_HANDLE_TYPE) {
        throw std::runtime_error{spdlog::fmt_lib::format("SM error! Could not create Sink handle type (error: {})", static_cast<int>(error))};
    }
}

void sink_handler::destroy_() noexcept {
    if (handle_type_ != NO_HANDLE_TYPE) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}


}       // namespace log4sp
