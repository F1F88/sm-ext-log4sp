#ifndef _LOG4SP_UTILS_H_
#define _LOG4SP_UTILS_H_

#include "extension.h"

#include "spdlog/common.h"
#include "spdlog/async_logger.h"


namespace log4sp {

/**
 * 将 cell_t 转为 spdlog::level::level_enum
 * 如果 cell_t 越界，则返回最近的边界值
 */
spdlog::level::level_enum cell_to_level(cell_t lvl);

/**
 * 将 cell_t 转为 spdlog::async_overflow_policy
 * 如果 cell_t 越界，则返回最近的边界值
 */
spdlog::async_overflow_policy cell_to_policy(cell_t policy);

/**
 * 将 cell_t 转为 spdlog::pattern_time_type
 * 如果 cell_t 越界，则返回最近的边界值
 */
spdlog::pattern_time_type cell_to_pattern_time_type(cell_t type);

/**
 * 获取插件调用 native 的源代码位置
 * 如果找不到返回空，可以用 empty() 判断是否为空
 */
spdlog::source_loc get_plugin_source_loc(IPluginContext *ctx);

/**
 * 获取堆栈信息
 * 参考自: sourcemod DebugReport::GetStackTrace
 */
std::vector<std::string> get_stack_trace(IPluginContext *ctx);

std::string FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param);

fmt::memory_buffer FormatParams(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param);


} // namespace log4sp

#endif // _LOG4SP_UTILS_H_
