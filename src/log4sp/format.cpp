#include <cassert>

#include "log4sp/format.h"
#include "am-float.h"


namespace log4sp {

namespace fmt_lib = spdlog::fmt_lib;
using spdlog::memory_buf_t;

/**
 * ref: https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.cpp
 */
#define LADJUST         0x00000001      /* left adjustment */
#define ZEROPAD         0x00000002      /* zero (as opposed to blank) pad */
#define UPPERDIGITS     0x00000004      /* make alpha digits uppercase */
#define TO_DIGIT(c)     ((c) - '0')
#define IS_DIGIT(c)     (c >= '0' && c <= '9')
#define ENTREF_MASK     (1 << 31)       /* See: https://github.com/alliedmodders/sourcemod/blob/4afbf9d57328de327c504c4a184670d992ae1609/core/HalfLife2.h#L60 */

#define THROW_ERROR(fmt, ...) \
    throw_log4sp_ex(fmt_lib::format(fmt, __VA_ARGS__));

#define CHECK_ARGS(x)   \
    if ((arg+x) > args) \
        THROW_ERROR("String formatted incorrectly - parameter {} (total {})", arg, args);


[[nodiscard]] std::string format_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param) {
    assert(ctx && params);

    char *format;
    CTX_LOCAL_TO_STRING(params[param], &format);
    unsigned int lparam = param + 1;
    return fmt_lib::to_string(format_to_buffer(ctx, format, params, &lparam));
}


static void ReorderTranslationParams(const SourceMod::Translation *pTrans, cell_t *params) noexcept {
    cell_t new_params[MAX_TRANSLATE_PARAMS];
    for (unsigned int i = 0; i < pTrans->fmt_count; ++i) {
        new_params[i] = params[pTrans->fmt_order[i]];
    }
    memcpy(params, new_params, pTrans->fmt_count * sizeof(cell_t));
}

static memory_buf_t Translate(SourcePawn::IPluginContext *ctx, const char *key, cell_t target, const cell_t *params, unsigned int *arg) {
    unsigned int langid;
    SourceMod::Translation pTrans;
    SourceMod::IPhraseCollection *pPhrases = plsys->FindPluginByContext(ctx->GetContext())->GetPhrases();

try_serverlang:
    if (target == SOURCEMOD_SERVER_LANGUAGE) {
        langid = translator->GetServerLanguage();
    } else if ((target >= 1) && (target <= playerhelpers->GetMaxClients())) {
        langid = translator->GetClientLanguage(target);
    } else {
        THROW_ERROR("Translation failed: invalid client index {} (arg {})", target, *arg);
    }

    if (pPhrases->FindTranslation(key, langid, &pTrans) != Trans_Okay) {
        if (target != SOURCEMOD_SERVER_LANGUAGE && langid != translator->GetServerLanguage()) {
            target = SOURCEMOD_SERVER_LANGUAGE;
            goto try_serverlang;
        } else if (langid != SOURCEMOD_LANGUAGE_ENGLISH) {
            if (pPhrases->FindTranslation(key, SOURCEMOD_LANGUAGE_ENGLISH, &pTrans) != Trans_Okay) {
                THROW_ERROR("Language phrase \"{}\" not found (arg {})", key, *arg);
            }
        } else {
            THROW_ERROR("Language phrase \"{}\" not found (arg {})", key, *arg);
        }
    }

    unsigned int max_params = pTrans.fmt_count;

    if (max_params) {
        cell_t new_params[MAX_TRANSLATE_PARAMS];

        /* Check if we're going to over the limit */
        if ((*arg) + (max_params - 1) > static_cast<unsigned int>(params[0])) {
            THROW_ERROR("Translation string formatted incorrectly - missing at least {} parameters (arg {})", ((*arg + (max_params - 1)) - params[0]), *arg);
        }

        /**
         * If we need to re-order the parameters, do so with a temporary array.
         * Otherwise, we could run into trouble with continual formats, a la ShowActivity().
         */
        memcpy(new_params, params, sizeof(cell_t) * (params[0] + 1));
        ReorderTranslationParams(&pTrans, &new_params[*arg]);

        return format_to_buffer(ctx, pTrans.szPhrase, new_params, arg);
    }

    return format_to_buffer(ctx, pTrans.szPhrase, params, arg);
}

