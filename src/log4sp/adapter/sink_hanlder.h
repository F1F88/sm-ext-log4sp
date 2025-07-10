#pragma once

#include "spdlog/sinks/sink.h"

#include "log4sp/adapter/handle_type_dispatch_adapter.h"


namespace log4sp {
class sink_handler final : public handle_type_dispatch_adapter<spdlog::sinks::sink*, spdlog::sinks::sink> {
public:

    /**
     * @brief 全局单例对象
     */
    [[nodiscard]]
    static sink_handler &instance() noexcept {
        static sink_handler instance;
        return instance;
    }

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
     * @note            移除后所有的 sink handle 都将被释放，所以 datas_ 会被清空。
     */
    static void destroy() noexcept;

    static constexpr const char *HANDLE_TYPE_NAME = "Sink";

    sink_handler(const sink_handler &) = delete;
    sink_handler &operator=(const sink_handler &) = delete;
    sink_handler(const sink_handler &&) = delete;

private:
    sink_handler() = default;
    ~sink_handler() = default;

    [[nodiscard]]
    const key_t &get_key(object_raw_t &raw) const noexcept override {
        return raw;
    }
};


}       // namespace log4sp
