#include <algorithm>
#include <iterator>

#include "log4sp/common.h"

/**
 * native void LogLevelToName(LogLevel lvl, char[] buffer, int maxlen);
 */
static cell_t LogLevelToName(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[1]));
    auto name = log4sp::level::to_string_view(lvl);

    size_t bytes;
    ctx->StringToLocalUTF8(params[2], params[3], name.data(), &bytes);
    return bytes;
}

/**
 * native void LogLevelToShortName(LogLevel lvl, char[] buffer, int maxlen);
 */
static cell_t LogLevelToShortName(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[1]));
    auto name = log4sp::level::to_short_string_view(lvl);

    size_t bytes;
    ctx->StringToLocalUTF8(params[2], params[3], name, &bytes);
    return bytes;
}

/**
 * native LogLevel NameToLogLevel(const char[] name);
 */
static cell_t NameToLogLevel(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);

    return static_cast<cell_t>(log4sp::level::from_str(name));
}

const sp_nativeinfo_t CommonNatives[] =
{
    {"LogLevelToName",              LogLevelToName},
    {"LogLevelToShortName",         LogLevelToShortName},
    {"NameToLogLevel",              NameToLogLevel},
    {nullptr,                       nullptr}
};
