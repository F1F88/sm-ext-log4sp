#ifndef _LOG4SP_ADAPTER_LOGGER_HANDLER_H_
#define _LOG4SP_ADAPTER_LOGGER_HANDLER_H_

#include <unordered_map>

#include "extension.h"


namespace spdlog {
    class logger;
}


namespace log4sp {

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
    static logger_handler &instance();

    /**
     * @brief 用于 SDK_OnLoad 时创建 handle type
     *
     * @return          HandleError error code.
     */
    HandleError create_handle_type();

    /**
     * @brief 获取 handle type
     *
     * @return          handle type 或者 NO_HANDLE_TYPE 代表还没创建或创建失败
     */
    HandleType_t handle_type() const;

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
    Handle_t create_handle(std::shared_ptr<spdlog::logger> object,
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
    std::shared_ptr<spdlog::logger> read_handle(Handle_t handle,
                                                HandleSecurity *security,
                                                HandleError *error);

    /**
     * @brief 根据 logger name 查找是否已创建为 handle
     *
     * @param name      logger 对象的名称
     * @return          logger 对象的 handle 或 BAD_HANDLE 表示不存在
     */
    Handle_t find_handle(const std::string &name);

    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(HandleType_t type, void *object) override;

    /**
     * @brief 用于 SDK_OnUnload 时移除 handle type。
     *        移除后，SourceMod 应该会 Free 这个 type 下的所有 handle 实例
     *        OnHandleDestroy 里会逐个从 handles_ 和 loggers_ 移除数据
     *        所以最终 handles_ 和 loggers_ 应该是空的
     */
    void remove_handle_type();

private:
    logger_handler() : handle_type_(NO_HANDLE_TYPE) {}
    ~logger_handler();

    logger_handler(const logger_handler&) = delete;
    logger_handler& operator=(const logger_handler&) = delete;

    HandleType_t handle_type_;
    std::unordered_map<std::string, Handle_t> handles_;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
};


}       // namespace log4sp
#include "log4sp/adapter/logger_handler-inl.h"
#endif  // _LOG4SP_ADAPTER_LOGGER_HANDLER_H_
