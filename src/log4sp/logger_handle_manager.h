#ifndef _LOG4SP_LOGGER_HANDLE_MANAGER_H_
#define _LOG4SP_LOGGER_HANDLE_MANAGER_H_

#include <unordered_map>

#include "spdlog/async.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <extension.h>


namespace log4sp {

struct logger_handle_data
{
    std::shared_ptr<spdlog::logger> logger;     // 智能指针（管理生命周期）
    bool isMultiThreaded;
    Handle_t handle;
    HandleType_t handleType;
    IChangeableForward *custom_err_forward_;    // 只有在调用 SetErrorHandler 后才不为 NULL

    constexpr bool empty() const noexcept
    {
        return logger == nullptr;
    }
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
     * 根据 name 获取 完整数据。
     *
     * @return      完整数据 或 空 代表参数 指针 无效 (可以使用 empty 方法判断是否为空)
     */
    logger_handle_data get_data(const std::string &logger_name);

    /**
     * 根据 handle 获取 完整数据。
     * 如果 handle 无效，则会中断 sourcepawn 代码，并记录错误信息。
     * 这是 read_handle() 的包装器。
     *
     * @return      完整数据 或 空 代表参数 handle 无效 (可以使用 empty 方法判断是否为空)
     */
    logger_handle_data get_data(IPluginContext *ctx, Handle_t handle);

    /**
     * 根据 handle 获取 handle type。
     *
     * @note 目前只有一种 logger handle type
     *
     * @return      handle 或 NO_HANDLE_TYPE 代表参数 handle 无效
     */
    HandleType_t get_handle_type(Handle_t handle);

    /**
     * 根据 handle 获取 指针。
     * 如果 handle 无效，则会中断 sourcepawn 代码，并记录错误信息。
     *
     * @return      指针 或 nullptr 代表参数 handle 无效
     */
    spdlog::logger* read_handle(IPluginContext *ctx, Handle_t handle);

    void drop(const std::string &logger_name);

    void drop(IPluginContext *ctx, Handle_t handle);

    void drop_all();

    void shutdown();

    logger_handle_data create_logger_st(IPluginContext *ctx, const char *name, std::vector<spdlog::sink_ptr> sinks);
    logger_handle_data create_logger_mt(IPluginContext *ctx, const char *name, std::vector<spdlog::sink_ptr> sinks, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data create_server_console_logger_st(IPluginContext *ctx, const char *name);
    logger_handle_data create_server_console_logger_mt(IPluginContext *ctx, const char *name, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data create_base_file_logger_st(IPluginContext *ctx, const char *name, const char *filename, bool truncate = false);
    logger_handle_data create_base_file_logger_mt(IPluginContext *ctx, const char *name, const char *filename, bool truncate = false, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data create_rotating_file_logger_st(IPluginContext *ctx,const char *name,const char *filename,size_t max_file_size, size_t max_files, bool rotate_on_open = false);
    logger_handle_data create_rotating_file_logger_mt(IPluginContext *ctx,const char *name,const char *filename,size_t max_file_size, size_t max_files, bool rotate_on_open = false, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

    logger_handle_data create_daily_file_logger_st(IPluginContext *ctx, const char *name, const char *filename, int hour = 0, int minute = 0, bool truncate = false, uint16_t max_files = 0);
    logger_handle_data create_daily_file_logger_mt(IPluginContext *ctx, const char *name, const char *filename, int hour = 0, int minute = 0, bool truncate = false, uint16_t max_files = 0, spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block);

private:
    bool register_logger_handle_(const std::string &key, logger_handle_data data);

    // 用于根据 logger 指针查找完整信息
    // 参考与 <spdlog/details/registry.h>, 但速度应该更快，因为单线程不需要锁
    std::unordered_map<std::string, logger_handle_data> logger_datas_;

    // 用于根据 handle 查找 type
    // std::unordered_map<Handle_t, HandleType_t> handle_types_;
};

} // namespace log4sp

#endif  // _LOG4SP_SINK_REGISTRY_H_
