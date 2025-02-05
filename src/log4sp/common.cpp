#include <cassert>

#include "log4sp/common.h"

namespace log4sp {

[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno) {
    spdlog::throw_spdlog_ex(msg, last_errno);
}

[[noreturn]] void throw_log4sp_ex(std::string msg) {
    spdlog::throw_spdlog_ex(std::move(msg));
}

[[nodiscard]] source_loc source_loc::from_plugin_ctx(SourcePawn::IPluginContext *ctx) noexcept {
    assert(ctx);

    uint32_t line{0};
    const char *file{nullptr};
    const char *func{nullptr};

    auto iter = ctx->CreateFrameIterator();
    do {
        if (iter->IsScriptedFrame()) {
            line = static_cast<uint32_t>(iter->LineNumber());
            file = iter->FilePath();
            func = iter->FunctionName();
            break;
        }
        iter->Next();
    } while (!iter->Done());
    ctx->DestroyFrameIterator(iter);

    return source_loc{file, line, func};
}

[[nodiscard]] std::vector<std::string> get_plugin_ctx_stack_trace(SourcePawn::IPluginContext *ctx) noexcept {
    assert(ctx);

    auto iter = ctx->CreateFrameIterator();
    if (iter->Done()) {
        ctx->DestroyFrameIterator(iter);
        return {};
    }

    std::vector<std::string> trace{"Call tack trace:"};

    for (int index = 0; !iter->Done(); iter->Next(), ++index) {
        if (iter->IsNativeFrame()) {
            auto func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            trace.emplace_back(fmt_lib::format("  [{}] {}", index, func));
        } else if (iter->IsScriptedFrame()) {
            auto func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            auto file = iter->FilePath();
            if (!file) {
                func = "<unknown>";
            }

            trace.emplace_back(fmt_lib::format("  [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
}


[[nodiscard]] std::string format_cell_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const uint32_t param) {
    assert(ctx && params);

    char *format;
    ctx->LocalToString(params[param], &format);
    uint32_t lparam{param + 1};
    return fmt_lib::to_string(format_cell_to_mem_buf(ctx, format, params, &lparam));
}


/**
 * ref: https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.cpp
 */
#define LADJUST         0x00000001      /* left adjustment */
#define ZEROPAD         0x00000002      /* zero (as opposed to blank) pad */
#define UPPERDIGITS     0x00000004      /* make alpha digits uppercase */

static void ReorderTranslationParams(const Translation *pTrans, cell_t *params) noexcept {
    cell_t new_params[MAX_TRANSLATE_PARAMS];
    for (uint32_t i = 0; i < pTrans->fmt_count; i++) {
        new_params[i] = params[pTrans->fmt_order[i]];
    }
    memcpy(params, new_params, pTrans->fmt_count * sizeof(cell_t));
}

static memory_buf_t Translate(SourcePawn::IPluginContext *ctx, const char *key, cell_t target, const cell_t *params, uint32_t *arg) {
    uint32_t langid;
    Translation pTrans;
    IPhraseCollection *pPhrases = plsys->FindPluginByContext(ctx->GetContext())->GetPhrases();

try_serverlang:
    if (target == SOURCEMOD_SERVER_LANGUAGE) {
        langid = translator->GetServerLanguage();
    } else if ((target >= 1) && (target <= playerhelpers->GetMaxClients())) {
        langid = translator->GetClientLanguage(target);
    } else {
        throw_log4sp_ex(fmt_lib::format("Translation failed: invalid client index {} (arg {})", target, *arg));
    }

    if (pPhrases->FindTranslation(key, langid, &pTrans) != Trans_Okay) {
        if (target != SOURCEMOD_SERVER_LANGUAGE && langid != translator->GetServerLanguage()) {
            target = SOURCEMOD_SERVER_LANGUAGE;
            goto try_serverlang;
        } else if (langid != SOURCEMOD_LANGUAGE_ENGLISH) {
            if (pPhrases->FindTranslation(key, SOURCEMOD_LANGUAGE_ENGLISH, &pTrans) != Trans_Okay) {
                throw_log4sp_ex(fmt_lib::format("Language phrase \"{}\" not found (arg {})", key, *arg));
            }
        } else {
            throw_log4sp_ex(fmt_lib::format("Language phrase \"{}\" not found (arg {})", key, *arg));
        }
    }

    uint32_t max_params{pTrans.fmt_count};

    if (max_params) {
        cell_t new_params[MAX_TRANSLATE_PARAMS];

        /* Check if we're going to over the limit */
        if ((*arg) + (max_params - 1) > static_cast<uint32_t>(params[0])) {
            throw_log4sp_ex(fmt_lib::format("Translation string formatted incorrectly - missing at least {} parameters (arg {})", ((*arg + (max_params - 1)) - params[0]), *arg));
        }

        /**
         * If we need to re-order the parameters, do so with a temporary array.
         * Otherwise, we could run into trouble with continual formats, a la ShowActivity().
         */
        memcpy(new_params, params, sizeof(cell_t) * (params[0] + 1));
        ReorderTranslationParams(&pTrans, &new_params[*arg]);

        return format_cell_to_mem_buf(ctx, pTrans.szPhrase, new_params, arg);
    }

    return format_cell_to_mem_buf(ctx, pTrans.szPhrase, params, arg);
}

static void AddString(memory_buf_t &out, const char *string, uint32_t width, int prec, int flags) noexcept {
    if (string == nullptr) {
        constexpr const char *nlstr{"(null)"};
        constexpr const uint32_t size{sizeof(nlstr)};

        if (!(flags & LADJUST)) {
            while (size < width--) {
                out.push_back(' ');
            }

            out.append(nlstr, nlstr + size);
        } else {
            out.append(nlstr, nlstr + size);

            while (size < width--) {
                out.push_back(' ');
            }
        }
    } else {
        uint32_t size{strlen(string)};
        if (prec >= 0 && static_cast<uint32_t>(prec) < size) {
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

static void AddFloat(memory_buf_t &out, double fval, uint32_t width, int prec, int flags) noexcept {
    int digits;                 // non-fraction part digits
    double tmp;                 // temporary
    int val;                    // temporary
    bool sign{false};           // false: positive, true: negative
    uint32_t fieldlength;       // for padding
    int significant_digits{0};  // number of significant digits written
    const int MAX_SIGNIFICANT_DIGITS{16};

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
        sign = true;
    }

    // compute whole-part digits count
    // Only print 0.something if 0 < fval < 1
    digits = std::max((int)std::log10(fval) + 1, 1);

    // compute the field length
    fieldlength = digits + prec + (prec > 0 ? 1 : 0) + (sign ? 1 : 0);

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
            out.push_back('0' + static_cast<char>(val));
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
            out.push_back('0' + static_cast<char>(val));
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

static void AddBinary(memory_buf_t &out, uint32_t val, uint32_t width, int flags) noexcept {
    char text[32];

    int iter{31};
    do {
        text[iter--] = (val & 1) ? '1' : '0';
    } while (val >>= 1);

    const char *begin = text + iter + 1;
    uint32_t digits = 31 - iter;

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

static void AddUInt(memory_buf_t &out, uint32_t val, uint32_t width, int flags) noexcept {
    char text[10];
    uint32_t digits{0};
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

static void AddInt(memory_buf_t &out, int val, uint32_t width, int flags) noexcept {
    char text[10];
    uint32_t digits{0};

    bool negative = val < 0;
    uint32_t unsignedVal = negative ? abs(val) : val;

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

static void AddHex(memory_buf_t &out, uint32_t val, uint32_t width, int flags) noexcept {
    constexpr const char *hexUpper{"0123456789ABCDEF"};
    constexpr const char *hexlower{"0123456789abcdef"};
    const char *hexAdjust = (flags & UPPERDIGITS) ? hexUpper : hexlower;
    char text[8];
    uint32_t digits{0};

    do {
        text[digits++] = hexAdjust[val & 0xF];
    } while(val >>= 4);

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

static bool DescribePlayer(int index, const char **namep, const char **authp, int *useridp) noexcept {
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

[[nodiscard]] memory_buf_t format_cell_to_mem_buf(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, uint32_t *param) {
    assert(ctx && format && params && *param > 0);

    memory_buf_t out;

    uint32_t args = params[0];  // params count
    uint32_t arg  = *param;     // 用于遍历 params 的指针
    const char *fmt = format;   // 用于遍历 format 的指针
    int flags;                  // 对齐 (左 / 右) | 填充符 ('0' / ' ')
    int prec;                   // 精度
    uint32_t width;             // 宽度

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
                int n{0};
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
                uint32_t n{0};
                do {
                    n = 10 * n + (ch - '0');
                    ch = *fmt++;
                } while(ch >= '0' && ch <= '9');
                width = n;
                goto reswitch;
            }
        case 'c': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *c;
                ctx->LocalToString(params[arg], &c);
                out.push_back(*c);
                ++arg;
                break;
            }
        case 'b': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
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
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddInt(out, static_cast<int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'u': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddUInt(out, static_cast<uint32_t>(*value), width, flags);
                ++arg;
                break;
            }
        case 'f': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddFloat(out, sp_ctof(*value), width, prec, flags);
                ++arg;
                break;
            }
        case 'L': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);

                if (*value) {
                    const char *name;
                    const char *auth;
                    int userid;
                    if (!DescribePlayer(*value, &name, &auth, &userid)) {
                        throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                    }

                    AddString(out, fmt_lib::format("{}<{}><{}><>", name, userid, auth).c_str(), width, prec, flags);
                } else {
                    AddString(out, "Console<0><Console><Console>", width, prec, flags);
                }
                ++arg;
                break;
            }
        case 'N': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);

                if (*value) {
                    const char *name;
                    if (!DescribePlayer(*value, &name, nullptr, nullptr)) {
                        throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                    }

                    AddString(out, name, width, prec, flags);
                } else {
                    AddString(out, "Console", width, prec, flags);
                }
                ++arg;
                break;
            }
        case 's': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *str;
                ctx->LocalToString(params[arg], &str);
                AddString(out, str, width, prec, flags);
                ++arg;
                break;
            }
        case 'T': {
                if (arg + 1 > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                cell_t *target;
                ctx->LocalToString(params[arg++], &key);
                ctx->LocalToPhysAddr(params[arg++], &target);
                auto phrase{Translate(ctx, key, *target, params, &arg)};
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 't': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                ctx->LocalToString(params[arg++], &key);
                auto phrase{Translate(ctx, key, static_cast<cell_t>(translator->GetGlobalTarget()), params, &arg)};
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 'X': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddHex(out, static_cast<uint32_t>(*value), width, flags | UPPERDIGITS);
                ++arg;
                break;
            }
        case 'x': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddHex(out, static_cast<uint32_t>(*value), width, flags);
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
