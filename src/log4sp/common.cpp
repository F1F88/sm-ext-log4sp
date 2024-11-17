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
    SPDLOG_TRACE("Scripted source location not found.");
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


std::string FormatToAmxTplString(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param)
{
    char *format;
    ctx->LocalToString(params[param], &format);

    int lparam = ++param;

    auto out = FormatParams(format, ctx, params, &lparam);
    return fmt::to_string(out);
}


/**
 * ref: https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.cpp
 */
#define LADJUST         0x00000001      /* left adjustment */
#define ZEROPAD         0x00000002      /* zero (as opposed to blank) pad */
#define UPPERDIGITS     0x00000004      /* make alpha digits uppercase */

inline void ReorderTranslationParams(const Translation *pTrans, cell_t *params)
{
    cell_t new_params[MAX_TRANSLATE_PARAMS];
    for (unsigned int i = 0; i < pTrans->fmt_count; i++)
    {
        new_params[i] = params[pTrans->fmt_order[i]];
    }
    memcpy(params, new_params, pTrans->fmt_count * sizeof(cell_t));
}

fmt::memory_buffer Translate(IPluginContext *ctx, const char *key, cell_t target, const cell_t *params, int *arg)
{
    unsigned int langid;
    Translation pTrans;
    IPlugin *pl = plsys->FindPluginByContext(ctx->GetContext());
    unsigned int max_params = 0;
    IPhraseCollection *pPhrases;

    pPhrases = pl->GetPhrases();

try_serverlang:
    if (target == SOURCEMOD_SERVER_LANGUAGE)
    {
        langid = translator->GetServerLanguage();
    }
    else if ((target >= 1) && (target <= playerhelpers->GetMaxClients()))
    {
        langid = translator->GetClientLanguage(target);
    }
    else
    {
        throw std::runtime_error(
            fmt::format("Translation failed: invalid client index {} (arg {})", target, *arg));
    }

    if (pPhrases->FindTranslation(key, langid, &pTrans) != Trans_Okay)
    {
        if (target != SOURCEMOD_SERVER_LANGUAGE && langid != translator->GetServerLanguage())
        {
            target = SOURCEMOD_SERVER_LANGUAGE;
            goto try_serverlang;
        }
        else if (langid != SOURCEMOD_LANGUAGE_ENGLISH)
        {
            if (pPhrases->FindTranslation(key, SOURCEMOD_LANGUAGE_ENGLISH, &pTrans) != Trans_Okay)
            {
                throw std::runtime_error(
                    fmt::format("Language phrase \"{}\" not found (arg {})", key, *arg));
            }
        }
        else
        {
            throw std::runtime_error(
                fmt::format("Language phrase \"{}\" not found (arg {})", key, *arg));
        }
    }

    max_params = pTrans.fmt_count;

    if (max_params)
    {
        cell_t new_params[MAX_TRANSLATE_PARAMS];

        /* Check if we're going to over the limit */
        if ((*arg) + (max_params - 1) > (size_t)params[0])
        {
            throw std::runtime_error(
                fmt::format(
                    "Translation string formatted incorrectly - missing at least {} parameters (arg {})",
                    ((*arg + (max_params - 1)) - params[0]),
                    *arg));
        }

        /**
         * If we need to re-order the parameters, do so with a temporary array.
         * Otherwise, we could run into trouble with continual formats, a la ShowActivity().
         */
        memcpy(new_params, params, sizeof(cell_t) * (params[0] + 1));
        ReorderTranslationParams(&pTrans, &new_params[*arg]);

        // return FormatParams(pTrans.szPhrase, ctx, new_params, arg);
        return FormatParams(pTrans.szPhrase, ctx, new_params, arg);
    }
    else
    {
        // return FormatParams(pTrans.szPhrase, ctx, params, arg);
        return FormatParams(pTrans.szPhrase, ctx, params, arg);
    }
}

