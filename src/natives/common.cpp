#include "log4sp/utils.h"

/**
 * native void LogLevelToName(LogLevel lvl, char[] buffer, int maxlen);
 */
static cell_t LogLevelToName(IPluginContext *ctx, const cell_t *params)
{
    auto lvl = log4sp::cell_to_level(params[1]);
    auto name = spdlog::level::to_string_view(lvl).data();

    ctx->StringToLocal(params[2], params[3], name);
    return 0;
}

/**
 * native void LogLevelToShortName(LogLevel lvl, char[] buffer, int maxlen);
 */
static cell_t LogLevelToShortName(IPluginContext *ctx, const cell_t *params)
{
    auto lvl = log4sp::cell_to_level(params[1]);
    auto name = spdlog::level::to_short_c_str(lvl);

    ctx->StringToLocal(params[2], params[3], name);
    return 0;
}

/**
 * native LogLevel NameToLogLevel(const char[] name);
 */
static cell_t NameToLogLevel(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);

    // ref: <common-inl.h> - spdlog::level::from_str()
    auto it = std::find(std::begin(spdlog::level::level_string_views), std::end(spdlog::level::level_string_views), name);
    if (it != std::end(spdlog::level::level_string_views))
    {
        return static_cast<cell_t>(std::distance(std::begin(spdlog::level::level_string_views), it));
    }

    if (!strcmp(name, "warning"))
    {
        return static_cast<cell_t>(spdlog::level::warn);
    }

    if (!strcmp(name, "err"))
    {
        return static_cast<cell_t>(spdlog::level::err);
    }

    if (!strcmp(name, "critical"))
    {
        return static_cast<cell_t>(spdlog::level::critical);
    }

    return static_cast<cell_t>(spdlog::level::off);
}

const sp_nativeinfo_t CommonNatives[] =
{
    {"LogLevelToName",              LogLevelToName},
    {"LogLevelToShortName",         LogLevelToShortName},
    {"NameToLogLevel",              NameToLogLevel},
    {nullptr,                       nullptr}
};
