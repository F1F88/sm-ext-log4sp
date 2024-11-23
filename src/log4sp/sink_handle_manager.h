#ifndef _LOG4SP_SINK_HANDLE_MANAGER_H_
#define _LOG4SP_SINK_HANDLE_MANAGER_H_

#include <unordered_map>

#include <extension.h>


namespace log4sp {

struct sink_handle_data {
public:
    const spdlog::sink_ptr &sink_ptr() const noexcept {
        return sink_ptr_;
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

private:
    sink_handle_data(spdlog::sink_ptr sink, bool is_multi_threaded, Handle_t handle, HandleType_t type)
        : sink_ptr_(sink), is_multi_threaded_(is_multi_threaded), handle_(handle), handle_type_(type) {}

    ~sink_handle_data() {}

    spdlog::sink_ptr sink_ptr_; // 智能指针（管理生命周期）
    bool is_multi_threaded_;
    Handle_t handle_;
    HandleType_t handle_type_;

    friend class sink_handle_manager;
};



/**
 * 由于 sourcemod 的 handlesys 只提供了通过 handle 查找原始指针的方法，所以添加此类用于反向查找 handle 数据。
 * 还封装了工厂方法，所有 native 中创建的 sink handle 都使用这些工厂方法来创建对象，方便管理生命周期。
 */
class sink_handle_manager
{
public:
    static sink_handle_manager &instance();

    /**
     * 用于
     *      - 根据 sink 原始指针获取 完整数据。
     *      - sink 原始指针的接口不足以满足需求时。
     *
     * @return      sink_handle_data 或 nullptr 代表参数 sink 无效
     */
    sink_handle_data* get_data(spdlog::sinks::sink *sink);

    /**
     * 根据 handle 获取 handle type。
     *
     * @return      handle 或 NO_HANDLE_TYPE 代表参数 handle 无效
     */
    HandleType_t get_handle_type(Handle_t handle);

    /**
     * 根据 handle 获取 原始指针。
     * 如果 handle 无效，则会中断 sourcepawn 代码，并记录错误信息。
     *
     * @return      sink 原始指针或 nullptr 代表参数 handle 无效
     */
    spdlog::sinks::sink* read_handle(IPluginContext *ctx, Handle_t handle);

    /**
     * 应该只在 SinkHandler::OnHandleDestroy 中使用
     */
    void drop(spdlog::sinks::sink *sink);

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     */
    void drop_all();

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     * drop_all() 的包装器
     */
    void shutdown();

    sink_handle_data* create_server_console_sink_st(IPluginContext *ctx);
    sink_handle_data* create_server_console_sink_mt(IPluginContext *ctx);

    sink_handle_data* create_base_file_sink_st(IPluginContext *ctx, const char *filename, bool truncate = false);
    sink_handle_data* create_base_file_sink_mt(IPluginContext *ctx, const char *filename, bool truncate = false);

    sink_handle_data* create_rotating_file_sink_st(IPluginContext *ctx, const char *base_filename, size_t max_size, size_t max_files, bool rotate_on_open = false);
    sink_handle_data* create_rotating_file_sink_mt(IPluginContext *ctx, const char *base_filename, size_t max_size, size_t max_files, bool rotate_on_open = false);

    sink_handle_data* create_daily_file_sink_st(IPluginContext *ctx, const char *base_filename, int rotation_hour, int rotation_minute, bool truncate = false, uint16_t max_files = 0);
    sink_handle_data* create_daily_file_sink_mt(IPluginContext *ctx, const char *base_filename, int rotation_hour, int rotation_minute, bool truncate = false, uint16_t max_files = 0);

    sink_handle_data* create_client_console_sink_st(IPluginContext *ctx);
    sink_handle_data* create_client_console_sink_mt(IPluginContext *ctx);

    sink_handle_data* create_client_chat_sink_st(IPluginContext *ctx);
    sink_handle_data* create_client_chat_sink_mt(IPluginContext *ctx);

private:
    sink_handle_manager() {}

    void register_sink_handle_(spdlog::sinks::sink *key, sink_handle_data *data);

    // 用于根据 sink 查找完整信息
    std::unordered_map<spdlog::sinks::sink*, sink_handle_data*> sink_datas_;

    // 用于根据 handle 查找 type
    std::unordered_map<Handle_t, HandleType_t> handle_types_;
};


} // namespace log4sp

#endif  // _LOG4SP_SINK_REGISTRY_H_
