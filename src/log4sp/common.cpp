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

        ctx->ReportError("Allocation of logger handle failed (error %d)", error);
        return BAD_HANDLE;
    }

    SPDLOG_TRACE("Create Logger handle({}) succeed (type={}, obj={}, name={}).", handle, g_LoggerHandleType, fmt::ptr(logger.get()), logger->name());
    return handle;
}

spdlog::logger *ReadHandleOrReportError(IPluginContext *ctx, Handle_t handle)
{
    spdlog::logger *logger;
    HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error = handlesys->ReadHandle(handle, g_LoggerHandleType, &sec, (void **)&logger);
    if (error != HandleError_None)
    {
        ctx->ReportError("Logger handle 0x%x is invalid (error %d).", handle, error);
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
        ctx->ReportError("Allocation of sink handle failed (error %d).", error);
        return BAD_HANDLE;
    }

    // 多重保护，超级无敌360度托马斯回旋安全
    if (SinkHandleRegistry::instance().hasKey(handle)) // So, 什么情况下会发生这个问题呢？
    {
        ctx->ReportError("Sink handel 0x%x already exists in the sink registry.", handle);
        return BAD_HANDLE;
    }

    log4sp::SinkHandleRegistry::instance().registerSink(handle, type, sink);
    SPDLOG_TRACE("Create Sink handle({}) succeed (type={}, obj={}).", handle, type, fmt::ptr(sink.get()));
    return handle;
}


spdlog::sink_ptr ReadHandleOrReportError(IPluginContext *ctx, Handle_t handle)
{
    SinkHandleInfo *info = SinkHandleRegistry::instance().get(handle);
    if (info == nullptr)
    {
        ctx->ReportError("Sink handle 0x%x not found in sink registry.", handle);
        return nullptr;
    }

    spdlog::sinks::sink *sink;
    HandleSecurity sec(ctx->GetIdentity(), myself->GetIdentity());
    HandleError error = handlesys->ReadHandle(handle, info->type, &sec, (void **)&sink);
    if (error != HandleError_None)
    {
        ctx->ReportError("Sink handle 0x%x is invalid (error %d).", handle, error);
        return nullptr;
    }

    // 多重保护，超级无敌360度托马斯回旋安全
    if (info->sink.get() != sink) // So, 什么情况下会发生这个问题呢？
    {
        ctx->ReportError("The read handle(0x%x) sink(0x%x) does not match the registered sink(0x%x).", handle, sink, info->sink.get());
        return nullptr;
    }

    return info->sink;
}

} // namespace sinks



// bool StringToInt(const char *str, int &result)
// {
//     bool flag;
//     try
//     {
//         result = std::stoi(str);
//         flag = true;
//     }
//     catch(const std::exception& e)
//     {
//         flag = false;
//     }
//     return flag;
// }

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
        auto loc = log4sp::GetScriptedLoc(ctx);
        auto lvlName = spdlog::level::to_string_view(result);
        spdlog::log(loc, spdlog::level::warn, "Level {} out of bounds, fix to {}.", lvl, lvlName);
    }
    return result;
}

/**
 * 将 path 路径分隔符全部替换为
 * 与 log4sp extension 编译的平台相同的分隔符
 * 返回替换后的字符串
 */
char *ReplacePathSep(const char *path)
{
    int len = strlen(path);
    char *result = new char[len + 1];

    // 与 spdlog 源码类似，Win、Linux 双平台分开处理
    if (sizeof(spdlog::details::os::folder_seps) == 2)
    {
        for (int i = 0; i < len; ++i)
        {
            result[i] = path[i] == '\\' ? '/' : path[i];
        }
    }
    else
    {
        for (int i = 0; i < len; ++i)
        {
            result[i] = path[i] == '/' ? '\\' : path[i];
        }
    }
    result[len] = '\0';
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
    for (int i = 1; !frames->Done(); ++i)
    {
        if (!frames->IsScriptedFrame() || !strncmp(frames->FilePath(), "log4sp", 6))
        {
            frames->Next(); // 当前栈帧不是插件调用的栈帧，我们需要继续向下查找
            // SPDLOG_TRACE("i={} | IsInternal={} | IsNative={} | IsScripted={} | file={} | func={} | line={}",
            //     ++i,
            //     frames->IsInternalFrame(),
            //     frames->IsNativeFrame(),
            //     frames->IsScriptedFrame(),
            //     frames->FilePath() == nullptr ? "null": frames->FilePath(),
            //     frames->FunctionName() == nullptr ? "null": frames->FunctionName(),
            //     frames->LineNumber()
            // );
        }
        else
        {
            break;          // 符合条件, 说明当前栈帧是用户调用 log4sp api 的 frame
        }
    }

    if (frames->Done())
    {
        ctx->DestroyFrameIterator(frames);
        spdlog::warn("Scripted source location not found, fix to empty.");
        // spdlog::dump_backtrace();
        return {};
    }

    const char *file = frames->FilePath();
    const char *func = frames->FunctionName();
    const int line = frames->LineNumber();
    ctx->DestroyFrameIterator(frames);

    /**
     * Bug：
     *      在 Pattern 中添加 %s 来显示文件名时，日志消息总是附带是完整的 path 而不是仅输出文件名
     * Description：
     *      如果 extension 编译时所用的操作系统和 plugin 编译时所用的操作系统不同
     *      则 pattern 的 %s 只能显示完整的路径名, 而不是只显示文件名
     * Reason：
     *      spdlog 中的 short_filename_formatter::basename 通过判断 spdlog::details::os::folder_seps
     *      来决定从 path 中截取文件名的方案，这导致 spdlog 无法识别其他操作系统的 path 中的分隔符
     * Slove:
     *      1. 传递 path 给 spdlog 前，将分隔符替换为 short_filename_formatter::basename 匹配的分隔符
     *      2. 更新 spdlog 为 v2.x 版本 (不采用！因为 v2.x 似乎不够完善, 并且活跃度较低)
     */
    return spdlog::source_loc(ReplacePathSep(file), line, func);
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
