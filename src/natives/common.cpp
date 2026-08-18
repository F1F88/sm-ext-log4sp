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

static cell_t GetLog4spVersion(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    constexpr int MAJOR = LOG4SP_V_MAJOR;
    constexpr int MINOR = LOG4SP_V_MINOR;
    constexpr int PATCH = LOG4SP_V_PATCH;
    constexpr int VERSION = (MAJOR << 16) | (MINOR << 8) | PATCH;
    constexpr const char *BUILD_TIME = __DATE__ " " __TIME__;
    constexpr const char *BUILD_TAGS =
#if defined(NDEBUG) || (!defined(DEBUG) && !defined(_DEBUG))
    "release"
#else
    "debug"
#endif
    ",git=" LOG4SP_SHA_SHORT;

    std::size_t bytes = 0;
    if (auto err = ctx->StringToLocalUTF8(params[1], params[2], BUILD_TIME, &bytes))
    {
        ctx->ReportError("Failed to write build time to buffer (error %d)", err);
        return 0;
    }

    if (auto err = ctx->StringToLocalUTF8(params[3], params[4], BUILD_TAGS, &bytes))
    {
        ctx->ReportError("Failed to write build tags to buffer (error %d)", err);
        return 0;
    }
    return VERSION;
}

const sp_nativeinfo_t CommonNatives[] =
{
    {"LogLevelToName",              LogLevelToName},
    {"LogLevelToShortName",         LogLevelToShortName},
    {"NameToLogLevel",              NameToLogLevel},
    {"GetLog4spVersion",            GetLog4spVersion},

    {nullptr,                       nullptr}
};
