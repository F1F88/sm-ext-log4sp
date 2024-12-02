#ifndef _LOG4SP_SINK_REGISTER_H_
#define _LOG4SP_SINK_REGISTER_H_

#include <unordered_map>

#include "spdlog/common.h"

#include "extension.h"

#include "log4sp/adapter/base_sink.h"


namespace log4sp {

/**
 * 参考 "spdlog/details/registry.h"
 * 但速度应该更快，因为单线程不需要锁
 */
class sink_register final {
public:
    static sink_register &instance() {
        static sink_register singleInstance;
        return singleInstance;
    }

    void register_sink(std::shared_ptr<base_sink> sink_adapter) {
        sinks_[sink_adapter.get()] = std::move(sink_adapter);
    }

    std::shared_ptr<base_sink> get(base_sink *sink_adapter_raw) {
        auto found = sinks_.find(sink_adapter_raw);
        return found == sinks_.end() ? nullptr : found->second;
    }

    /**
     * 应该只在 SinkHandler::OnHandleDestroy 中使用
     */
    void drop(base_sink *sink_adapter_raw) {
        sinks_.erase(sink_adapter_raw);
    }

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     */
    void drop_all() {
        sinks_.clear();
    }

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     * drop_all() 的包装器
     */
    void shutdown() {
        drop_all();
    }

private:
    sink_register() {}
    ~sink_register() {}

    std::unordered_map<base_sink *, std::shared_ptr<base_sink>> sinks_;
};


}       // namespace log4sp
#endif  // _LOG4SP_SINK_REGISTER_H_
