#ifndef _LOG4SP_ADAPTER_LOGGER_HANDLER_H_
#define _LOG4SP_ADAPTER_LOGGER_HANDLER_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "extension.h"


namespace log4sp {

class logger_proxy;


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
    [[nodiscard]] static logger_handler &instance();

    /**
     * @brief 用于 SDK_OnLoad 时创建 handle type。
     *
     * @note  需要与 destroy 配对使用。
     *
     * @return          HandleError error code.
     * @exception       Logger handle type 已存在，或创建失败。
     */
    static void initialize();

    /**
     * @brief 用于 SDK_OnUnload 时移除 handle type。
     *
     * @note  需要与 initialize 配对使用。
     * @note  为了避免影响其他清理工作，此方法不抛出异常。
     * @note  移除后所有的 logger handle 都将被释放，所以 handles_ 和 loggers_ 会被清空。
     */
    static void destroy();

    /**
     * @brief 获取 handle type
     *
     * @return          handle type 或者 NO_HANDLE_TYPE 代表还没创建或创建失败
     */
    [[nodiscard]] HandleType_t handle_type() const;

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
    [[nodiscard]] Handle_t create_handle(std::shared_ptr<logger_proxy> object,
                                         const HandleSecurity *security,
                                         const HandleAccess *access,
                                         HandleError *error);

    /**
     * @brief handlesys->ReadHandle 的适配器
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          object 智能指针或 nullptr 表示读取失败.
     */
    [[nodiscard]] std::shared_ptr<logger_proxy> read_handle(Handle_t handle,
                                                            HandleSecurity *security,
                                                            HandleError *error);

    /**
     * @brief 根据 name 查找 handle
     *
     * @param name      logger 对象的名称
     * @return          logger 对象的 handle 或 BAD_HANDLE 表示不存在
     */
    [[nodiscard]] Handle_t find_handle(const std::string &name);

    /**
     * @brief 根据 name 查找 logger
     *
     * @param name      logger 对象的名称
     * @return          logger 对象的智能指针或 nullptr 表示不存在
     */
    [[nodiscard]] std::shared_ptr<logger_proxy> find_logger(const std::string &name);

    /**
     * @brief 返回所有 logger 名称组成的数组
     *
     * @note 拓展启动时就注册了一个全局 logger，所以这个方法返回的数组大小至少为 1
     *
     * @return          所有 logger 名称组成的数组
     */
    [[nodiscard]] std::vector<std::string> get_all_logger_names();

    /**
     * Apply a user defined function on all logger handles.
     * Example:
     *      apply_all([&](std::shared_ptr<spdlog::logger> l) {l->flush();});
     */
    void apply_all(const std::function<void(std::shared_ptr<logger_proxy>)> &fun);

    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(HandleType_t type, void *object) override;

private:
    logger_handler() : handle_type_(NO_HANDLE_TYPE) {}
    ~logger_handler();

    logger_handler(const logger_handler&) = delete;
    logger_handler& operator=(const logger_handler&) = delete;

    void initialize_();
    void destroy_();

    HandleType_t handle_type_;
    std::unordered_map<std::string, Handle_t> handles_;
    std::unordered_map<std::string, std::shared_ptr<logger_proxy>> loggers_;
};


}       // namespace log4sp
#include "log4sp/adapter/logger_handler-inl.h"
#endif  // _LOG4SP_ADAPTER_LOGGER_HANDLER_H_
