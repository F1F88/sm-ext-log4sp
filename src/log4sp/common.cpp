#include <cassert>

#include "log4sp/common.h"

namespace log4sp {

using spdlog::filename_t;

[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno) {
    spdlog::throw_spdlog_ex(msg, last_errno);
}

[[noreturn]] void throw_log4sp_ex(std::string msg) {
    spdlog::throw_spdlog_ex(std::move(msg));
}


[[nodiscard]] spdlog::filename_t unbuild_path(SourceMod::PathType type, const filename_t &filename) noexcept
{
    const char *base = nullptr;
    switch (type)
    {
    case SourceMod::PathType::Path_Game:
        base = smutils->GetGamePath();
        break;
    case SourceMod::PathType::Path_SM:
        base = smutils->GetSourceModPath();
        break;
    case SourceMod::PathType::Path_SM_Rel:
        // TODO
        break;
    default:
        break;
    }

    if (base)
    {
        return filename.substr(std::strlen(base) + 1);
    }
    return filename;
}

[[nodiscard]]
SourceMod::IPlugin *plsys_find_plugin_by_ctx(SourcePawn::IPluginContext *ctx) noexcept
{
#if SMINTERFACE_EXTENSIONAPI_VERSION < 9
    return plsys->FindPluginByContext(ctx->GetContext());
#else
    return plsys->FindPluginByContext(ctx);
#endif
}


}       // namespace log4sp
