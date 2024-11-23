#ifndef _LOG4SP_LOGGER_HANDLE_MANAGER_H_
#define _LOG4SP_LOGGER_HANDLE_MANAGER_H_

#include <unordered_map>

#include "spdlog/async.h"

#include <extension.h>


namespace log4sp {

struct logger_handle_data {
public:
    const std::shared_ptr<spdlog::logger> &logger() const noexcept {
        return logger_;
    }

    const bool is_multi_threaded() const noexcept {
        return is_multi_threaded_;
    }

    const Handle_t handle() const noexcept {
        return handle_;
    }

    const HandleType_t handle_type() const noexcept {
        return handle_type_;
    }

    const IChangeableForward& error_forward() const noexcept {
        return *error_forward_;
    }

    void set_error_handler(IChangeableForward *forward) {
        if (error_forward_ != NULL)
        {
            forwards->ReleaseForward(error_forward_);
        }

        error_forward_ = forward;
    }

private:
    logger_handle_data(std::shared_ptr<spdlog::logger> logger, bool is_multi_threaded, Handle_t handle, HandleType_t type, IChangeableForward *forward)
        : logger_(logger), is_multi_threaded_(is_multi_threaded), handle_(handle), handle_type_(type), error_forward_(forward) {}

    ~logger_handle_data() {
        if (error_forward_ != NULL) {
            forwards->ReleaseForward(error_forward_);
        }
    }

    std::shared_ptr<spdlog::logger> logger_;// 智能指针（管理生命周期）
    bool is_multi_threaded_;
    Handle_t handle_;
    HandleType_t handle_type_;
    IChangeableForward *error_forward_;     // 只有在调用 SetErrorHandler 后才不为 NULL

    friend class logger_handle_manager;
};



/**
 * 由于 sourcemod 的 handlesys 只提供了通过 handle 查找原始指针的方法，所以添加此类用于反向查找 handle 数据。
 * 还封装了工厂方法，所有 native 中创建的 logger handle 都使用这些工厂方法来创建对象，方便管理生命周期。
 */
class logger_handle_manager
{
public:
    static logger_handle_manager &instance();

    /**
     * 用于
     *      - 根据 name 获取 完整数据。
     *      - 检查 logger name 是否已存在。
     *      - logger 原始指针的接口不足以满足需求时。
     *
     * @return      logger_handle_data 或 nullptr 代表没有名为 logger_name 的数据
     */
    logger_handle_data* get_data(const std::string &logger_name);

    /**
     * 根据 handle 获取 handle type。
     * @note 目前只有一种 logger handle type
     *
     * @return      handle 或 NO_HANDLE_TYPE 代表参数 handle 无效
     */
    HandleType_t get_handle_type(Handle_t handle);

    /**
     * 根据 handle 获取 指针。
     * 如果 handle 无效，则会中断 sourcepawn 代码，并记录错误信息。
     *
     * @return      logger 或 nullptr 代表参数 handle 无效
     */
    spdlog::logger* read_handle(IPluginContext *ctx, Handle_t handle);

    /**
     * 应该只在 LoggerHandler::OnHandleDestroy 中使用
     * 默认 logger 不应该触发 LoggerHandler::OnHandleDestroy
     */
    void drop(const std::string &logger_name);

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     */
    void drop_all();

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     * drop_all() 的包装器
     */
    void shutdown();

    logger_handle_data* create_logger_st(IPluginContext *ctx, std::string name, std::vector<spdlog::sink_ptr> sinks);
    logger_handle_data* create_logger_mt(IPluginContext *ctx, std::string name, std::vector<spdlog::sink_ptr> sinks, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data* create_server_console_logger_st(IPluginContext *ctx, std::string name);
    logger_handle_data* create_server_console_logger_mt(IPluginContext *ctx, std::string name, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data* create_base_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, bool truncate = false);
    logger_handle_data* create_base_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, bool truncate = false, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data* create_rotating_file_logger_st(IPluginContext *ctx,std::string name,std::string filename,size_t max_file_size, size_t max_files, bool rotate_on_open = false);
    logger_handle_data* create_rotating_file_logger_mt(IPluginContext *ctx,std::string name,std::string filename,size_t max_file_size, size_t max_files, bool rotate_on_open = false, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data* create_daily_file_logger_st(IPluginContext *ctx, std::string name, std::string filename, int hour = 0, int minute = 0, bool truncate = false, uint16_t max_files = 0);
    logger_handle_data* create_daily_file_logger_mt(IPluginContext *ctx, std::string name, std::string filename, int hour = 0, int minute = 0, bool truncate = false, uint16_t max_files = 0, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

private:
    logger_handle_manager() {}

    void register_logger_handle_(const std::string &key, log4sp::logger_handle_data *data);

    // 用于根据 logger 指针查找完整信息
    // 参考与 <spdlog/details/registry.h>, 但速度应该更快，因为单线程不需要锁
    std::unordered_map<std::string, logger_handle_data*> logger_datas_;

    // 用于根据 handle 查找 type
    // std::unordered_map<Handle_t, HandleType_t> handle_types_;
};

} // namespace log4sp

#include "logger_handle_manager-inl.h"

#endif  // _LOG4SP_LOGGER_HANDLE_MANAGER_H_
