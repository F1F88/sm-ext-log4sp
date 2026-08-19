#pragma once

#include <cstring>

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


#pragma pack(push, 1)
struct CellSourceLoc
{
    char filename[CharArraySize<256>::bytes];
    cell_t line;
    char funcname[CharArraySize<256>::bytes];

    CellSourceLoc() = default;
    CellSourceLoc(const spdlog::source_loc &loc) noexcept : line(loc.line)
    {
        if (loc.filename)
            ke::SafeStrcpy(filename, sizeof(filename), loc.filename);
        else
            filename[0] = '\0';

        if (loc.funcname)
            ke::SafeStrcpy(funcname, sizeof(funcname), loc.funcname);
        else
            funcname[0] = '\0';
    }

    [[nodiscard]]
    spdlog::source_loc ToSourceLoc() const noexcept
    {
        return spdlog::source_loc{filename, line, funcname};
    }

#if defined(DEBUG) || defined(_DEBUG)
    void Debug() const noexcept
    {
        smutils->LogMessage(myself, "[DEBUG] cell_source_loc{funcname=%s,line=%d,filename=%s}", funcname, line, filename);
    }
#endif
};
#pragma pack(pop)

static_assert(offsetof(CellSourceLoc, filename) == 0);
static_assert(offsetof(CellSourceLoc, line) == 256);
static_assert(offsetof(CellSourceLoc, funcname) == 256 + 4);


[[nodiscard]] inline
spdlog::source_loc SourceLocFrom(SourcePawn::IPluginContext *ctx) noexcept
{
    assert(ctx);

    unsigned int line = 0;
    const char *file = nullptr;
    const char *func = nullptr;

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    do
    {
        if (iter->IsScriptedFrame())
        {
            line = iter->LineNumber();
            file = iter->FilePath();
            func = iter->FunctionName();
            break;
        }
        iter->Next();
    } while (!iter->Done());
    ctx->DestroyFrameIterator(iter);

    return spdlog::source_loc(file, static_cast<int>(line), func);
}

[[nodiscard]] inline
std::vector<std::string> StackTraceInfoFrom(SourcePawn::IPluginContext *ctx) noexcept
{
    assert(ctx);

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    if (iter->Done())
    {
        ctx->DestroyFrameIterator(iter);
        return {};
    }

    std::vector<std::string> trace{"Call stack trace:"};

    for (int index = 0; !iter->Done(); iter->Next(), ++index)
    {
        using spdlog::fmt_lib::format;

        if (iter->IsNativeFrame())
        {
            const char *func = iter->FunctionName();
            if (!func)
            {
                func = "<unknown function>";
            }

            trace.emplace_back(format("  [{}] {}", index, func));
        }
        else if (iter->IsScriptedFrame())
        {
            const char *func = iter->FunctionName();
            if (!func)
            {
                func = "<unknown function>";
            }

            const char *file = iter->FilePath();
            if (!file)
            {
                file = "<unknown>";
            }

            trace.emplace_back(format("  [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
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


#define FILE_EVENT_FUNCTION(func)                                                                   \
    [func](const spdlog::filename_t &filename)                                                      \
    {                                                                                               \
        if (func)                                                                                   \
        {                                                                                           \
            auto path = Log4sp::UnbuildPath<SourceMod::PathType::Path_Game>(filename);              \
            auto fwd  = forwards->CreateForwardEx(nullptr, SourceMod::ExecType::ET_Ignore, 1, nullptr, SourceMod::ParamType::Param_String);\
            if (!fwd)                                                                               \
                spdlog::throw_spdlog_ex("Failed to create file event forward.");                    \
                                                                                                    \
            if (!fwd->AddFunction(func))                                                            \
            {                                                                                       \
                forwards->ReleaseForward(fwd);                                                      \
                spdlog::throw_spdlog_ex("Failed to add file event function.");                      \
            }                                                                                       \
                                                                                                    \
            if (auto err = fwd->PushString(path.c_str()))                                           \
            {                                                                                       \
                forwards->ReleaseForward(fwd);                                                      \
                spdlog::throw_spdlog_ex(spdlog::fmt_lib::format("Failed to push filename into file event forward (error {})", err));\
            }                                                                                       \
                                                                                                    \
            if (auto err = fwd->Execute())                                                          \
            {                                                                                       \
                forwards->ReleaseForward(fwd);                                                      \
                spdlog::throw_spdlog_ex(spdlog::fmt_lib::format("Failed to execute file event forward (error {})", err));\
            }                                                                                       \
            forwards->ReleaseForward(fwd);                                                          \
        }                                                                                           \
    }
