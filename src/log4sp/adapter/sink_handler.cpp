#include "log4sp/common.h"
#include "log4sp/adapter/sink_hanlder.h"


namespace log4sp {

void sink_handler::initialize() {
    SourceMod::HandleAccess access;

    // Init plugin create Sinks access
    // 插件创建的 Sink Handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;

    auto error = instance().create_handle_type_(HANDLE_TYPE_NAME, NO_HANDLE_TYPE, nullptr, &access, myself->GetIdentity());
    if (error != SourceMod::HandleError_None)
        throw std::runtime_error("Failed to create \"" + std::string(HANDLE_TYPE_NAME) + "\" Handle type! error " + std::to_string(error) + ".");
}

void sink_handler::destroy() noexcept {
    instance().remove_handle_type_();
}


}       // namespace log4sp
