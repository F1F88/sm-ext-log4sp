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

/**
 * 发生错误时，err_helper 负责将 origin, err_msg, source_loc 记录或传递给自定义处理器
 *
 * 其中 source_loc 除了 LogSrc 和 LogLoc 等少部分情况已指定外，其他都只能通过 SourcePawn::IPluginContext 查询。
 *
 * 错误属于少见的特殊情况，所有情况都用 SourcePawn::IPluginContext 查询是不合适的。
 *
 * 所有 src_helper::get 用于在发生错误时，返回 source_loc。其中：
 * 如果 source_loc 已指定则直接返回；
 * 如果 source_loc 未指定则使用 SourcePawn::IPluginContext 查询，并更新 source_loc。
 *
 * 这可以保证只在发生错误时查询，且只查询一次，从而提高性能。
 */
class src_helper final {
public:
    // 若 source_loc == empty 则 ctx 必须 != nullptr （反之亦然）
    src_helper(const spdlog::source_loc &loc) noexcept : src_helper(loc, nullptr) {}
    src_helper(SourcePawn::IPluginContext *ctx) noexcept : src_helper({}, ctx) {}
    src_helper(const spdlog::source_loc &loc, SourcePawn::IPluginContext *ctx) noexcept : loc_(loc), ctx_(ctx) {
        assert(!loc.empty() || ctx);
    }

    [[nodiscard]] spdlog::source_loc get() const noexcept;
    [[nodiscard]] static spdlog::source_loc get_from_plugin_ctx(SourcePawn::IPluginContext *ctx) noexcept;
    [[nodiscard]] static std::vector<std::string> get_stack_trace(SourcePawn::IPluginContext *ctx) noexcept;

private:
    mutable spdlog::source_loc loc_;
    SourcePawn::IPluginContext *ctx_;
};

class err_helper final {
public:
    void handle_ex(const std::string &origin, const src_helper &src, const std::exception &ex) const noexcept;
    void handle_unknown_ex(const std::string &origin, const src_helper &src) const noexcept;
    void set_err_handler(SourceMod::IChangeableForward *handler) noexcept;
    ~err_helper() noexcept;
private:
    void release_forward() noexcept;

    SourceMod::IChangeableForward *custom_error_handler_{nullptr};
};


[[nodiscard]] spdlog::filename_t unbuild_path(SourceMod::PathType type, const spdlog::filename_t &filename) noexcept;

[[nodiscard]] std::string format_cell_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param);
[[nodiscard]] spdlog::memory_buf_t format_cell_to_mem_buf(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, unsigned int *param);


}   // namespace log4sp
