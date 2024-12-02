#ifndef _LOG4SP_ADAPTER_SINGLE_THREAD_SINK_H_
#define _LOG4SP_ADAPTER_SINGLE_THREAD_SINK_H_

#include "log4sp/adapter/base_sink.h"


namespace log4sp {

class single_thread_sink final : public base_sink {
public:
    static std::shared_ptr<single_thread_sink> create(spdlog::sink_ptr sink,
                                                      IPluginContext *ctx);

    static std::shared_ptr<single_thread_sink> create(spdlog::sink_ptr sink,
                                                      const HandleSecurity *security,
                                                      const HandleAccess *access,
                                                      HandleError *error);

    const bool is_multi_thread() const noexcept override {
        return false;
    }

private:
    // 不要手动创建！
    // 这本该是私有的，但为了方便使用 std::make_shared<> 所以公开
    // 虽然可以使用 std::shared_ptr<> 替代，但是会降低性能
    single_thread_sink(spdlog::sink_ptr sink) : base_sink(std::move(sink)) {}
};


}       // namespace log4sp
#include "log4sp/adapter/single_thread_sink-inl.h"
#endif  // _LOG4SP_ADAPTER_SINGLE_THREAD_SINK_H_
