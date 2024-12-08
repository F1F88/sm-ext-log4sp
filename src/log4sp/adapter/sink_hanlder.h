#ifndef _LOG4SP_ADAPTER_SINK_HANDLER_H_
#define _LOG4SP_ADAPTER_SINK_HANDLER_H_

#include <unordered_map>

#include "extension.h"


namespace spdlog {
namespace sinks {

class sink;

}       // namespace spdlog
}       // namespace sinks


namespace log4sp {

/**
 * SourceMod handlesys 的适配器
 * 原版的 handlesys 不便于管理智能指针对象的生命周期
 * 所以这个类增强了对智能指针对象生命周期的管理
 * 原生 spdlog::sinks::sink 可以满足大部分需求，唯一需要做的是延长智能指针的生命周期
 * 不需要对每一种 sink 进行适配
 */
class sink_handler final : public IHandleTypeDispatch {
public:
    /**
     * @brief 全局单例对象
     */
    static sink_handler &instance();

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
    Handle_t create_handle(std::shared_ptr<spdlog::sinks::sink> object,
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
    std::shared_ptr<spdlog::sinks::sink> read_handle(Handle_t handle,
                                                     HandleSecurity *security,
                                                     HandleError *error);

    /**
     * @brief Called when destroying a handle.  Must be implemented.
     *
     * @param type      Handle type.
     * @param object    Handle internal object.
     */
    void OnHandleDestroy(HandleType_t type, void *object) override;

    /**
     * @brief 用于 SDK_OnUnload 时移除 handle type
     *        移除后，SourceMod 应该会 Free 这个 type 下的所有 handle 实例
     *        OnHandleDestroy 里会逐个从 handles_ 和 sinks_ 移除数据
     *        所以最终 handles_ 和 sinks_ 应该是空的
     */
    void remove_handle_type();

private:
    sink_handler() : handle_type_(NO_HANDLE_TYPE) {}
    ~sink_handler();

    sink_handler(const sink_handler&) = delete;
    sink_handler& operator=(const sink_handler&) = delete;

    HandleType_t handle_type_;
    std::unordered_map<spdlog::sinks::sink*, Handle_t> handles_;
    std::unordered_map<spdlog::sinks::sink*, std::shared_ptr<spdlog::sinks::sink>> sinks_;
};


}       // namespace log4sp
#include "log4sp/adapter/sink_handler-inl.h"
#endif  // _LOG4SP_ADAPTER_SINK_HANDLER_H_