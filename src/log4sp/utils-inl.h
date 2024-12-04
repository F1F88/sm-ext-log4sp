#ifndef _LOG4SP_UTILS_INL_H_
#define _LOG4SP_UTILS_INL_H_

#include "log4sp/utils.h"

namespace log4sp {


inline spdlog::level::level_enum cell_to_level(cell_t lvl) noexcept {
    if (lvl < 0) {
        return static_cast<spdlog::level::level_enum>(0);
    }

    if (lvl >= static_cast<cell_t>(spdlog::level::n_levels)) {
        return static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    return static_cast<spdlog::level::level_enum>(lvl);
}

inline spdlog::async_overflow_policy cell_to_policy(cell_t policy) noexcept {
    if (policy < 0) {
        return static_cast<spdlog::async_overflow_policy>(0);
    }

    if (policy >= 3) {
        return static_cast<spdlog::async_overflow_policy>(2);
    }

    return static_cast<spdlog::async_overflow_policy>(policy);
}

inline spdlog::pattern_time_type cell_to_pattern_time_type(cell_t type) noexcept {
    if (type < 0) {
        return static_cast<spdlog::pattern_time_type>(0);
    }

    if (type >= 2) {
        return static_cast<spdlog::pattern_time_type>(1);
    }

    return static_cast<spdlog::pattern_time_type>(type);
}

inline spdlog::source_loc get_plugin_source_loc(IPluginContext *ctx) {
    auto iter = ctx->CreateFrameIterator();
    spdlog::source_loc loc;

    for (; !iter->Done(); iter->Next()) {
        if (iter->IsScriptedFrame()) {
            loc.funcname = iter->FunctionName();
            loc.filename = iter->FilePath();
            loc.line = iter->LineNumber();
            break;
        }
    }

    ctx->DestroyFrameIterator(iter);
    return loc;
}

inline std::vector<std::string> get_stack_trace(IPluginContext *ctx) {
    auto iter = ctx->CreateFrameIterator();
    if (!iter->Done()) {
        ctx->DestroyFrameIterator(iter);
        return {};
    }

    std::vector<std::string> trace;
    trace.push_back(" Call tack trace:");

    const char *func;
    const char *file;

    for (int index = 0; !iter->Done(); iter->Next(), ++index) {
        if (iter->IsNativeFrame()) {
            func = iter->FunctionName();
            func = func != nullptr ? func : "<unknown function>";

            trace.push_back(fmt::format("   [{}] {}", index, func));
            continue;
        }

        if (iter->IsScriptedFrame()) {
            func = iter->FunctionName();
            func = func != nullptr ? func : "<unknown function>";

            file = iter->FilePath();
            file = file != nullptr ? file : "<unknown>";

            trace.push_back(fmt::format("   [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
}


inline std::string format_cell_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, unsigned int param) {
    char *format;
    ctx->LocalToString(params[param], &format);

    int lparam = ++param;

    auto out = format_cell_to_memory_buf(format, ctx, params, &lparam);
    return fmt::to_string(out);
}


/**
 * ref: https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.cpp
 */
#define LADJUST         0x00000001      /* left adjustment */
#define ZEROPAD         0x00000002      /* zero (as opposed to blank) pad */
#define UPPERDIGITS     0x00000004      /* make alpha digits uppercase */

inline static void ReorderTranslationParams(const Translation *pTrans, cell_t *params) {
    cell_t new_params[MAX_TRANSLATE_PARAMS];
    for (unsigned int i = 0; i < pTrans->fmt_count; i++) {
        new_params[i] = params[pTrans->fmt_order[i]];
    }
    memcpy(params, new_params, pTrans->fmt_count * sizeof(cell_t));
}

inline static fmt::memory_buffer Translate(IPluginContext *ctx, const char *key, cell_t target, const cell_t *params, int *arg) {
    unsigned int langid;
    Translation pTrans;
    IPlugin *pl = plsys->FindPluginByContext(ctx->GetContext());
    unsigned int max_params = 0;
    IPhraseCollection *pPhrases;

    pPhrases = pl->GetPhrases();

try_serverlang:
    if (target == SOURCEMOD_SERVER_LANGUAGE) {
        langid = translator->GetServerLanguage();
    } else if ((target >= 1) && (target <= playerhelpers->GetMaxClients())) {
        langid = translator->GetClientLanguage(target);
    } else {
        throw std::runtime_error(fmt::format("Translation failed: invalid client index {} (arg {})", target, *arg));
    }

    if (pPhrases->FindTranslation(key, langid, &pTrans) != Trans_Okay) {
        if (target != SOURCEMOD_SERVER_LANGUAGE && langid != translator->GetServerLanguage()) {
            target = SOURCEMOD_SERVER_LANGUAGE;
            goto try_serverlang;
        } else if (langid != SOURCEMOD_LANGUAGE_ENGLISH) {
            if (pPhrases->FindTranslation(key, SOURCEMOD_LANGUAGE_ENGLISH, &pTrans) != Trans_Okay) {
                throw std::runtime_error(fmt::format("Language phrase \"{}\" not found (arg {})", key, *arg));
            }
        } else {
            throw std::runtime_error(fmt::format("Language phrase \"{}\" not found (arg {})", key, *arg));
        }
    }

    max_params = pTrans.fmt_count;

    if (max_params) {
        cell_t new_params[MAX_TRANSLATE_PARAMS];

        /* Check if we're going to over the limit */
        if ((*arg) + (max_params - 1) > (size_t)params[0]) {
            throw std::runtime_error(fmt::format("Translation string formatted incorrectly - missing at least {} parameters (arg {})", ((*arg + (max_params - 1)) - params[0]), *arg));
        }

        /**
         * If we need to re-order the parameters, do so with a temporary array.
         * Otherwise, we could run into trouble with continual formats, a la ShowActivity().
         */
        memcpy(new_params, params, sizeof(cell_t) * (params[0] + 1));
        ReorderTranslationParams(&pTrans, &new_params[*arg]);

        return format_cell_to_memory_buf(pTrans.szPhrase, ctx, new_params, arg);
    }
    else {
        return format_cell_to_memory_buf(pTrans.szPhrase, ctx, params, arg);
    }
}

inline static void AddString(fmt::memory_buffer &out, const char *string, int width, int prec, int flags) {
    if (string == nullptr) {
        const char nlstr[] = {'(','n','u','l','l',')','\0'};
        const int size = sizeof(nlstr);

        if (!(flags & LADJUST)) {
            while (size < width--) {
                out.push_back(' ');
            }

            out.append(nlstr, nlstr + sizeof(nlstr));
        } else {
            out.append(nlstr, nlstr + sizeof(nlstr));

            while (size < width--) {
                out.push_back(' ');
            }
        }
    } else {
        int size = strlen(string);
        if (prec >= 0 && prec < size) {
            size = prec;
        }

        if (!(flags & LADJUST)) {
            while (size < width--) {
                out.push_back(' ');
            }

            out.append(string, string + size);
        } else {
            out.append(string, string + size);

            while (size < width--) {
                out.push_back(' ');
            }
        }
    }
}

inline static void AddFloat(fmt::memory_buffer &out, double fval, int width, int prec, int flags) {
    int digits;                 // non-fraction part digits
    double tmp;                 // temporary
    int val;                    // temporary
    int sign = 0;               // 0: positive, 1: negative
    int fieldlength;            // for padding
    int significant_digits = 0; // number of significant digits written
    const int MAX_SIGNIFICANT_DIGITS = 16;

    if (std::isnan(fval)) {
        AddString(out, "NaN", width, prec, flags);
        return;
    }

    // default precision
    if (prec < 0) {
        prec = 6;
    }

    // get the sign
    if (fval < 0) {
        fval = -fval;
        sign = 1;
    }

    // compute whole-part digits count
    digits = (int)std::log10(fval) + 1;

    // Only print 0.something if 0 < fval < 1
    if (digits < 1) {
        digits = 1;
    }

    // compute the field length
    fieldlength = digits + prec + ((prec > 0) ? 1 : 0) + sign;

    // minus sign BEFORE left padding if padding with zeros
    if (sign && (flags & ZEROPAD)) {
        out.push_back('-');
    }

    // right justify if required
    if ((flags & LADJUST) == 0) {
        if (flags & ZEROPAD) {
            while (fieldlength < width--) {
                out.push_back('0');
            }
        } else {
            while (fieldlength < width--) {
                out.push_back(' ');
            }
        }
    }

    // minus sign AFTER left padding if padding with spaces
    if (sign && !(flags & ZEROPAD)) {
        out.push_back('-');
    }

    // write the whole part
    tmp = std::pow(10.0, digits - 1);
    if (++significant_digits > MAX_SIGNIFICANT_DIGITS) {
        while (digits--) {
            out.push_back('0');
        }
    } else {
        while (digits--) {
            val = (int)(fval / tmp);
            out.push_back('0' + val);
            fval -= val * tmp;
            tmp *= 0.1;
        }
    }

    // write the fraction part
    if (prec) {
        out.push_back('.');
    }

    tmp = std::pow(10.0, prec);

    fval *= tmp;
    if (++significant_digits > MAX_SIGNIFICANT_DIGITS) {
        while (prec--) {
            out.push_back('0');
        }
    } else {
        while (prec--) {
            tmp *= 0.1;
            val = (int)(fval / tmp);
            out.push_back('0' + val);
            fval -= val * tmp;
        }
    }

    // left justify if required
    if (flags & LADJUST) {
        while (fieldlength < width--) {
            // right-padding only with spaces, ZEROPAD is ignored
            out.push_back(' ');
        }
    }
}

inline static void AddBinary(fmt::memory_buffer &out, unsigned int val, int width, int flags) {
    char text[32];

    int iter = 31;
    do {
        text[iter--] = (val & 1) ? '1' : '0';
    } while (val >>= 1);

    const char *begin = text + iter + 1;
    int digits = 31 - iter;

    if (!(flags & LADJUST)) {
        if (flags & ZEROPAD) {
            while (digits < width--) {
                out.push_back('0');
            }
        } else {
            while (digits < width--) {
                out.push_back(' ');
            }
        }

        out.append(begin, text + 32);
    } else {
        out.append(begin, text + 32);

        if (flags & ZEROPAD) {
            while (digits < width--) {
                out.push_back('0');
            }
        } else {
            while (digits < width--) {
                out.push_back(' ');
            }
        }
    }
}

inline static void AddUInt(fmt::memory_buffer &out, unsigned int val, int width, int flags) {
    char text[10];
    int digits = 0;
    do {
        text[digits++] = '0' + val % 10;
    } while (val /= 10);

    if (!(flags & LADJUST)) {
        if (flags & ZEROPAD) {
            while (digits < width--) {
                out.push_back('0');
            }
        } else {
            while (digits < width--) {
                out.push_back(' ');
            }
        }

        while (digits--) {
            out.push_back(text[digits]);
        }
    } else {
        width -= digits;
        while (digits--) {
            out.push_back(text[digits]);
        }

        if (flags & ZEROPAD) {
            while (width-- > 0) {
                out.push_back('0');
            }
        } else {
            while (width-- > 0) {
                out.push_back(' ');
            }
        }
    }
}

inline static void AddInt(fmt::memory_buffer &out, int val, int width, int flags) {
    char text[10];
    int digits = 0;

    bool negative = val < 0;
    unsigned int unsignedVal = negative ? abs(val) : val;

    do {
        text[digits++] = '0' + unsignedVal % 10;
    } while (unsignedVal /= 10);

    if (!(flags & LADJUST)) {
        if (flags & ZEROPAD) {
            if (negative) {
                out.push_back('-');
            }

            while (digits < width--) {
                out.push_back('0');
            }
        } else {
            while (digits < width--) {
                out.push_back(' ');
            }

            if (negative) {
                out.push_back('-');
            }
        }

        while (digits--) {
            out.push_back(text[digits]);
        }
    } else {
        if (negative) {
            out.push_back('-');
        }

        width -= digits;
        while (digits--) {
            out.push_back(text[digits]);
        }

        if (flags & ZEROPAD) {
            while (width-- > 0) {
                out.push_back('0');
            }
        } else {
            while (width-- > 0) {
                out.push_back(' ');
            }
        }
    }
}

inline static void AddHex(fmt::memory_buffer &out, unsigned int val, int width, int flags) {
    char text[8];
    int digits = 0;

    if (flags & UPPERDIGITS) {
        const char hexAdjust[] = "0123456789ABCDEF";
        do {
            text[digits++] = hexAdjust[val & 0xF];
        } while(val >>= 4);
    } else {
        const char hexAdjust[] = "0123456789abcdef";
        do {
            text[digits++] = hexAdjust[val & 0xF];
        } while(val >>= 4);
    }

    if (!(flags & LADJUST)) {
        if (flags & ZEROPAD) {
            while (digits < width--) {
                out.push_back('0');
            }
        } else {
            while (digits < width--) {
                out.push_back(' ');
            }
        }

        while (digits--) {
            out.push_back(text[digits]);
        }
    } else {
        width -= digits;
        while (digits--) {
            out.push_back(text[digits]);
        }

        if (flags & ZEROPAD) {
            while (width-- > 0) {
                out.push_back('0');
            }
        } else {
            while (width-- > 0) {
                out.push_back(' ');
            }
        }
    }
}

inline static bool DescribePlayer(int index, const char **namep, const char **authp, int *useridp) {
    auto player = playerhelpers->GetGamePlayer(index);
    if (!player || !player->IsConnected()) {
        return false;
    }

    if (namep != nullptr) {
        *namep = player->GetName();
    }

    if (authp != nullptr) {
        const char *auth = player->GetAuthString();
        *authp = (auth && *auth) ? auth : "STEAM_ID_PENDING";
    }

    if (useridp != nullptr) {
        *useridp = player->GetUserId();
    }

    return true;
}

inline fmt::memory_buffer format_cell_to_memory_buf(const char *format, SourcePawn::IPluginContext *ctx, const cell_t *params, int *param) {
    auto out = fmt::memory_buffer();

    int args = params[0];       // params count
    int arg  = *param;          // 用于遍历 params 的指针
    const char *fmt = format;   // 用于遍历 format 的指针
    int flags;                  // 对齐 (左 / 右) | 填充符 ('0' / ' ')
    int width;                  // 宽度
    int prec;                   // 精度

    while (true) {
        const char *begin = fmt;

        // run through the format string until we hit a '%' or '\0'
        while (*fmt != '%' && *fmt != '\0') {
            ++fmt;
        }

        out.append(begin, fmt);

        if (*fmt == '\0') {
            *param = arg;
            return out;
        }

        // skip over the '%'
        ++fmt;

        // reset formatting state
        flags = 0;
        width = 0;
        prec = -1;

rflag:
		char ch = *fmt++;
reswitch:
        switch(ch) {
        case '-': {
                flags |= LADJUST;
                goto rflag;
            }
        case '.': {
                int n = 0;
                ch = *fmt++;
                while (ch >= '0' && ch <= '9') {
                    n = 10 * n + (ch - '0');
                    ch = *fmt++;
                }
                prec = (n < 0) ? -1 : n;
                goto reswitch;
            }
        case '0': {
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
        case '9': {
                int n = 0;
                do {
                    n = 10 * n + (ch - '0');
                    ch = *fmt++;
                } while(ch >= '0' && ch <= '9');
                width = n;
                goto reswitch;
            }
        case 'c': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *c;
                ctx->LocalToString(params[arg], &c);
                out.push_back(*c);
                ++arg;
                break;
            }
        case 'b': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddBinary(out, *value, width, flags);
                ++arg;
                break;
            }
        case 'd':
        case 'i': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddInt(out, static_cast<int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'u': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddUInt(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'f': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddFloat(out, sp_ctof(*value), width, prec, flags);
                ++arg;
                break;
            }
        case 'L': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                char buffer[255];
                if (*value) {
                    const char *name;
                    const char *auth;
                    int userid;
                    if (!DescribePlayer(*value, &name, &auth, &userid)) {
                        throw std::runtime_error(fmt::format("Client index {} is invalid (arg {})", static_cast<int>(*value), arg));
                    }

                    ke::SafeSprintf(buffer, sizeof(buffer), "%s<%d><%s><>", name, userid, auth);
                } else {
                    ke::SafeStrcpy(buffer, sizeof(buffer), "Console<0><Console><Console>");
                }
                AddString(out, buffer, width, prec, flags);
                ++arg;
                break;
            }
        case 'N': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);

                const char *name = "Console";
                if (*value) {
                    if (!DescribePlayer(*value, &name, nullptr, nullptr)) {
                        throw std::runtime_error(fmt::format("Client index {} is invalid (arg {})", static_cast<int>(*value), arg));
                    }
                }
                AddString(out, name, width, prec, flags);
                ++arg;
                break;
            }
        case 's': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *str;
                ctx->LocalToString(params[arg], &str);
                AddString(out, str, width, prec, flags);
                ++arg;
                break;
            }
        case 'T': {
                if (arg + 1 > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                cell_t *target;
                ctx->LocalToString(params[arg++], &key);
                ctx->LocalToPhysAddr(params[arg++], &target);
                fmt::memory_buffer phrase = Translate(ctx, key, *target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 't': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                cell_t target = translator->GetGlobalTarget();
                ctx->LocalToString(params[arg++], &key);
                fmt::memory_buffer phrase = Translate(ctx, key, target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 'X': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                flags |= UPPERDIGITS;
                AddHex(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'x': {
                if (arg > args) {
                    throw std::runtime_error(fmt::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddHex(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case '%': {
                out.push_back(ch);
                break;
            }
        case '\0': {
                out.push_back('%');
                *param = arg;
                return out;
            }
        default: {
                out.push_back(ch);
                break;
            }
        }
    }

    *param = arg;
    return out;
}


}       // namespace log4sp
#endif  // _LOG4SP_UTILS_INL_H_