bool AddString(fmt::memory_buffer &out, const char *string, int width, int prec)
{
    int size = 0;
    static char nlstr[] = {'(','n','u','l','l',')','\0'};

    if (string == NULL)
    {
        string = nlstr;
        prec = -1;
    }

    if (prec >= 0)
    {
        for (size = 0; size < prec; ++size)
        {
            if (string[size] == '\0')
            {
                break;
            }
        }
    }
    else
    {
        while (string[size++]);
        size--;
    }

    width -= size;

    while (size--)
    {
        out.push_back(*string++);
    }

    while (width-- > 0)
    {
        out.push_back(' ');
    }

    return true;
}

void AddFloat(fmt::memory_buffer &out, double fval, int width, int prec, int flags)
{
    int digits;                 // non-fraction part digits
    double tmp;                 // temporary
    int val;                    // temporary
    int sign = 0;               // 0: positive, 1: negative
    int fieldlength;            // for padding
    int significant_digits = 0; // number of significant digits written
    const int MAX_SIGNIFICANT_DIGITS = 16;

    if (std::isnan(fval))
    {
        AddString(out, "NaN", width, prec);
        return;
    }

    // default precision
    if (prec < 0)
    {
        prec = 6;
    }

    // get the sign
    if (fval < 0)
    {
        fval = -fval;
        sign = 1;
    }

    // compute whole-part digits count
    digits = (int)std::log10(fval) + 1;

    // Only print 0.something if 0 < fval < 1
    if (digits < 1)
    {
        digits = 1;
    }

    // compute the field length
    fieldlength = digits + prec + ((prec > 0) ? 1 : 0) + sign;

    // minus sign BEFORE left padding if padding with zeros
    if (sign && (flags & ZEROPAD))
    {
        out.push_back('-');
    }

    // right justify if required
    if ((flags & LADJUST) == 0)
    {
        if (flags & ZEROPAD)
        {
            while ((fieldlength < width))
            {
                out.push_back('0');
                width--;
            }
        }
        else
        {
            while ((fieldlength < width))
            {
                out.push_back(' ');
                width--;
            }
        }
    }

    // minus sign AFTER left padding if padding with spaces
    if (sign && !(flags & ZEROPAD))
    {
        out.push_back('-');
    }

    // write the whole part
    tmp = std::pow(10.0, digits - 1);
    if (++significant_digits > MAX_SIGNIFICANT_DIGITS)
    {
        while ((digits--))
        {
            out.push_back('0');
        }
    }
    else
    {
        while ((digits--))
        {
            val = (int)(fval / tmp);
            out.push_back('0' + val);
            fval -= val * tmp;
            tmp *= 0.1;
        }
    }

    // write the fraction part
    if (prec)
    {
        out.push_back('.');
    }

    tmp = std::pow(10.0, prec);

    fval *= tmp;
    if (++significant_digits > MAX_SIGNIFICANT_DIGITS)
    {
        while (prec--)
        {
            out.push_back('0');
        }
    }
    else
    {
        while (prec--)
        {
            tmp *= 0.1;
            val = (int)(fval / tmp);
            out.push_back('0' + val);
            fval -= val * tmp;
        }
    }

    // left justify if required
    if (flags & LADJUST)
    {
        while ((fieldlength < width))
        {
            // right-padding only with spaces, ZEROPAD is ignored
            out.push_back(' ');
            width--;
        }
    }
}

void AddBinary(fmt::memory_buffer &out, unsigned int val, int width, int flags)
{
    char text[32];
    int digits;

    digits = 0;
    do
    {
        text[digits++] = (val & 1) ? '1' : '0';
        val >>= 1;
    } while (val);

    if (!(flags & LADJUST))
    {
        if (flags & ZEROPAD)
        {
            while (digits < width)
            {
                out.push_back('0');
                width--;
            }
        }
        else
        {
            while (digits < width)
            {
                out.push_back(' ');
                width--;
            }
        }
    }

    width -= digits;
    while (digits--)
    {
        out.push_back(text[digits]);
    }

    if (flags & LADJUST)
    {
        if (flags & ZEROPAD)
        {
            while (width-- > 0)
            {
                out.push_back('0');
            }
        }
        else
        {
            while (width-- > 0)
            {
                out.push_back(' ');
            }
        }
    }
}

