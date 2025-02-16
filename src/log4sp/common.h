#pragma once

#include <vector>

#include "spdlog/common.h"
#include "spdlog/pattern_formatter.h"

#include "extension.h"


namespace log4sp {

namespace sinks {
class base_sink;
}

namespace details       = spdlog::details;
namespace fmt_lib       = spdlog::fmt_lib;

using memory_buf_t      = fmt_lib::basic_memory_buffer<char, 250>;

using level_t           = spdlog::level_t;
using formatter         = spdlog::formatter;
using pattern_formatter = spdlog::pattern_formatter;
using string_view_t     = spdlog::string_view_t;
using log4sp_ex         = spdlog::spdlog_ex;
using pattern_time_type = spdlog::pattern_time_type;
using filename_t        = spdlog::filename_t;
using file_event_handlers = spdlog::file_event_handlers;

using log_clock         = std::chrono::system_clock;
using sink_ptr          = std::shared_ptr<sinks::base_sink>;
using sinks_init_list   = std::initializer_list<sink_ptr>;

#define LOG4SP_LEVEL_TRACE          SPDLOG_LEVEL_TRACE
#define LOG4SP_LEVEL_DEBUG          SPDLOG_LEVEL_DEBUG
#define LOG4SP_LEVEL_INFO           SPDLOG_LEVEL_INFO
#define LOG4SP_LEVEL_WARN           SPDLOG_LEVEL_WARN
#define LOG4SP_LEVEL_ERROR          SPDLOG_LEVEL_ERROR
#define LOG4SP_LEVEL_FATAL          SPDLOG_LEVEL_CRITICAL
#define LOG4SP_LEVEL_OFF            SPDLOG_LEVEL_OFF

#define LOG4SP_LEVEL_NAMES          SPDLOG_LEVEL_NAMES
#define LOG4SP_SHORT_LEVEL_NAMES    SPDLOG_SHORT_LEVEL_NAMES

namespace level {
using level_enum = spdlog::level::level_enum;
using level_enum::trace;
using level_enum::debug;
using level_enum::info;
using level_enum::warn;
using level_enum::err;
using level_enum::critical;
using level_enum::off;
using level_enum::n_levels;

[[nodiscard]] constexpr size_t to_number(const level_enum lvl) noexcept {
    return static_cast<size_t>(lvl);
}

constexpr auto levels_count = to_number(level_enum::n_levels);

constexpr string_view_t level_string_views[] LOG4SP_LEVEL_NAMES;
constexpr const char   *short_level_names[]  LOG4SP_SHORT_LEVEL_NAMES;

[[nodiscard]] constexpr string_view_t to_string_view(const level_enum lvl) noexcept {
    return level_string_views[to_number(lvl)];
}

[[nodiscard]] constexpr const char *to_short_string_view(const level_enum lvl) noexcept {
    return short_level_names[to_number(lvl)];
}

[[nodiscard]] constexpr level_enum from_number(const uint32_t value) noexcept {
    switch (value) {
        case LOG4SP_LEVEL_TRACE:    return level_enum::trace;
        case LOG4SP_LEVEL_DEBUG:    return level_enum::debug;
        case LOG4SP_LEVEL_INFO:     return level_enum::info;
        case LOG4SP_LEVEL_WARN:     return level_enum::warn;
        case LOG4SP_LEVEL_ERROR:    return level_enum::err;
        case LOG4SP_LEVEL_FATAL:    return level_enum::critical;
        default:                    return level_enum::off;
    }
}

[[nodiscard]] constexpr level_enum from_str(const char *name) noexcept {
    for (size_t i = 0; i < std::size(level_string_views); ++i) {
        if (!strcmp(name, level_string_views[i].data())) {
            return from_number(i);
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

[[nodiscard]] constexpr level_enum from_short_str(const char *name) noexcept {
    for (size_t i = 0; i < std::size(short_level_names); ++i) {
        if (!strcmp(name, short_level_names[i])) {
            return from_number(i);
        }
    }
    return level_enum::off;
}

}   // namespace level



[[nodiscard]] constexpr int64_t int32_to_int64(const uint32_t high, const uint32_t low) noexcept {
    return (static_cast<int64_t>(static_cast<uint32_t>(high)) << 32) | static_cast<uint32_t>(low);
}

[[nodiscard]] constexpr pattern_time_type number_to_pattern_time_type(const uint32_t type) noexcept {
    return type == 0 ? pattern_time_type::local : pattern_time_type::utc;
}

[[noreturn]] void throw_log4sp_ex(std::string msg);
[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno);

struct source_loc final : public spdlog::source_loc {
    constexpr source_loc() = default;
    constexpr source_loc(const char *filename_in, uint32_t line_in, const char *funcname_in)
        : spdlog::source_loc(filename_in, line_in, funcname_in) {}

    [[nodiscard]] constexpr bool empty() const noexcept {
        return !filename || line <= 0 || !funcname;
    }

    // return filename without the leading path
    [[nodiscard]] static constexpr const char *basename(const char *path) {
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

    [[nodiscard]] static source_loc from_plugin_ctx(SourcePawn::IPluginContext *ctx) noexcept;
};

[[nodiscard]] std::vector<std::string> get_plugin_ctx_stack_trace(SourcePawn::IPluginContext *ctx) noexcept;

[[nodiscard]] std::string format_cell_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const uint32_t param);
[[nodiscard]] memory_buf_t format_cell_to_mem_buf(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, uint32_t *param);


}   // namespace log4sp
