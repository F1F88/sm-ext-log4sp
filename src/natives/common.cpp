#include "log4sp/common.h"


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
#if defined(LOG4SP_BUILD_TAGS)
    ",git=" LOG4SP_SHA_SHORT "," LOG4SP_BUILD_TAGS;
#else
    ",git=" LOG4SP_SHA_SHORT;
#endif

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
    {"GetLog4spVersion",            GetLog4spVersion},
    {nullptr,                       nullptr}
};
