#include <cassert>

#include "log4sp/adapter/sink_hanlder.h"


namespace log4sp {

using spdlog::sink_ptr;
using spdlog::sinks::sink;

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

[[nodiscard]] SourceMod::Handle_t sink_handler::create_handle(sink_ptr object, const SourceMod::HandleSecurity *security, const SourceMod::HandleAccess *access, SourceMod::HandleError *error) noexcept {
    assert(handle_type_);

    SourceMod::Handle_t handle{handlesys->CreateHandleEx(handle_type_, object.get(), security, access, error)};
    if (!handle) {
        return BAD_HANDLE;
    }

    assert(handles_.find(object.get()) == handles_.end());
    assert(sinks_.find(object.get()) == sinks_.end());

    handles_[object.get()] = handle;
    sinks_[object.get()] = object;

    return handle;
}

[[nodiscard]] sink_ptr sink_handler::read_handle(SourceMod::Handle_t handle, SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept {
    assert(handle_type_);

    sink *object;
    SourceMod::HandleError err{handlesys->ReadHandle(handle, handle_type_, security, (void **)&object)};
    if (err != SourceMod::HandleError_None) {
        if (error) {
            *error = err;
        }
        return nullptr;
    }

    assert(sinks_.find(object) != sinks_.end());
    return sinks_.find(object)->second;
}

[[nodiscard]] sink *sink_handler::read_handle_raw(SourceMod::Handle_t handle, SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept {
    assert(handle_type_);

    sink *object;
    SourceMod::HandleError err{handlesys->ReadHandle(handle, handle_type_, security, (void **)&object)};
    if (err != SourceMod::HandleError_None) {
        if (error) {
            *error = err;
        }
        return nullptr;
    }

    assert(sinks_.find(object) != sinks_.end());
    return object;
}

void sink_handler::OnHandleDestroy(SourceMod::HandleType_t type, void *object) {
    auto sink_obj = static_cast<sink*>(object);

    assert(handles_.find(sink_obj) != handles_.end());
    assert(sinks_.find(sink_obj) != sinks_.end());

    handles_.erase(sink_obj);
    sinks_.erase(sink_obj);
}


void sink_handler::initialize_() {
    SourceMod::HandleAccess access;
    SourceMod::HandleError error;

    // 默认情况下，创建的 handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;

    handle_type_ = handlesys->CreateType("Sink", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (!handle_type_) {
        throw_log4sp_ex("SM error! Could not create Sink handle type (error: " + std::to_string(error) + ")" );
    }
}

void sink_handler::destroy_() noexcept {
    if (handle_type_) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
}


}       // namespace log4sp
