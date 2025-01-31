#pragma once

#include <vector>

#include "spdlog/common.h"

#include "extension.h"


namespace log4sp {

/**
 * 将一个长度为 2 的 cell_t 数组转换为 int64
 */
[[nodiscard]] int64_t cell_to_int64(const cell_t arr[2]) noexcept;

/**
 * 将 cell_t 转为 spdlog::level::level_enum
 * 如果 cell_t 越界，则返回最近的边界值
 */
[[nodiscard]] spdlog::level::level_enum cell_to_level(cell_t lvl) noexcept;

/**
 * 将 cell_t 转为 spdlog::pattern_time_type
 * 如果 cell_t 越界，则返回最近的边界值
 */
[[nodiscard]] spdlog::pattern_time_type cell_to_pattern_time_type(cell_t type) noexcept;

/**
 * 获取插件调用 native 的源代码位置
 * 如果找不到返回空，可以用 empty() 判断是否为空
 */
[[nodiscard]] spdlog::source_loc get_plugin_source_loc(IPluginContext *ctx);

/**
 * 获取堆栈信息
 * 参考自: sourcemod DebugReport::GetStackTrace
 */
[[nodiscard]] std::vector<std::string> get_stack_trace(IPluginContext *ctx);

/**
 * 格式化 params 数组，风格与 AMXTpl 一致，但是格式化长度不受限制
 * param 指向的参数是格式模板，后面的参数是可变参数
 * 当格式与参数不匹配时抛出异常
 * 这是 format_cell_to_memory_buf 的包装器
 */
[[nodiscard]] std::string format_cell_to_string(IPluginContext *ctx, const cell_t *params, unsigned int param);

/**
 * 格式化 params 数组，风格与 AMXTpl 一致，但是格式化长度不受限制
 * param 指向的参数是可变参数
 * 当格式与参数不匹配时抛出异常
 */
[[nodiscard]] spdlog::fmt_lib::memory_buffer format_cell_to_memory_buf(const char *format, IPluginContext *ctx, const cell_t *params, int *param);


}       // namespace log4sp
