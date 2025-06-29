#pragma once

#include <vector>

#include "spdlog/common.h"

#include "extension.h"


namespace log4sp {

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


}   // namespace log4sp
