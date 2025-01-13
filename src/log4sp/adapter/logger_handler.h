#ifndef _LOG4SP_ADAPTER_LOGGER_HANDLER_H_
#define _LOG4SP_ADAPTER_LOGGER_HANDLER_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "extension.h"


namespace log4sp {

class logger;


/**
 * SourceMod handlesys 的适配器
 * 原版的 handlesys 不便于管理智能指针对象的生命周期
 * 所以这个类增强了对智能指针对象生命周期的管理
 * 同时提供了 find_handle 方法用于根据 logger name 查找已创建的 handle
 */
class logger_handler final : public IHandleTypeDispatch {
public:
    /**
     * @brief 全局单例对象
     */
    [[nodiscard]] static logger_handler &instance() noexcept;

    /**
     * @brief 用于 SDK_OnLoad 时创建 handle type。
     * @note  需要与 destroy 配对使用。
     *
     * @exception       Logger handle type 已存在，或创建失败。
     */
    static void initialize();

    /**
     * @brief 用于 SDK_OnUnload 时移除 handle type。
     * @note  需要与 initialize 配对使用。
     * @note  为了避免影响其他清理工作，此方法不抛出异常。
     * @note  移除后所有的 logger handle 都将被释放，所以 handles_ 和 loggers_ 会被清空。
     */
    static void destroy() noexcept;

    /**
     * @brief 获取 handle type
     *
     * @return          handle type 或者 NO_HANDLE_TYPE 代表还没创建或创建失败
     */
    [[nodiscard]] HandleType_t handle_type() const noexcept;

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
    [[nodiscard]] Handle_t create_handle(std::shared_ptr<logger> object,
                                         const HandleSecurity *security,
                                         const HandleAccess *access,
                                         HandleError *error) noexcept;

    /**
     * @brief handlesys->ReadHandle 的适配器
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          object 智能指针或 nullptr 表示读取失败.
     */
    [[nodiscard]] std::shared_ptr<logger> read_handle(Handle_t handle,
                                                      HandleSecurity *security,
                                                      HandleError *error) const noexcept;

    /**
     * @brief handlesys->ReadHandle 的适配器
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          object 智能指针或 nullptr 表示读取失败.
     */
    [[nodiscard]] logger *read_handle_raw(Handle_t handle,
                                          HandleSecurity *security,
                                          HandleError *error) const noexcept;

    /**
     * @brief 根据 name 查找 handle
     *
     * @param name      logger 对象的名称
     * @return          logger 对象的 handle 或 BAD_HANDLE 表示不存在
     */
    [[nodiscard]] Handle_t find_handle(const std::string &name) const noexcept;

    /**
     * @brief 根据 name 查找 logger
     *
     * @param name      logger 对象的名称
     * @return          logger 对象的智能指针或 nullptr 表示不存在
     */
    [[nodiscard]] std::shared_ptr<logger> find_logger(const std::string &name) const noexcept;

    /**
     * Apply a user defined function on all logger handles.
     */
    void apply_all(const std::function<void(const Handle_t)> &fun);

    /**
     * Apply a user defined function on all logger handles.
     * Example:
     *      apply_all([&](std::shared_ptr<spdlog::logger> l) {l->flush();});
     */
    void apply_all(const std::function<void(std::shared_ptr<logger>)> &fun);

    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(HandleType_t type, void *object) override;

    logger_handler(const logger_handler &) = delete;
    logger_handler &operator=(const logger_handler &) = delete;

private:
    logger_handler() = default;
    ~logger_handler() = default;

    void initialize_();
    void destroy_() noexcept;

    HandleType_t handle_type_{NO_HANDLE_TYPE};
    std::unordered_map<std::string, Handle_t> handles_;
    std::unordered_map<std::string, std::shared_ptr<logger>> loggers_;
};


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_LOGGER_HANDLER_H_
