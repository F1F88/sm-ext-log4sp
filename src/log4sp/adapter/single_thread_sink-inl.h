#ifndef _LOG4SP_ADAPTER_SINGLE_THREAD_SINK_INL_H_
#define _LOG4SP_ADAPTER_SINGLE_THREAD_SINK_INL_H_

#include "log4sp/sink_register.h"

#include "log4sp/adapter/single_thread_sink.h"


namespace log4sp {

inline std::shared_ptr<single_thread_sink> single_thread_sink::create(spdlog::sink_ptr sink,
                                                                      IPluginContext *ctx) {
    HandleSecurity security = {nullptr, myself->GetIdentity()};
    HandleError error;

    std::shared_ptr<single_thread_sink> sink_adapter = create(sink, &security, nullptr, &error);
    if (sink_adapter == nullptr) {
        ctx->ReportError("SM Error! Allocation of single threaded sink handle failed. (err=%d)", error);
        return nullptr;
    }

    return sink_adapter;

}

inline std::shared_ptr<single_thread_sink> single_thread_sink::create(spdlog::sink_ptr sink,
                                                                      const HandleSecurity *security,
                                                                      const HandleAccess *access,
                                                                      HandleError *error) {
    // 1. 创建适配器
    // auto sink_adapter = std::make_shared<single_thread_sink>(sink);
    auto sink_adapter = std::shared_ptr<single_thread_sink>(new single_thread_sink(sink));

    // 2. 为适配器创建 handle
    Handle_t handle = handlesys->CreateHandleEx(g_SinkHandleType, sink_adapter.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return nullptr;
    }

    // 3. 保存 handle
    sink_adapter->handle_ = handle;

    // 4. 注册适配器到 logger register
    sink_register::instance().register_sink(sink_adapter);
    return sink_adapter;
}


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_SINGLE_THREAD_SINK_INL_H_
