#ifndef _LOG4SP_UTILS_H_
#define _LOG4SP_UTILS_H_

#include "extension.h"


namespace log4sp {

bool CellToLevel(cell_t lvl, spdlog::level::level_enum &result);

spdlog::level::level_enum CellToLevelOrLogWarn(IPluginContext *ctx, cell_t lvl);

spdlog::source_loc GetScriptedLoc(IPluginContext *ctx);

std::vector<std::string> GetStackTrace(IPluginContext *ctx);

std::string FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param);

fmt::memory_buffer FormatParams(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param);


} // namespace log4sp

#endif // _LOG4SP_UTILS_H_
