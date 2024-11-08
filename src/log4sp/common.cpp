#include <log4sp/common.h>

namespace log4sp {
namespace logger {

bool CheckNameOrReportError(IPluginContext *ctx, const char *name)
{
    // Creating Logger with the same name will cause srcds(Source Dedicated Server) to crash
    if (spdlog::get(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return false;
    }
    return true;
}

Handle_t CreateHandleOrReportError(IPluginContext *ctx, std::shared_ptr<spdlog::logger> logger)
{
    HandleError error;
    HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
    Handle_t handle = handlesys->CreateHandleEx(g_LoggerHandleType, logger.get(), &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        spdlog::dump_backtrace();       // dump message is used to troubleshoot problems
        spdlog::drop(logger->name());   // Don't forget to drop the Logger you just created

        ctx->ReportError("Allocation of logger handle failed. (error %d)", error);
        return BAD_HANDLE;
    }

    SPDLOG_TRACE("Logger handle created successfully. (name={}, ptr={}, type={}, hdl={})", logger->name(), fmt::ptr(logger.get()), g_LoggerHandleType, handle);
    return handle;
}

spdlog::logger *ReadHandleOrReportError(IPluginContext *ctx, Handle_t handle)
{
    spdlog::logger *logger;
    HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error = handlesys->ReadHandle(handle, g_LoggerHandleType, &sec, (void **)&logger);
    if (error != HandleError_None)
    {
        ctx->ReportError("Invalid logger handle. (hdl=0x%x, error=%d)", handle, error);
        return nullptr;
    }
    return logger;
}

} // namespace logger



namespace sinks {

Handle_t CreateHandleOrReportError(IPluginContext *ctx, HandleType_t type, spdlog::sink_ptr sink)
{
    HandleError error;
    HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
    Handle_t handle = handlesys->CreateHandleEx(type, sink.get(), &sec, NULL, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of sink handle failed. (error %d)", error);
        return BAD_HANDLE;
    }

    // 多重保护，超级无敌360度托马斯回旋安全
    if (SinkHandleRegistry::instance().hasKey(handle)) // So, 什么情况下会发生这个问题呢？
    {
        ctx->ReportError("Sink with handle (0x%x) already exists.", handle);
        return BAD_HANDLE;
    }

    log4sp::SinkHandleRegistry::instance().registerSink(handle, type, sink);
    SPDLOG_TRACE("Sink handle created successfully. (ptr={}, type={}, hdl={})", fmt::ptr(sink.get()), type, handle);
    return handle;
}

spdlog::sink_ptr ReadHandleOrReportError(IPluginContext *ctx, Handle_t handle)
{
    SinkHandleInfo *info = SinkHandleRegistry::instance().get(handle);
    if (info == nullptr)
    {
        ctx->ReportError("The sink handle (0x%x) is not registered.", handle);
        return nullptr;
    }

    spdlog::sinks::sink *sink;
    HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error = handlesys->ReadHandle(handle, info->type, &sec, (void **)&sink);
    if (error != HandleError_None)
    {
        ctx->ReportError("Invalid sink handle. (hdl=0x%x, error=%d)", handle, error);
        return nullptr;
    }

    // 多重保护，超级无敌360度托马斯回旋安全
    if (info->sink.get() != sink) // So, 什么情况下会发生这个问题呢？
    {
        // 从注册器获取的 sink ptr 与从 handlesys 获取的 sink prt 不匹配
        ctx->ReportError("The sink of sink handle registry does not match the sink of handlesys. (hdl={}, sinkReg={}, hdlSys={})", handle, info->sink.get(), sink);
        return nullptr;
    }

    return info->sink;
}

} // namespace sinks



/**
 * 将 cell_t 转换为 level
 * 如果 cell_t 超出 level 边界，将其修正为最近的边界值
 * 并返回 false
 */
bool CellToLevel(cell_t lvl, spdlog::level::level_enum &result)
{
    if (lvl < 0)
    {
        result = static_cast<spdlog::level::level_enum>(0);
        return false;
    }

    if (lvl >= spdlog::level::n_levels)
    {
        result = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
        return false;
    }

    result = static_cast<spdlog::level::level_enum>(lvl);
    return true;
}

/**
 * 将 cell_t 转换为 level
 * 如果 level 超出边界，输出一条警告信息
 * 并返回最近的边界值
 */
spdlog::level::level_enum CellToLevelOrLogWarn(IPluginContext *ctx, cell_t lvl)
{
    spdlog::level::level_enum result;
    if (!CellToLevel(lvl, result))
    {
        spdlog::log(log4sp::GetScriptedLoc(ctx), spdlog::level::warn, "Invliad level ({}), return '{}'.", lvl, spdlog::level::to_string_view(result));
    }
    return result;
}

/**
 * 返回 Scripted 调用 log4sp extension natives 的代码位置
 * 如果找不到，输出一条警告信息
 * 并返回空
 */
spdlog::source_loc GetScriptedLoc(IPluginContext *ctx)
{
    SourcePawn::IFrameIterator *frames = ctx->CreateFrameIterator();

    for (; !frames->Done(); frames->Next())
    {
        if (frames->IsScriptedFrame())
        {
            const char *file = frames->FilePath();
            const char *func = frames->FunctionName();
            int line = frames->LineNumber();
            ctx->DestroyFrameIterator(frames); // 千万不要忘记

            return spdlog::source_loc(file, line, func);
        }
    }

    ctx->DestroyFrameIterator(frames); // 千万不要忘记
    SPDLOG_WARN("Scripted source location not found.");
    return {};
}

// ref: sourcemod DebugReport::GetStackTrace
std::vector<std::string> GetStackTrace(IPluginContext *ctx)
{
    IFrameIterator *iter = ctx->CreateFrameIterator();

    std::vector<std::string> trace;
    iter->Reset();

    if (!iter->Done())
    {
        trace.push_back(" Call tack trace:");

        char temp[3072];
        const char *fn;

        for (int index = 0; !iter->Done(); iter->Next(), ++index)
        {
            fn = iter->FunctionName();
            if (!fn)
            {
                fn = "<unknown function>";
            }

            if (iter->IsNativeFrame())
            {
                g_pSM->Format(temp, sizeof(temp), "   [%d] %s", index, fn);
                trace.push_back(temp);
                continue;
            }

            if (iter->IsScriptedFrame())
            {
                const char *file = iter->FilePath();
                if (!file)
                {
                    file = "<unknown>";
                }
                g_pSM->Format(temp, sizeof(temp), "   [%d] Line %d, %s::%s", index, iter->LineNumber(), file, fn);
                trace.push_back(temp);
            }
        }
    }
    return trace;
}

/**
 * 按 sourcemod format 的风格格式化参数 (https://wiki.alliedmods.net/Format_Class_Functions_(SourceMod_Scripting))
 * 这个函数的目的主要是为了复用 buffer 以及方便后续改造 smutils->FormatString (TODO: 支持无限长度)
 */
char *FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param)
{
    static char buffer[2048];
    smutils->FormatString(buffer, sizeof(buffer), ctx, params, param);
    return buffer;
}

} // namespace log4sp
