#include <algorithm>
#include <iterator>

#include "log4sp/common.h"

using spdlog::level::from_str;
using spdlog::level::to_short_c_str;
using spdlog::level::to_string_view;

static cell_t LogLevelToName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto lvl = log4sp::num_to_lvl(params[3]);
    auto name = to_string_view(lvl);

    size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[1], params[2], name.data(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t LogLevelToShortName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto lvl = log4sp::num_to_lvl(params[3]);
    auto name = to_short_c_str(lvl);

    size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[1], params[2], name, &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t NameToLogLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);

    return static_cast<cell_t>(from_str(name));
}

const sp_nativeinfo_t CommonNatives[] =
{
    {"LogLevelToName",              LogLevelToName},
    {"LogLevelToShortName",         LogLevelToShortName},
    {"NameToLogLevel",              NameToLogLevel},
    {nullptr,                       nullptr}
};
