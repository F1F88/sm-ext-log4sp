#ifndef _LOG4SP_ADAPTER_ASYNC_LOGGER_H_
#define _LOG4SP_ADAPTER_ASYNC_LOGGER_H_

#include "spdlog/async_logger.h"

#include "log4sp/adapter/base_logger.h"


namespace log4sp {

class async_logger final : public base_logger {
public:
    static std::shared_ptr<async_logger> create(std::shared_ptr<spdlog::async_logger> logger,
                                               IPluginContext *ctx);

    static std::shared_ptr<async_logger> create(std::shared_ptr<spdlog::async_logger> logger,
                                               const HandleSecurity *security,
                                               const HandleAccess *access,
                                               HandleError *error);

    const bool is_async() const noexcept override {
        return true;
    }

    void add_sink(spdlog::sink_ptr sink) override;

    void remove_sink(spdlog::sink_ptr sink) override;

    void set_error_forward(IChangeableForward *forward) override;

    void error_handler(const std::string &msg) override;

private:
    // 不要手动创建！
    // 这本该是私有的，但为了方便使用 std::make_shared<> 所以公开
    // 虽然可以使用 std::shared_ptr<> 替代，但是会降低性能
    async_logger(std::shared_ptr<spdlog::logger> logger) : base_logger(std::move(logger)) {}

    std::mutex error_handler_mutex_;
};


}       // namespace log4sp
#include "log4sp/adapter/async_logger-inl.h"
#endif  // _LOG4SP_ADAPTER_ASYNC_LOGGER_H_