void AddUInt(fmt::memory_buffer &out, unsigned int val, int width, int flags)
{
    char text[32];
    int digits;

    digits = 0;
    do
    {
        text[digits++] = '0' + val % 10;
        val /= 10;
    } while (val);

    if (!(flags & LADJUST))
    {
        if (flags & ZEROPAD)
        {
            while (digits < width)
            {
                out.push_back('0');
                width--;
            }
        }
        else
        {
            while ((digits < width))
            {
                out.push_back(' ');
                width--;
            }
        }
    }

    width -= digits;
    while (digits--)
    {
        out.push_back(text[digits]);
    }

    if (flags & LADJUST)
    {
        if (flags & ZEROPAD)
        {
            while (width-- > 0)
            {
                out.push_back('0');
            }
        }
        else
        {
            while (width-- > 0)
            {
                out.push_back(' ');
            }
        }
    }
}

void AddInt(fmt::memory_buffer &out, int val, int width, int flags)
{
    char text[32];
    int digits;
    int signedVal;
    unsigned int unsignedVal;

    digits = 0;
    signedVal = val;
    if (val < 0)
    {
        /* we want the unsigned version */
        unsignedVal = abs(val);
    }
    else
    {
        unsignedVal = val;
    }

    do
    {
        text[digits++] = '0' + unsignedVal % 10;
        unsignedVal /= 10;
    } while (unsignedVal);

    if (signedVal < 0)
    {
        text[digits++] = '-';
    }

    if (!(flags & LADJUST))
    {
        if (flags & ZEROPAD)
        {
            while ((digits < width))
            {
                out.push_back('0');
                width--;
            }
        }
        else
        {
            while ((digits < width))
            {
                out.push_back(' ');
                width--;
            }
        }
    }

    width -= digits;
    while (digits--)
    {
        out.push_back(text[digits]);
    }

    if (flags & LADJUST)
    {
        if (flags & ZEROPAD)
        {
            while (width-- > 0)
            {
                out.push_back('0');
            }
        }
        else
        {
            while (width-- > 0)
            {
                out.push_back(' ');
            }
        }
    }
}

void AddHex(fmt::memory_buffer &out, unsigned int val, int width, int flags)
{
    static char hexAdjustUppercase[] = "0123456789ABCDEF";
    static char hexAdjustLowercase[] = "0123456789abcdef";
    char text[32];
    int digits;
    const char *hexadjust;

    hexadjust = (flags & UPPERDIGITS) ? hexAdjustUppercase : hexAdjustLowercase;

    digits = 0;
    do
    {
        text[digits++] = hexadjust[val % 16];
        val /= 16;
    } while(val);

    if (!(flags & LADJUST))
    {
        if (flags & ZEROPAD)
        {
            while (digits < width)
            {
                out.push_back('0');
                width--;
            }
        }
        else
        {
            while (digits < width)
            {
                out.push_back(' ');
                width--;
            }
        }
    }

    width -= digits;
    while (digits--)
    {
        out.push_back(text[digits]);
    }

    if (flags & LADJUST)
    {
        if (flags & ZEROPAD)
        {
            while (width-- > 0)
            {
                out.push_back('0');
            }
        }
        else
        {
            while (width-- > 0)
            {
                out.push_back(' ');
            }
        }
    }
}

bool DescribePlayer(int index, const char **namep, const char **authp, int *useridp)
{
    auto player = playerhelpers->GetGamePlayer(index);
    if (!player || !player->IsConnected())
        return false;

    if (namep)
        *namep = player->GetName();
    if (authp) {
        const char *auth = player->GetAuthString();
        *authp = (auth && *auth) ? auth : "STEAM_ID_PENDING";
    }
    if (useridp)
        *useridp = player->GetUserId();
    return true;
}

