#ifndef _LOG4SP_UTILS_H_
#define _LOG4SP_UTILS_H_

#include "extension.h"


namespace log4sp {

/**
 * 将 cell_t 转为 spdlog::level::level_enum
 * 如果 cell_t 越界，则返回最近的边界值
 * 记得释放传入的 frame 参数
 */
spdlog::level::level_enum cell_to_level(cell_t lvl);

/**
 * 获取插件调用 native 的源代码位置
 * 如果找不到返回空，可以用 empty() 判断是否为空
 * 记得释放传入的 frame 参数
 */
spdlog::source_loc get_script_source_loc(IFrameIterator *iter);

std::vector<std::string> get_stack_trace(IFrameIterator *iter);

std::string FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param);

fmt::memory_buffer FormatParams(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param);


} // namespace log4sp

#endif // _LOG4SP_UTILS_H_
