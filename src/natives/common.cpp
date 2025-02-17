#include <algorithm>
#include <iterator>

#include "log4sp/common.h"

/**
 * native int LogLevelToName(char[] buffer, int maxlen, LogLevel lvl);
 */
static cell_t LogLevelToName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[3]));
    auto name = log4sp::level::to_string_view(lvl);

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[1], params[2], name.data(), &bytes);
    return bytes;
}

/**
 * native int LogLevelToShortName(char[] buffer, int maxlen, LogLevel lvl);
 */
static cell_t LogLevelToShortName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[3]));
    auto name = log4sp::level::to_short_string_view(lvl);

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[1], params[2], name, &bytes);
    return bytes;
}

/**
 * native LogLevel NameToLogLevel(const char[] name);
 */
static cell_t NameToLogLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
