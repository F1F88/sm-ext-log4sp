#ifndef _LOG4SP_ADAPTER_SYNC_LOGGER_H_
#define _LOG4SP_ADAPTER_SYNC_LOGGER_H_

#include "log4sp/adapter/base_logger.h"


namespace log4sp {
class base_sink;

class sync_logger final : public base_logger {
public:
    static std::shared_ptr<sync_logger> create(std::shared_ptr<spdlog::logger> logger,
                                               IPluginContext *ctx);

    static std::shared_ptr<sync_logger> create(std::shared_ptr<spdlog::logger> logger,
                                               const HandleSecurity *security,
                                               const HandleAccess *access,
                                               HandleError *error);

    const bool is_async() const noexcept override {
        return false;
    }

    void add_sink(spdlog::sink_ptr sink) override;

    void remove_sink(spdlog::sink_ptr sink) override;

    void set_error_forward(IChangeableForward *forward) override;

    void error_handler(const std::string &msg) override;

private:
    // 不要手动创建！
    // 这本该是私有的，但为了方便使用 std::make_shared<> 所以公开
    // 虽然可以使用 std::shared_ptr<> 替代，但是会降低性能
    sync_logger(std::shared_ptr<spdlog::logger> logger) : base_logger(std::move(logger)) {}
};


}       // namespace log4sp
#include "log4sp/adapter/sync_logger-inl.h"
#endif  // _LOG4SP_ADAPTER_SYNC_LOGGER_H_
