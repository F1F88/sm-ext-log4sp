#pragma once

#include "log4sp/logger.h"
#include "log4sp/adapter/handle_type_dispatch_adapter.h"


namespace log4sp {
class logger_handler final : public handle_type_dispatch_adapter<std::string, logger> {
public:
    /**
     * @brief 全局单例对象
     */
    [[nodiscard]]
    static logger_handler &instance() noexcept {
        static logger_handler instance;
        return instance;
    }

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
     * @note  移除后所有的 logger handle 都将被释放，所以 datas_ 会被清空。
     */
    static void destroy() noexcept;

    /**
     * Apply a user defined function on all logger handles.
     */
    void apply_all(const std::function<void(const std::pair<SourceMod::Handle_t, std::shared_ptr<logger>>)> &fun) {
        for (auto &data : datas_) {
            fun(data.second);
        }
    }

    auto find_handle(const key_t &key) const noexcept -> handle_t {
        auto found = datas_.find(key);
        return (found == datas_.end()) ? BAD_HANDLE : found->second.first;
    }

    auto find_object(const key_t &key) const noexcept -> object_ptr_t {
        auto found = datas_.find(key);
        return (found == datas_.end()) ? nullptr : found->second.second;
    }

    static constexpr const char *HANDLE_TYPE_NAME = "Logger";

    logger_handler(const logger_handler &) = delete;
    logger_handler &operator=(const logger_handler &) = delete;
    logger_handler(const logger_handler &&) = delete;

private:
    logger_handler() = default;
    ~logger_handler() = default;

    void create_global_logger();

    [[nodiscard]]
    const key_t &get_key(object_raw_t &raw) const noexcept override {
        return raw->name();
    }
};


}       // namespace log4sp
