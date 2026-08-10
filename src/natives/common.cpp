#include <algorithm>
#include <iterator>

#include "log4sp/common.h"


static cell_t LogLevelToName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::to_string_view;

    auto lvl = Log4sp::NumToLvl(params[3]);
    auto name = spdlog::level::to_string_view(lvl);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[1], params[2], name.data(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t LogLevelToShortName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto lvl = Log4sp::NumToLvl(params[3]);
    auto name = spdlog::level::to_short_c_str(lvl);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[1], params[2], name, &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t NameToLogLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);

    return static_cast<cell_t>(spdlog::level::from_str(name));
}

const sp_nativeinfo_t CommonNatives[] =
{
    {"LogLevelToName",              LogLevelToName},
    {"LogLevelToShortName",         LogLevelToShortName},
    {"NameToLogLevel",              NameToLogLevel},
    {nullptr,                       nullptr}
};
