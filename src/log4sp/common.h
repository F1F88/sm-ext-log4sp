#pragma once

#include <vector>

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

    for (int i = 0; i < std::size(level_string_views); ++i) {
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

    for (int i = 0; i < std::size(short_level_names); ++i) {
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

    const char *file{path};
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

[[nodiscard]] spdlog::source_loc get_source_loc(SourcePawn::IPluginContext *ctx) noexcept;
[[nodiscard]] std::vector<std::string> get_stack_trace(SourcePawn::IPluginContext *ctx) noexcept;

[[nodiscard]] std::string format_cell_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param);
[[nodiscard]] spdlog::memory_buf_t format_cell_to_mem_buf(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, unsigned int *param);


}   // namespace log4sp
