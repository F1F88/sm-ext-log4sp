#ifndef _LOG4SP_ADAPTER_BASE_LOGGER_H_
#define _LOG4SP_ADAPTER_BASE_LOGGER_H_

#include "spdlog/logger.h"

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
class base_logger {
public:
    base_logger(std::shared_ptr<spdlog::logger> logger)
        : raw_(logger), handle_(BAD_HANDLE), error_forward_(nullptr) {}

    ~base_logger() {
        if (error_forward_ != nullptr) {
            forwards->ReleaseForward(error_forward_);
        }
    }

    const std::shared_ptr<spdlog::logger> &raw() const {
        return raw_;
    }

    const Handle_t handle() const noexcept {
        return handle_;
    }

    /**
     * 成功返回 std::shared_ptr<base_logger>
     * 失败返回 nullptr
     */
    // static std::shared_ptr<base_logger> create(std::shared_ptr<spdlog::logger> logger, IPluginContext *ctx);
    // static std::shared_ptr<base_logger> create(std::shared_ptr<spdlog::logger> logger, const HandleSecurity *security, const HandleAccess *access, HandleError *error);

    static base_logger* read(Handle_t handle, IPluginContext *ctx) {
        HandleSecurity security = {nullptr, myself->GetIdentity()};
        base_logger *logger_adapter_raw;

        HandleError error = handlesys->ReadHandle(handle, g_LoggerHandleType, &security, (void **)&logger_adapter_raw);
        if (error != HandleError_None) {
            ctx->ReportError("Invalid logger handle. (hdl=%X, err=%d)", handle, error);
            return nullptr;
        }

        return logger_adapter_raw;
    }

    virtual const bool is_async() const noexcept = 0;

    // * 以下函数存在并发问题，由 Adapter 重写解决

    virtual void add_sink(spdlog::sink_ptr sink) = 0;

    virtual void remove_sink(spdlog::sink_ptr sink) = 0;

    // 隐藏 spdlog::logger 的 set_error_forward()
    virtual void set_error_forward(IChangeableForward *forward) = 0;

    // 替代 spdlog::logger 的 err_handler_()
    // sink_it_() 和 flush_() 将在捕获到异常时调用此函数
    virtual void error_handler(const std::string &msg) = 0;

protected:
    std::shared_ptr<spdlog::logger> raw_;   // Adaptee
    Handle_t handle_;                       // Target (调用 create() 创建时自动赋值)
    IChangeableForward *error_forward_;     // set_error_forward() 赋值
};


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_BASE_LOGGER_H_
