#pragma once

#include "spdlog/common.h"

#include "extension.h"


namespace Log4sp {

[[nodiscard]] constexpr
spdlog::level::level_enum NumToLvl(const int value) noexcept
{
    using spdlog::level::level_enum;
    switch (value)
    {
        case SPDLOG_LEVEL_TRACE:    return level_enum::trace;
        case SPDLOG_LEVEL_DEBUG:    return level_enum::debug;
        case SPDLOG_LEVEL_INFO:     return level_enum::info;
        case SPDLOG_LEVEL_WARN:     return level_enum::warn;
        case SPDLOG_LEVEL_ERROR:    return level_enum::err;
        case SPDLOG_LEVEL_CRITICAL: return level_enum::critical;
        default:                    return level_enum::off;
    }
}

[[nodiscard]] constexpr
spdlog::level::level_enum StrToLvl(const char *name) noexcept
{
    using spdlog::level::level_enum;
    using spdlog::string_view_t;
    constexpr string_view_t levels[] SPDLOG_LEVEL_NAMES;
    constexpr int size = std::size(levels);
    static_assert(size == level_enum::n_levels);

    for (int i = 0; i < size; ++i)
    {
        if (!strcmp(name, levels[i].data()))
            return NumToLvl(i);
    }

    if (!strcmp(name, "warning"))
        return level_enum::warn;

    if (!strcmp(name, "err"))
        return level_enum::err;

    if (!strcmp(name, "critical"))
        return level_enum::critical;

    return level_enum::off;
}

[[nodiscard]] constexpr
spdlog::level::level_enum ShortNameToLevel(const char *name) noexcept
{
    using spdlog::level::level_enum;
    constexpr const char *levels[] SPDLOG_SHORT_LEVEL_NAMES;
    constexpr int size = std::size(levels);
    static_assert(size == level_enum::n_levels);

    for (int i = 0; i < size; ++i)
    {
        if (!strcmp(name, levels[i]))
            return NumToLvl(i);
    }
    return level_enum::off;
}

[[nodiscard]] constexpr
spdlog::pattern_time_type NumToPatternTimeType(const int type) noexcept
{
    using spdlog::pattern_time_type;
    return type == 0 ? pattern_time_type::local : pattern_time_type::utc;
}

[[nodiscard]] constexpr
const char *FilenameFrom(const char *path) noexcept
{
    if (!path)
        return path;

    const char *file = path;
    while (*path)
    {
        if (*path == '\\' || *path == '/')
        {
            file = path + 1;
        }
        ++path;
    }
    return file;
}

template <SourceMod::PathType T>
[[nodiscard]] inline
spdlog::filename_t UnbuildPath(const spdlog::filename_t &filename) noexcept
{
    const char *base = nullptr;

    if constexpr (T == SourceMod::PathType::Path_Game)
    {
        base = smutils->GetGamePath();
    }
    else if constexpr (T == SourceMod::PathType::Path_SM)
    {
        base = smutils->GetSourceModPath();
    }
    else if constexpr (T == SourceMod::PathType::Path_SM_Rel)
    {
        // TODO
        static_assert(T != T, "UnbuildPath: Unsupported Path_SM_Rel used.");
    }
    else
    {
        static_assert(T != T, "UnbuildPath: Unsupported PathType used.");
    }

    if (base)
        return filename.substr(std::strlen(base) + 1);
    return filename;
}

[[noreturn]] inline
void ThrowLog4spEx(std::string msg)
{
    spdlog::throw_spdlog_ex(std::move(msg));
}

[[noreturn]] inline
void ThrowLog4spEx(const std::string &msg, int last_errno)
{
    spdlog::throw_spdlog_ex(msg, last_errno);
}

[[nodiscard]] inline
SourceMod::IPlugin* PluginSysFindPluginByCtx(SourcePawn::IPluginContext *ctx) noexcept
{
#if SMINTERFACE_EXTENSIONAPI_VERSION < 9
    return plsys->FindPluginByContext(ctx->GetContext());
#else
    return plsys->FindPluginByContext(ctx);
#endif
}


}   // namespace Log4sp

#ifndef DEBUG
    #define CTX_LOCAL_TO_PHYS_ADDR(local_addr, phys_addr)   ctx->LocalToPhysAddr(local_addr, phys_addr);
    #define CTX_LOCAL_TO_STRING(local_addr, addr)           ctx->LocalToString(local_addr, addr);
    #define CTX_LOCAL_TO_STRING_NULL(local_addr, addr)      ctx->LocalToStringNULL(local_addr, addr);
    #define CTX_STRING_TO_LOCAL(local_addr, bytes, source)  ctx->StringToLocal(local_addr, bytes, source);
    #define CTX_STRING_TO_LOCAL_UTF8(local_addr, maxbytes, source, wrtnbytes) \
        ctx->StringToLocalUTF8(local_addr, maxbytes, source, wrtnbytes);
