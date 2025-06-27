#pragma once

#include <functional>

#include "spdlog/common.h"

#include "extension.h"


namespace log4sp {

[[nodiscard]] constexpr spdlog::level::level_enum num_to_lvl(const int value) noexcept {
    using spdlog::level::level_enum;
    switch (value) {
        case SPDLOG_LEVEL_TRACE:    return level_enum::trace;
        case SPDLOG_LEVEL_DEBUG:    return level_enum::debug;
        case SPDLOG_LEVEL_INFO:     return level_enum::info;
        case SPDLOG_LEVEL_WARN:     return level_enum::warn;
        case SPDLOG_LEVEL_ERROR:    return level_enum::err;
        case SPDLOG_LEVEL_CRITICAL: return level_enum::critical;
        default:                    return level_enum::off;
    }
}

[[nodiscard]] constexpr spdlog::level::level_enum str_to_lvl(const char *name) noexcept {
    using spdlog::level::level_enum;
    using spdlog::string_view_t;
    constexpr string_view_t level_string_views[] SPDLOG_LEVEL_NAMES;
    constexpr int size = std::size(level_string_views);
    static_assert(size == level_enum::n_levels);

    for (int i = 0; i < size; ++i) {
        if (!strcmp(name, level_string_views[i].data())) {
            return num_to_lvl(i);
        }
    }

    if (!strcmp(name, "warning")) {
        return level_enum::warn;
    }

    if (!strcmp(name, "err")) {
        return level_enum::err;
    }

    if (!strcmp(name, "critical")) {
        return level_enum::critical;
    }

    return level_enum::off;
}

[[nodiscard]] constexpr spdlog::level::level_enum str_short_to_lvl(const char *name) noexcept {
    using spdlog::level::level_enum;
    constexpr const char *short_level_names[] SPDLOG_SHORT_LEVEL_NAMES;
    constexpr int size = std::size(short_level_names);
    static_assert(size == level_enum::n_levels);

    for (int i = 0; i < size; ++i) {
        if (!strcmp(name, short_level_names[i])) {
            return num_to_lvl(i);
        }
    }
    return level_enum::off;
}

[[nodiscard]] constexpr spdlog::pattern_time_type number_to_pattern_time_type(const int type) noexcept {
    using spdlog::pattern_time_type;
    return type == 0 ? pattern_time_type::local : pattern_time_type::utc;
}

[[nodiscard]] constexpr const char *get_path_filename(const char *path) noexcept {
    if (!path) {
        return path;
    }

    const char *file = path;
    while (*path) {
        if (*path == '\\' || *path == '/') {
            file = path + 1;
        }
        ++path;
    }
    return file;
}

[[noreturn]] void throw_log4sp_ex(std::string msg);
[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno);

[[nodiscard]] spdlog::filename_t unbuild_path(SourceMod::PathType type, const spdlog::filename_t &filename) noexcept;


}   // namespace log4sp

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
        auto forward = forwards->CreateForwardEx(name, et, num_params, types, ##__VA_ARGS__);
#else
    #define FWDS_CREATE_EX(name, et, num_params, types, ...) \
        auto forward = forwards->CreateForwardEx(name, et, num_params, types, ##__VA_ARGS__); \
        assert(forward);
#endif

#ifndef DEBUG
    #define FWD_ADD_FUNCTION(function)                      forward->AddFunction(function);
    #define FWD_EXECUTE(...)                                forward->Execute(##__VA_ARGS__);
    #define FWD_PUSH_ARRAY(inarray, cells, ...)             forward->PushArray(inarray, cells, ##__VA_ARGS__);
    #define FWD_PUSH_CELL(cell)                             forward->PushCell(cell);
    #define FWD_PUSH_CELL_BY_REF(cell, ...)                 forward->PushCellByRef(cell, ##__VA_ARGS__);
    #define FWD_PUSH_FLOAT(number)                          forward->PushFloat(cell);
    #define FWD_PUSH_FLOAT_BY_REF(number, ...)              forward->PushFloatByRef(cell, ##__VA_ARGS__);
    #define FWD_PUSH_STRING(string)                         forward->PushString(string);
    #define FWD_PUSH_STRING_EX(buffer, length, sz_flags, cp_flags) \
        forward->PushStringEx(buffer, length, sz_flags, cp_flags);
#else
    #define FWD_ADD_FUNCTION(function)                      assert(forward->AddFunction(function));
    #define FWD_EXECUTE(...)                                assert(!forward->Execute(##__VA_ARGS__));
    #define FWD_PUSH_ARRAY(inarray, cells, ...)             assert(!forward->PushArray(inarray, cells, ##__VA_ARGS__));
    #define FWD_PUSH_CELL(cell)                             assert(!forward->PushCell(cell));
    #define FWD_PUSH_CELL_BY_REF(cell, ...)                 assert(!forward->PushCellByRef(cell, ##__VA_ARGS__));
    #define FWD_PUSH_FLOAT(number)                          assert(!forward->PushFloat(cell));
    #define FWD_PUSH_FLOAT_BY_REF(number, ...)              assert(!forward->PushFloatByRef(cell, ##__VA_ARGS__));
    #define FWD_PUSH_STRING(string)                         assert(!forward->PushString(string));
    #define FWD_PUSH_STRING_EX(buffer, length, sz_flags, cp_flags) \
        assert(!forward->PushStringEx(buffer, length, sz_flags, cp_flags));
#endif

#ifndef DEBUG
    #define HANDLE_SYS_FREE_HANDLE(handle, security)        handlesys->FreeHandle(handle, security);
#else
    #define HANDLE_SYS_FREE_HANDLE(handle, security)        assert(!handlesys->FreeHandle(handle, security));
#endif

#define FILE_EVENT_CALLBACK(callback)                                                               \
    [callback](const filename_t &filename) {                                                        \
        if (callback) {                                                                             \
            auto path = log4sp::unbuild_path(SourceMod::PathType::Path_Game, filename);             \
            FWDS_CREATE_EX(nullptr, ET_Ignore, 1, nullptr, Param_String);                           \
            FWD_ADD_FUNCTION(callback);                                                             \
            FWD_PUSH_STRING(path.c_str());                                                          \
            FWD_EXECUTE();                                                                          \
            forwards->ReleaseForward(forward);                                                      \
        }                                                                                           \
    }
