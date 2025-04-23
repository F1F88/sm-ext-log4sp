#pragma once

#include <memory>
#include <unordered_map>

#include "log4sp/common.h"
#include "log4sp/sinks/base_sink.h"


namespace log4sp {

/**
 * SourceMod handlesys 的适配器
 * 原版的 handlesys 不便于管理智能指针对象的生命周期
 * 所以这个类增强了对智能指针对象生命周期的管理
 */
class sink_handler final : public SourceMod::IHandleTypeDispatch {
public:
    /**
     * @brief 全局单例对象
     */
    [[nodiscard]] static sink_handler &instance() noexcept;

    /**
     * @brief 用于 SDK_OnLoad 时创建 handle type。
     *
     * @exception       Sink handle type 已存在，或创建失败。
     * @note            需要与 destroy 配对使用。
     */
    static void initialize();

    /**
     * @brief 用于 SDK_OnUnload 时移除 handle type。
     *
     * @note            需要与 initialize 配对使用。
     * @note            为了避免影响其他清理工作，此方法不抛出异常。
     * @note            移除后所有的 sink handle 都将被释放，所以 handles_ 和 sinks_ 会被清空。
     */
    static void destroy() noexcept;

    /**
     * @brief 获取 handle type
     *
     * @return          handle type 或者 NO_HANDLE_TYPE 代表还没创建或创建失败
     */
    [[nodiscard]] SourceMod::HandleType_t handle_type() const noexcept;

    /**
     * @brief handlesys->CreateHandleEx 的适配器
     *        除了创建 handle，还会保存对象的智能指针，以延长生命周期
     *
     * @param object    Object to bind to the handle.
     * @param security  Security pointer; pOwner is written as the owner,
     *                  pIdent is used as the parent identity for authorization.
     * @param access    Access right descriptor for the Handle; NULL for type defaults.
     * @param error     Optional pointer to store an error code on failure (undefined on success).
     * @return          object 对象的 handle 或 BAD_HANDLE 表示创建失败
     */
    [[nodiscard]] SourceMod::Handle_t create_handle(sink_ptr object,
                                                    const SourceMod::HandleSecurity *security,
                                                    const SourceMod::HandleAccess *access,
                                                    SourceMod::HandleError *error) noexcept;

    /**
     * @brief handlesys->ReadHandle 的适配器
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          object 智能指针或 nullptr 表示读取失败.
     */
    [[nodiscard]] sink_ptr read_handle(SourceMod::Handle_t handle,
                                       SourceMod::HandleSecurity *security,
                                       SourceMod::HandleError *error) const noexcept;

    /**
     * @brief handlesys->ReadHandle 的适配器
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          object 指针或 nullptr 表示读取失败.
     */
    [[nodiscard]] sinks::base_sink *read_handle_raw(SourceMod::Handle_t handle,
                                                    SourceMod::HandleSecurity *security,
                                                    SourceMod::HandleError *error) const noexcept;

    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(SourceMod::HandleType_t type, void *object) override;

    sink_handler(const sink_handler &) = delete;
    sink_handler &operator=(const sink_handler &) = delete;
    sink_handler(const sink_handler &&) = delete;

private:
    sink_handler() = default;
    ~sink_handler() = default;

    void initialize_();
    void destroy_() noexcept;

    SourceMod::HandleType_t handle_type_{NO_HANDLE_TYPE};
    std::unordered_map<sinks::base_sink*, SourceMod::Handle_t> handles_;
    std::unordered_map<sinks::base_sink*, sink_ptr> sinks_;
};


}       // namespace log4sp
