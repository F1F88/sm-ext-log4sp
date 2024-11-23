#ifndef _LOG4SP_UTILS_H_
#define _LOG4SP_UTILS_H_

#include "extension.h"


namespace log4sp {

/**
 * 将 cell_t 转为 spdlog::level::level_enum
 * 如果 cell_t 越界，则返回最近的边界值
 */
spdlog::level::level_enum cell_to_level(cell_t lvl);

spdlog::source_loc GetScriptedLoc(IPluginContext *ctx);

std::vector<std::string> GetStackTrace(IPluginContext *ctx);

std::string FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param);

fmt::memory_buffer FormatParams(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param);


} // namespace log4sp

#endif // _LOG4SP_UTILS_H_