fmt::memory_buffer FormatParams(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param)
{
    int arg;                // 用于遍历 params 的指针
    int args = params[0];   // param count
    auto out = fmt::memory_buffer();
    char ch;
    int flags;
    int width;
    int prec;
    int n;
    char sign;
    const char *fmt;

    arg = *param;
    fmt = format;

    while (true)
    {
        // run through the format string until we hit a '%' or '\0'
        for (ch = *fmt; ((ch = *fmt) != '%') && (ch != '\0'); ++fmt)
        {
            out.push_back(ch);
        }

        if (ch == '\0')
        {
            return out;
        }

        // skip over the '%'
        ++fmt;

        // reset formatting state
        flags = 0;
        width = 0;
        prec = -1;

rflag:
		ch = *fmt++;
reswitch:
        switch(ch)
        {
        case '-':
            {
                flags |= LADJUST;
                goto rflag;
            }
        case '!':
            {
                goto rflag;
            }
        case '.':
            {
                n = 0;
                while(std::isdigit((ch = *fmt++)))
                {
                    n = 10 * n + (ch - '0');
                }
                prec = (n < 0) ? -1 : n;
                goto reswitch;
            }
        case '0':
            {
                flags |= ZEROPAD;
                goto rflag;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                n = 0;
                do
                {
                    n = 10 * n + (ch - '0');
                    ch = *fmt++;
                } while(std::isdigit(ch));
                width = n;
                goto reswitch;
            }
        case 'c':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *c;
                ctx->LocalToString(params[arg], &c);
                out.push_back(*c);
                ++arg;
                break;
            }
        case 'b':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddBinary(out, *value, width, flags);
                ++arg;
                break;
            }
        case 'd':
        case 'i':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddInt(out, static_cast<int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'u':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddUInt(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'f':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddFloat(out, sp_ctof(*value), width, prec, flags);
                ++arg;
                break;
            }
        case 'L':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                char buffer[255];
                if (*value)
                {
                    const char *name;
                    const char *auth;
                    int userid;
                    if (!DescribePlayer(*value, &name, &auth, &userid))
                        throw std::runtime_error(
                            fmt::format("Client index {} is invalid (arg {})", static_cast<int>(*value), arg));

                    ke::SafeSprintf(buffer, sizeof(buffer), "%s<%d><%s><>", name, userid, auth);
                }
                else
                {
                    ke::SafeStrcpy(buffer, sizeof(buffer), "Console<0><Console><Console>");
                }
                AddString(out, buffer, width, prec);
                ++arg;
                break;
            }
        case 'N':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);

                const char *name = "Console";
                if (*value) {
                    if (!DescribePlayer(*value, &name, nullptr, nullptr))
                        throw std::runtime_error(
                            fmt::format("Client index {} is invalid (arg {})", static_cast<int>(*value), arg));
                }
                AddString(out, name, width, prec);
                ++arg;
                break;
            }
        case 's':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *str;
                ctx->LocalToString(params[arg], &str);
                AddString(out, str, width, prec);
                ++arg;
                break;
            }
        case 'T':
            {
                if (arg + 1 > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                cell_t *target;
                ctx->LocalToString(params[arg++], &key);
                ctx->LocalToPhysAddr(params[arg++], &target);
                out.append(Translate(ctx, key, *target, params, &arg));
                break;
            }
        case 't':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                cell_t target = translator->GetGlobalTarget();
                ctx->LocalToString(params[arg++], &key);
                out.append(Translate(ctx, key, target, params, &arg));
                break;
            }
        case 'X':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                flags |= UPPERDIGITS;
                AddHex(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'x':
            {
                if (arg > args) {
                    throw std::runtime_error(
                        fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddHex(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case '%':
            {
                out.push_back(ch);
                break;
            }
        case '\0':
            {
                out.push_back('%');
                return out;
            }
        default:
            {
                out.push_back(ch);
                break;
            }
        }
    }
    return out;
}

} // namespace log4sp
