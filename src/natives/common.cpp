#include <log4sp/common.h>

/**
 * native void LogLevelToName(LogLevel lvl, char[] buffer, int maxlen);
 */
static cell_t LogLevelToName(IPluginContext *ctx, const cell_t *params)
{
    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[1]);
    const char *name = spdlog::level::to_string_view(lvl).data();

    ctx->StringToLocal(params[2], params[3], name);
    return 0;
}

/**
 * native void LogLevelToShortName(LogLevel lvl, char[] buffer, int maxlen);
 */
static cell_t LogLevelToShortName(IPluginContext *ctx, const cell_t *params)
{
    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[1]);
    const char *name = spdlog::level::to_short_c_str(lvl);

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

    // ref: spdlog::level::from_str()
    auto it = std::find(std::begin(spdlog::level::level_string_views), std::end(spdlog::level::level_string_views), name);
    if (it != std::end(spdlog::level::level_string_views) )
    {
        return static_cast<cell_t>(std::distance(std::begin(spdlog::level::level_string_views), it));
    }

    if (!strcmp(name, "warn"))
    {
        return static_cast<cell_t>(spdlog::level::warn);
    }

    if (!strcmp(name, "err"))
    {
        return static_cast<cell_t>(spdlog::level::err);
    }

    spdlog::log(log4sp::GetScriptedLoc(ctx), spdlog::level::warn, "Invalid level name '{}', return LogLevel_Off.", name);
    return static_cast<cell_t>(spdlog::level::off);;
}

const sp_nativeinfo_t CommonNatives[] =
{
    {"LogLevelToName",              LogLevelToName},
    {"LogLevelToShortName",         LogLevelToShortName},
    {"NameToLogLevel",              NameToLogLevel},
    {NULL,                          NULL}
};
