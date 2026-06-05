#pragma once

#include <vector>

#include "spdlog/common.h"

#include "extension.h"


namespace Log4sp {

/**
 * 发生错误时，ErrHelper 负责将 origin, err_msg, source_loc 记录或传递给自定义处理器
 *
 * 其中 source_loc 除了 LogSrc 和 LogLoc 等少部分情况已指定外，其他都只能通过 SourcePawn::IPluginContext 查询。
 *
 * 错误属于少见的特殊情况，所有情况都用 SourcePawn::IPluginContext 查询是不合适的。
 *
 * 所有 SrcHelper::Get 用于在发生错误时，返回 source_loc。其中：
 * 如果 source_loc 已指定则直接返回；
 * 如果 source_loc 未指定则使用 SourcePawn::IPluginContext 查询，并更新 source_loc。
 *
 * 这可以保证只在发生错误时查询，且只查询一次，从而提高性能。
 */
class SrcHelper final
{
public:
    // 若 source_loc == empty 则 ctx 必须 != nullptr （反之亦然）
    SrcHelper(const spdlog::source_loc &loc) noexcept : SrcHelper(loc, nullptr) {}
    SrcHelper(SourcePawn::IPluginContext *ctx) noexcept : SrcHelper({}, ctx) {}
    SrcHelper(const spdlog::source_loc &loc, SourcePawn::IPluginContext *ctx) noexcept : m_Loc(loc), m_Ctx(ctx) {
        assert(!loc.empty() || ctx);
    }

    [[nodiscard]] spdlog::source_loc Get() const noexcept;
    [[nodiscard]] static spdlog::source_loc GetFromPluginCtx(SourcePawn::IPluginContext *ctx) noexcept;
    [[nodiscard]] static std::vector<std::string> GetStackTrace(SourcePawn::IPluginContext *ctx) noexcept;

private:
    mutable spdlog::source_loc m_Loc;
    SourcePawn::IPluginContext *m_Ctx;
};

class ErrHelper final
{
public:
    void HandleEx(const std::string &origin, const SrcHelper &src, const std::exception &ex) const noexcept;
    void HandleUnknownEx(const std::string &origin, const SrcHelper &src) const noexcept;
    void SetErrHandler(SourceMod::IChangeableForward *handler) noexcept;
    ~ErrHelper() noexcept;
private:
    void ReleaseForward() noexcept;

    SourceMod::IChangeableForward *m_CustomErrorHandler{nullptr};
};


}   // namespace Log4sp