#else
    #define CTX_LOCAL_TO_PHYS_ADDR(local_addr, phys_addr)   assert(!ctx->LocalToPhysAddr(local_addr, phys_addr));
    #define CTX_LOCAL_TO_STRING(local_addr, addr)           assert(!ctx->LocalToString(local_addr, addr));
    #define CTX_LOCAL_TO_STRING_NULL(local_addr, addr)      assert(!ctx->LocalToStringNULL(local_addr, addr));
    #define CTX_STRING_TO_LOCAL(local_addr, bytes, source)  assert(!ctx->StringToLocal(local_addr, bytes, source));
    #define CTX_STRING_TO_LOCAL_UTF8(local_addr, maxbytes, source, wrtnbytes) \
        assert(!ctx->StringToLocalUTF8(local_addr, maxbytes, source, wrtnbytes));
#endif

#ifndef DEBUG
    #define FWDS_CREATE_EX(name, et, num_params, types, ...) \
        auto fwd = forwards->CreateForwardEx(name, et, num_params, types, ##__VA_ARGS__);
#else
    #define FWDS_CREATE_EX(name, et, num_params, types, ...) \
        auto fwd = forwards->CreateForwardEx(name, et, num_params, types, ##__VA_ARGS__); \
        assert(fwd);
#endif

#ifndef DEBUG
    #define FWD_ADD_FUNCTION(func)                          fwd->AddFunction(func);
    #define FWD_EXECUTE(...)                                fwd->Execute(##__VA_ARGS__);
    #define FWD_PUSH_ARRAY(inarray, cells, ...)             fwd->PushArray(inarray, cells, ##__VA_ARGS__);
    #define FWD_PUSH_CELL(cell)                             fwd->PushCell(cell);
    #define FWD_PUSH_CELL_BY_REF(cell, ...)                 fwd->PushCellByRef(cell, ##__VA_ARGS__);
    #define FWD_PUSH_FLOAT(cell)                            fwd->PushFloat(cell);
    #define FWD_PUSH_FLOAT_BY_REF(cell, ...)                fwd->PushFloatByRef(cell, ##__VA_ARGS__);
    #define FWD_PUSH_STRING(str)                            fwd->PushString(str);
    #define FWD_PUSH_STRING_EX(buffer, length, sz_flags, cp_flags) \
        fwd->PushStringEx(buffer, length, sz_flags, cp_flags);
#else
    #define FWD_ADD_FUNCTION(func)                          assert(fwd->AddFunction(func));
    #define FWD_EXECUTE(...)                                assert(!fwd->Execute(##__VA_ARGS__));
    #define FWD_PUSH_ARRAY(inarray, cells, ...)             assert(!fwd->PushArray(inarray, cells, ##__VA_ARGS__));
    #define FWD_PUSH_CELL(cell)                             assert(!fwd->PushCell(cell));
    #define FWD_PUSH_CELL_BY_REF(cell, ...)                 assert(!fwd->PushCellByRef(cell, ##__VA_ARGS__));
    #define FWD_PUSH_FLOAT(cell)                            assert(!fwd->PushFloat(cell));
    #define FWD_PUSH_FLOAT_BY_REF(cell, ...)                assert(!fwd->PushFloatByRef(cell, ##__VA_ARGS__));
    #define FWD_PUSH_STRING(str)                            assert(!fwd->PushString(str));
    #define FWD_PUSH_STRING_EX(buffer, length, sz_flags, cp_flags) \
        assert(!fwd->PushStringEx(buffer, length, sz_flags, cp_flags));
#endif

#ifndef DEBUG
    #define HANDLE_SYS_FREE_HANDLE(handle, security)        handlesys->FreeHandle(handle, security);
#else
    #define HANDLE_SYS_FREE_HANDLE(handle, security)        assert(!handlesys->FreeHandle(handle, security));
#endif

#define FILE_EVENT_FUNCTION(func)                                                                   \
    [func](const spdlog::filename_t &filename)                                                      \
    {                                                                                               \
        if (func)                                                                                   \
        {                                                                                           \
            auto path = Log4sp::UnbuildPath<SourceMod::PathType::Path_Game>(filename);              \
            FWDS_CREATE_EX(nullptr, ET_Ignore, 1, nullptr, Param_String);                           \
            FWD_ADD_FUNCTION(func);                                                                 \
            FWD_PUSH_STRING(path.c_str());                                                          \
            FWD_EXECUTE();                                                                          \
            forwards->ReleaseForward(fwd);                                                          \
        }                                                                                           \
    }