static void AddString(memory_buf_t &out, const char *string, unsigned int width, int prec, int flags) noexcept {
    if (string == nullptr) {
        AddString(out, "(null)", width, prec, flags);
        return;
    }

    unsigned int size = std::strlen(string);
    if (prec >= 0 && static_cast<unsigned int>(prec) < size) {
        size = static_cast<unsigned int>(prec);
    }

    // 需要填充的字符数
    unsigned int pads = (width <= size) ? (0u) : (width - size);

    // right justify if required
    if (!(flags & LADJUST)) {
        while (pads) {
            pads--;
            out.push_back(' ');
        }
    }

    out.append(string, string + size);

    // left justify if required
    if (flags & LADJUST) {
        while (pads) {
            pads--;
            out.push_back(' ');
        }
    }
}

static void AddFloat(memory_buf_t &out, double fval, unsigned int width, int prec, int flags) noexcept {
    int digits;                 // non-fraction part digits
    double tmp;                 // temporary
    int val;                    // temporary
    bool sign{false};           // false: positive, true: negative
    unsigned int fieldlength;   // for padding
    int significant_digits{0};  // number of significant digits written
    constexpr const int MAX_SIGNIFICANT_DIGITS{16};

    if (ke::IsNaN(static_cast<float>(fval))) {
        AddString(out, "NaN", width, prec, flags);
        return;
    }

    if (ke::IsInfinite(static_cast<float>(fval))) {
        AddString(out, "Inf", width, prec, flags);
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
    digits = (int)std::log10(fval) + 1;

    // Only print 0.something if 0 < fval < 1
    if (digits < 1) {
        digits = 1;
    }

    // compute the field length
    fieldlength = digits + prec + ((prec > 0) ? 1 : 0) + (sign ? 1 : 0);

    // minus sign BEFORE left padding if padding with zeros
    if (sign && (flags & ZEROPAD)) {
        out.push_back('-');
    }

    // right justify if required
    if (!(flags & LADJUST)) {
        while (fieldlength < width--) {
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
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

static void AddBinary(memory_buf_t &out, unsigned int val, unsigned int width, int flags) noexcept {
    constexpr const int MAX_BINARY = 32;// FIXME: 如果 sourcemod 支持了 64 位 cell_t, 则此处需要修复
    int iter = MAX_BINARY - 1;          // 从字符串末尾向前遍历, 以保证添加到输出时为正序
    char text[MAX_BINARY];              // 值的二进制字符串

    do {
        text[iter--] = (val & 1) ? '1' : '0';
    } while (val >>= 1);

    const char *begin = text + iter + 1;
    unsigned int digits = MAX_BINARY - iter - 1;

    // 需要填充的字符数
    unsigned int pads = (width <= digits) ? (0u) : (width - digits);

    // right justify if required
    if (!(flags & LADJUST)) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    out.append(begin, text + MAX_BINARY);

    // left justify if required
    if (flags & LADJUST) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

static void AddUInt(memory_buf_t &out, unsigned int val, unsigned int width, int flags) noexcept {
    char text[10];
    unsigned int digits{0};
    do {
        text[digits++] = '0' + val % 10;
    } while (val /= 10);

    // 需要填充的字符数
    unsigned int pads = (width <= digits) ? (0u) : (width - digits);

    // right justify if required
    if (!(flags & LADJUST)) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    while (digits) {
        out.push_back(text[--digits]);
    }

    // left justify if required
    if (flags & LADJUST) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

static void AddInt(memory_buf_t &out, int val, unsigned int width, int flags) noexcept {
    char text[10];
    unsigned int digits{0};

    bool negative = val < 0;
    unsigned int unsignedVal = negative ? std::abs(val) : val;

    do {
        text[digits++] = '0' + unsignedVal % 10;
    } while (unsignedVal /= 10);

    // 需要填充的字符数
    unsigned int pads = (width <= digits) ? (0u) : (width - digits);
    if (pads > 0 && negative) {
        pads--;
    }

    // minus sign BEFORE left padding if padding with zeros
    if (negative && (flags & ZEROPAD)) {
        out.push_back('-');
    }

    // right justify if required
    if (!(flags & LADJUST)) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    // minus sign AFTER left padding if padding with spaces
    if (negative && !(flags & ZEROPAD)) {
        out.push_back('-');
    }

    while (digits) {
        out.push_back(text[--digits]);
    }

    // left justify if required
    if (flags & LADJUST) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

static void AddHex(memory_buf_t &out, unsigned int val, unsigned int width, int flags) noexcept {
    constexpr const char *hexUpper{"0123456789ABCDEF"};
    constexpr const char *hexlower{"0123456789abcdef"};
    const char *hexAdjust = (flags & UPPERDIGITS) ? hexUpper : hexlower;
    char text[8];
    unsigned int digits{0};

    do {
        text[digits++] = hexAdjust[val & 0xF];
    } while(val >>= 4);

    // 需要填充的字符数
    unsigned int pads = (width <= digits) ? (0u) : (width - digits);

    // right justify if required
    if (!(flags & LADJUST)) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    while (digits) {
        out.push_back(text[--digits]);
    }

    // left justify if required
    if (flags & LADJUST) {
        while (pads) {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

static bool DescribePlayer(int entRef, const char **namep, const char **authp, int *useridp) noexcept {
    int index = entRef;
    if (entRef & ENTREF_MASK) {
        index = gamehelpers->ReferenceToIndex(entRef);
    }

    SourceMod::IGamePlayer *player = playerhelpers->GetGamePlayer(index);
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

[[nodiscard]] memory_buf_t format_to_buffer(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, unsigned int *param) {
    assert(ctx && format && params && *param > 0 && *param <= SP_MAX_EXEC_PARAMS);

    memory_buf_t out;

    unsigned int args = params[0];  // params count
    unsigned int arg  = *param;     // 用于遍历 params 的指针
    const char *fmt = format;       // 用于遍历 format 的指针
    int flags;                      // 对齐 (左 / 右) | 填充符 ('0' / ' ')
    int prec;                       // 精度
    unsigned int width;             // 宽度

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
                while (IS_DIGIT(ch)) {
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
                unsigned int n{0};
                do {
                    n = 10 * n + (ch - '0');
                    ch = *fmt++;
                } while(IS_DIGIT(ch));
                width = n;
                goto reswitch;
            }
        case 'c': {
                CHECK_ARGS(0);
                char *c;
                CTX_LOCAL_TO_STRING(params[arg], &c);

                out.push_back(*c);
                ++arg;
                break;
            }
        case 'b': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddBinary(out, *value, width, flags);
                ++arg;
                break;
            }
        case 'd':
        case 'i': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddInt(out, static_cast<int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'u': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddUInt(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'f': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddFloat(out, sp_ctof(*value), width, prec, flags);
                ++arg;
                break;
            }
        case 'L': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                if (*value) {
                    const char *name;
                    const char *auth;
                    int userid;
                    if (!DescribePlayer(*value, &name, &auth, &userid))
                        THROW_ERROR("Client index {} is invalid (arg {})", *value, arg);

                    AddString(out, fmt_lib::format("{}<{}><{}><>", name, userid, auth).c_str(), width, prec, flags);
                } else {
                    AddString(out, "Console<0><Console><Console>", width, prec, flags);
                }
                ++arg;
                break;
            }
        case 'N': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                if (*value) {
                    const char *name;
                    if (!DescribePlayer(*value, &name, nullptr, nullptr))
                        THROW_ERROR("Client index {} is invalid (arg {})", *value, arg);

                    AddString(out, name, width, prec, flags);
                } else {
                    AddString(out, "Console", width, prec, flags);
                }
                ++arg;
                break;
            }
        case 's': {
                CHECK_ARGS(0);
                char *str;
                CTX_LOCAL_TO_STRING(params[arg], &str);

                AddString(out, str, width, prec, flags);
                ++arg;
                break;
            }
        case 'T': {
                CHECK_ARGS(1);
                char *key;
                cell_t *target;
                CTX_LOCAL_TO_STRING(params[arg++], &key);
                CTX_LOCAL_TO_PHYS_ADDR(params[arg++], &target);

                memory_buf_t phrase = Translate(ctx, key, *target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 't': {
                CHECK_ARGS(0);
                char *key;
                CTX_LOCAL_TO_STRING(params[arg++], &key);
                auto target = static_cast<cell_t>(translator->GetGlobalTarget());

                memory_buf_t phrase = Translate(ctx, key, target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 'X': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddHex(out, static_cast<unsigned int>(*value), width, flags | UPPERDIGITS);
                ++arg;
                break;
            }
        case 'x': {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

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
