#ifndef _LOG4SP_ADAPTER_BASE_SINK_H_
#define _LOG4SP_ADAPTER_BASE_SINK_H_

#include "spdlog/common.h"

#include "extension.h"


namespace log4sp {

/**
 * 受限的适配器设计模式
 * 因为无法继承 spdlog 也无法继承 handle
 * 且创建 handle 需要使用对象的指针作为参数，所以 create() 只能是静态的
 *
 * Adapter 角色
 * Target: handle
 * Client: natives
 * Adaptee: raw
 * Adapter: 派生类
 */
class base_sink {
public:
    base_sink(spdlog::sink_ptr sink) : raw_(sink) {}

    const spdlog::sink_ptr &raw() const {
        return raw_;
    }

    const Handle_t handle() const noexcept {
        return handle_;
    }

    /**
     * 成功返回 std::shared_ptr<base_sink>
     * 失败返回 nullptr
     */
    // static std::shared_ptr<base_sink> create(spdlog::sink_ptr sink, IPluginContext *ctx);
    // static std::shared_ptr<base_sink> create(spdlog::sink_ptr sink, const HandleSecurity *security, const HandleAccess *access, HandleError *error);

    static base_sink* read(Handle_t handle, IPluginContext *ctx) {
        HandleSecurity security = {nullptr, myself->GetIdentity()};
        base_sink *sink_adapter_raw;

        HandleError error = handlesys->ReadHandle(handle, g_SinkHandleType, &security, (void **)&sink_adapter_raw);
        if (error != HandleError_None) {
            ctx->ReportError("Invalid sink handle. (hdl=%X, err=%d)", handle, error);
            return nullptr;
        }

        return sink_adapter_raw;
    }

    virtual const bool is_multi_thread() const noexcept = 0;

protected:
    spdlog::sink_ptr raw_;  // Adaptee
    Handle_t handle_;       // Target (调用 create() 创建时自动赋值)
};


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_BASE_SINK_H_
