#ifndef _LOG4SP_COMMON_H_
#define _LOG4SP_COMMON_H_

#include "extension.h"

#include <log4sp/sink_registry.h>


namespace log4sp {
namespace logger {

bool CheckNameOrReportError(IPluginContext *ctx, const char *name);

Handle_t CreateHandleOrReportError(IPluginContext *ctx, std::shared_ptr<spdlog::logger> logger);

spdlog::logger *ReadHandleOrReportError(IPluginContext *ctx, Handle_t handle);

} // namespace logger



namespace sinks {

Handle_t CreateHandleOrReportError(IPluginContext *ctx, HandleType_t type, spdlog::sink_ptr sink);

spdlog::sink_ptr ReadHandleOrReportError(IPluginContext *ctx, Handle_t handle);

} // namespace sinks



bool CellToLevel(cell_t lvl, spdlog::level::level_enum &result);

spdlog::level::level_enum CellToLevelOrLogWarn(IPluginContext *ctx, cell_t lvl);

spdlog::source_loc GetScriptedLoc(IPluginContext *ctx);

std::vector<std::string> GetStackTrace(IPluginContext *ctx);

std::string FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param);

fmt::memory_buffer FormatParams(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param);


} // namespace log4sp

#endif // _LOG4SP_COMMON_H_
