#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/adapter/logger_handler.h"


namespace log4sp {

void logger_handler::initialize() {
    SourceMod::HandleAccess access;

    // Init plugin create Logger access
    // 插件创建的 Logger Handle 可以被任意插件释放, 但不允许克隆
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;
    access.access[SourceMod::HandleAccess_Clone] |= HANDLE_RESTRICT_IDENTITY;

    auto error = instance().create_handle_type_(HANDLE_TYPE_NAME, NO_HANDLE_TYPE, nullptr, &access, myself->GetIdentity());
    if (error != SourceMod::HandleError_None)
        throw std::runtime_error("Failed to create \"" + std::string(HANDLE_TYPE_NAME) + "\" Handle type! error " + std::to_string(error) + ".");

    instance().create_global_logger();
}

void logger_handler::destroy() noexcept {
    instance().remove_handle_type_();
}

void logger_handler::create_global_logger() {
    using spdlog::sinks::stdout_sink_st;

    SourceMod::HandleAccess access;
    SourceMod::HandleError error;

    // Init Global Logger access
    // 拓展创建的全局 Logger Handle 不可以被插件释放, 生命周期由拓展管控, 且不允许克隆
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] |= HANDLE_RESTRICT_IDENTITY;
    access.access[SourceMod::HandleAccess_Clone]  |= HANDLE_RESTRICT_IDENTITY;
    SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());

    try {
        auto sink = std::make_shared<stdout_sink_st>();
        auto logger = std::make_shared<log4sp::logger>(SMEXT_CONF_LOGTAG, sink);
        auto handle = create_handle(logger, &security, &access, &error);
        if (!handle)
            throw std::runtime_error("error " + std::to_string(error));
    } catch (const std::exception &ex) {
        throw std::runtime_error("Failed to create global Logger Handle (reason: " + std::string(ex.what()) + ")");
    }
}

}       // namespace log4sp
