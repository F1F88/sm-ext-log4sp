#include <cassert>
#include <type_traits>
#include <limits>

#include "am-float.h"

#include "log4sp/format.h"


namespace Log4sp {

// ref: https://github.com/alliedmodders/sourcemod/blob/master/core/logic/sprintf.cpp
inline static constexpr int LADJUST     = 0x00000001;   // left adjustment
inline static constexpr int ZEROPAD     = 0x00000002;   // zero (as opposed to blank) pad
inline static constexpr int UPPERDIGITS = 0x00000004;   // make alpha digits uppercase

#define CHECK_ARGS(x)   \
    if ((arg+x) > args) \
        ThrowError("String formatted incorrectly - parameter {} (total {})", arg, args);


template <class... Args>
[[noreturn]]
inline static
void ThrowError(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::throw_spdlog_ex(spdlog::fmt_lib::format(fmt, std::forward<Args>(args)...));
}

[[nodiscard]]
std::string FormatToString(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param)
{
    assert(ctx && params);

    char *format;
    CTX_LOCAL_TO_STRING(params[param], &format);
    unsigned int lparam = param + 1;
    return spdlog::fmt_lib::to_string(FormatToBuffer(ctx, format, params, &lparam));
}


inline static
void ReorderTranslationParams(const SourceMod::Translation *pTrans, cell_t *params) noexcept
{
    cell_t new_params[MAX_TRANSLATE_PARAMS];
    for (unsigned int i = 0; i < pTrans->fmt_count; ++i)
    {
        new_params[i] = params[pTrans->fmt_order[i]];
    }
    memcpy(params, new_params, pTrans->fmt_count * sizeof(cell_t));
}

inline static
spdlog::memory_buf_t Translate(SourcePawn::IPluginContext *ctx, const char *key, cell_t target, const cell_t *params, unsigned int *arg)
{
    unsigned int langid;
    SourceMod::Translation pTrans;
    SourceMod::IPlugin *pPlugin = PluginSysFindPluginByCtx(ctx);
    SourceMod::IPhraseCollection *pPhrases = pPlugin->GetPhrases();

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
        ThrowError("Translation failed: invalid client index {} (arg {})", target, *arg);
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
                ThrowError("Language phrase \"{}\" not found (arg {})", key, *arg);
            }
        }
        else
        {
            ThrowError("Language phrase \"{}\" not found (arg {})", key, *arg);
        }
    }

    unsigned int max_params = pTrans.fmt_count;

    if (max_params)
    {
        cell_t new_params[MAX_TRANSLATE_PARAMS];

        /* Check if we're going to over the limit */
        if ((*arg) + (max_params - 1) > static_cast<unsigned int>(params[0]))
            ThrowError("Translation string formatted incorrectly - missing at least {} parameters (arg {})",
                        ((*arg + (max_params - 1)) - params[0]), *arg);

        /**
         * If we need to re-order the parameters, do so with a temporary array.
         * Otherwise, we could run into trouble with continual formats, a la ShowActivity().
         */
        memcpy(new_params, params, sizeof(cell_t) * (params[0] + 1));
        ReorderTranslationParams(&pTrans, &new_params[*arg]);

        return FormatToBuffer(ctx, pTrans.szPhrase, new_params, arg);
    }

    return FormatToBuffer(ctx, pTrans.szPhrase, params, arg);
}

inline static
void AddString(spdlog::memory_buf_t &out, const char *string, unsigned int width, int prec, int flags) noexcept
{
    if (string == nullptr)
    {
        AddString(out, "(null)", width, prec, flags);
        return;
    }

    unsigned int size = static_cast<unsigned int>(std::strlen(string));
    if (prec >= 0 && static_cast<unsigned int>(prec) < size)
    {
        size = static_cast<unsigned int>(prec);
    }

    // Number of spaces to be pad
    unsigned int pads = (width <= size) ? (0u) : (width - size);

    // right justify if required
    if (!(flags & LADJUST))
    {
        while (pads)
        {
            pads--;
            out.push_back(' ');
        }
    }

    out.append(string, string + size);

    // left justify if required
    if (flags & LADJUST)
    {
        while (pads)
        {
            pads--;
            out.push_back(' ');
        }
    }
}

inline static
void AddFloat(spdlog::memory_buf_t &out, double fval, unsigned int width, int prec, int flags) noexcept
{
    int digits;                 // non-fraction part digits
    double tmp;                 // temporary
    int val;                    // temporary
    bool sign = false;          // false: positive, true: negative
    unsigned int fieldlength;   // for padding
    int significant_digits = 0; // number of significant digits written
    constexpr const int MAX_SIGNIFICANT_DIGITS = 16;

    if (ke::IsNaN(static_cast<float>(fval)))
    {
        AddString(out, "NaN", width, prec, flags);
        return;
    }

    if (ke::IsInfinite(static_cast<float>(fval)))
    {
        const char *str = ((fval < 0.0f) ? "-Inf" : "Inf");
        AddString(out, str, width, prec, flags);
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
        sign = true;
    }

    // compute whole-part digits count
    digits = (int)std::log10(fval) + 1;

    // Only print 0.something if 0 < fval < 1
    if (digits < 1)
    {
        digits = 1;
    }

    // compute the field length
    fieldlength = digits + prec + ((prec > 0) ? 1 : 0) + (sign ? 1 : 0);

    // minus sign BEFORE left padding if padding with zeros
    if (sign && (flags & ZEROPAD))
    {
        out.push_back('-');
    }

    // right justify if required
    if (!(flags & LADJUST))
    {
        while (fieldlength < width--)
        {
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
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
        while (digits--)
        {
            out.push_back('0');
        }
    }
    else
    {
        while (digits--)
        {
            val = (int)(fval / tmp);
            out.push_back('0' + static_cast<char>(val));
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
            out.push_back('0' + static_cast<char>(val));
            fval -= val * tmp;
        }
    }

    // left justify if required
    if (flags & LADJUST)
    {
        while (fieldlength < width--)
        {
            // right-padding only with spaces, ZEROPAD is ignored
            out.push_back(' ');
        }
    }
}

template <typename T>
inline static
void AddBinary(spdlog::memory_buf_t &out, T val, unsigned int width, int flags) noexcept
{
    static_assert(std::is_unsigned_v<T> && std::is_integral_v<T>, "T must be an unsigned integral type");

    constexpr const int MAX_TEXT = sizeof(T) * CHAR_BIT;
    char text[MAX_TEXT];
    int iter = MAX_TEXT - 1;

    do
    {
        text[iter--] = (val & 1) ? '1' : '0';
    } while (val >>= 1);

    const char *begin   = text + iter + 1;
    unsigned int digits = MAX_TEXT - iter - 1;
    unsigned int pads   = (width <= digits) ? (0u) : (width - digits);

    // right justify if required
    if (!(flags & LADJUST))
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    out.append(begin, text + MAX_TEXT);

    // left justify if required
    if (flags & LADJUST)
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

template <typename T>
inline static void AddUInt(spdlog::memory_buf_t &out, T val, unsigned int width, int flags) noexcept
{
    static_assert(std::is_unsigned_v<T> && std::is_integral_v<T>, "T must be an unsigned integral type");
    static_assert(std::numeric_limits<std::uint32_t>::digits10 == 9);
    static_assert(std::numeric_limits<std::uint64_t>::digits10 == 19);

    constexpr unsigned int MAX_TEXT = std::numeric_limits<T>::digits10 + 1;
    char text[MAX_TEXT];
    unsigned int digits = 0;

    do {
        text[digits++] = '0' + val % 10;
    } while (val /= 10);

    unsigned int pads = (width <= digits) ? (0u) : (width - digits);

    // right justify if required
    if (!(flags & LADJUST))
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    while (digits)
    {
        out.push_back(text[--digits]);
    }

    // left justify if required
    if (flags & LADJUST)
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

template <typename T>
inline static
void AddInt(spdlog::memory_buf_t &out, T val, unsigned int width, int flags) noexcept
{
    static_assert(std::is_integral_v<T>, "T must be an integral type");
    static_assert(std::numeric_limits<std::int32_t>::digits10 == 9);
    static_assert(std::numeric_limits<std::int64_t>::digits10 == 18);

    constexpr unsigned int MAX_TEXT = std::numeric_limits<int64_t>::digits10 + 2;
    char text[MAX_TEXT];
    unsigned int digits = 0;

    const bool negative = val < 0;
    std::make_unsigned_t<T> unsignedVal = negative ? std::abs(val) : val;

    do {
        text[digits++] = '0' + unsignedVal % 10;
    } while (unsignedVal /= 10);

    unsigned int pads = (width <= digits) ? (0u) : (width - digits);
    if (pads > 0 && negative) {
        pads--;
    }

    // minus sign BEFORE left padding if padding with zeros
    if (negative && (flags & ZEROPAD))
    {
        out.push_back('-');
    }

    // right justify if required
    if (!(flags & LADJUST))
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    // minus sign AFTER left padding if padding with spaces
    if (negative && !(flags & ZEROPAD))
    {
        out.push_back('-');
    }

    while (digits)
    {
        out.push_back(text[--digits]);
    }

    // left justify if required
    if (flags & LADJUST)
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

template <typename T>
inline static
void AddHex(spdlog::memory_buf_t &out, T val, unsigned int width, int flags) noexcept
{
    static_assert(std::is_unsigned_v<T> && std::is_integral_v<T>, "T must be an unsigned integral type");

    constexpr const char *hexUpper = "0123456789ABCDEF";
    constexpr const char *hexLower = "0123456789abcdef";
    const char *hexAdjust = (flags & UPPERDIGITS) ? hexUpper : hexLower;

    constexpr unsigned int MAX_TEXT = sizeof(T) * 16 / CHAR_BIT;
    char text[MAX_TEXT];
    unsigned int digits = 0;

    do {
        text[digits++] = hexAdjust[val & 0xF];
    } while(val >>= 4);

    unsigned int pads = (width <= digits) ? (0u) : (width - digits);

    // right justify if required
    if (!(flags & LADJUST))
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }

    while (digits)
    {
        out.push_back(text[--digits]);
    }

    // left justify if required
    if (flags & LADJUST)
    {
        while (pads)
        {
            pads--;
            out.push_back((flags & ZEROPAD) ? '0' : ' ');
        }
    }
}

inline static
bool DescribePlayer(int entRef, const char **namep, const char **authp, int *useridp) noexcept
{
    // ref: https://github.com/alliedmodders/sourcemod/blob/4afbf9d57328de327c504c4a184670d992ae1609/core/HalfLife2.h#L60
    constexpr int ENTREF_MASK = (1 << 31);

    int index = entRef;
    if (entRef & ENTREF_MASK)
    {
        index = gamehelpers->ReferenceToIndex(entRef);
    }

    SourceMod::IGamePlayer *player = playerhelpers->GetGamePlayer(index);
    if (!player || !player->IsConnected())
    {
        return false;
    }

    if (namep != nullptr)
    {
        *namep = player->GetName();
    }

    if (authp != nullptr)
    {
        const char *auth = player->GetAuthString();
        *authp = (auth && *auth) ? auth : "STEAM_ID_PENDING";
    }

    if (useridp != nullptr)
    {
        *useridp = player->GetUserId();
    }

    return true;
}

[[nodiscard]]
inline
spdlog::memory_buf_t FormatToBuffer(SourcePawn::IPluginContext *ctx, const char *layout, const cell_t *params, unsigned int *param)
{
    assert(ctx && layout && params && *param <= SP_MAX_EXEC_PARAMS);

    using spdlog::memory_buf_t;
    using spdlog::fmt_lib::format;

    memory_buf_t out;
    unsigned int args = params[0];  // params count
    unsigned int arg  = *param;     // 用于遍历 params 的指针
    const char *iter  = layout;     // 用于遍历 layout 的指针
    int flags;                      // 对齐 (左 / 右) | 填充符 ('0' / ' ')
    int prec;                       // 精度
    unsigned int width;             // 宽度

    while (true)
    {
        const char *begin = iter;

        // run through the layout string until we hit a '%' or '\0'
        while (*iter != '%' && *iter != '\0')
        {
            ++iter;
        }

        out.append(begin, iter);

        if (*iter == '\0')
        {
            *param = arg;
            return out;
        }

        // skip over the '%'
        ++iter;

        // reset formatting state
        flags = 0;
        width = 0;
        prec = -1;

rflag:
        char ch = *iter++;
reswitch:
        switch(ch)
        {
        case '-':
            {
                flags |= LADJUST;
                goto rflag;
            }
        case '.':
            {
                int n = 0;
                ch = *iter++;
                while (ch >= '0' && ch <= '9')
                {
                    n = 10 * n + (ch - '0');
                    ch = *iter++;
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
                unsigned int n = 0;
                do
                {
                    n = 10 * n + (ch - '0');
                    ch = *iter++;
                } while(ch >= '0' && ch <= '9');
                width = n;
                goto reswitch;
            }
        case 'c':
            {
                CHECK_ARGS(0);
                char *c;
                CTX_LOCAL_TO_STRING(params[arg], &c);

                out.push_back(*c);
                ++arg;
                break;
            }
        case 'b':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddBinary(out, static_cast<std::uint32_t>(*value), width, flags);
                ++arg;
                break;
            }
        case 'd':
        case 'i':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddInt(out, static_cast<int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'u':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddUInt(out, static_cast<std::uint32_t>(*value), width, flags);
                ++arg;
                break;
            }
        case 'f':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddFloat(out, sp_ctof(*value), width, prec, flags);
                ++arg;
                break;
            }
        case 'L':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                if (*value)
                {
                    const char *name;
                    const char *auth;
                    int userid;
                    if (!DescribePlayer(*value, &name, &auth, &userid))
                        ThrowError("Client index {} is invalid (arg {})", *value, arg);

                    AddString(out, format("{}<{}><{}><>", name, userid, auth).c_str(), width, prec, flags);
                }
                else
                {
                    AddString(out, "Console<0><Console><Console>", width, prec, flags);
                }
                ++arg;
                break;
            }
        case 'N':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                if (*value)
                {
                    const char *name;
                    if (!DescribePlayer(*value, &name, nullptr, nullptr))
                        ThrowError("Client index {} is invalid (arg {})", *value, arg);

                    AddString(out, name, width, prec, flags);
                }
                else
                {
                    AddString(out, "Console", width, prec, flags);
                }
                ++arg;
                break;
            }
        case 'E':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                CBaseEntity *entity = gamehelpers->ReferenceToEntity(*value);
                if (!entity)
                    ThrowError("Entity index {} is invalid (arg {})", *value, arg);

                // 可能返回 nullptr, 但 AddString 有保障机制
                const char *classname = gamehelpers->GetEntityClassname(entity);
                AddString(out, classname, width, prec, flags);
                ++arg;
                break;
            }
        case 's':
            {
                CHECK_ARGS(0);
                char *str;
                CTX_LOCAL_TO_STRING(params[arg], &str);

                AddString(out, str, width, prec, flags);
                ++arg;
                break;
            }
        case 'T':
            {
                CHECK_ARGS(1);
                char *key;
                cell_t *target;
                CTX_LOCAL_TO_STRING(params[arg++], &key);
                CTX_LOCAL_TO_PHYS_ADDR(params[arg++], &target);

                spdlog::memory_buf_t phrase = Translate(ctx, key, *target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 't':
            {
                CHECK_ARGS(0);
                char *key;
                CTX_LOCAL_TO_STRING(params[arg++], &key);
                auto target = static_cast<cell_t>(translator->GetGlobalTarget());

                spdlog::memory_buf_t phrase = Translate(ctx, key, target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 'X':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddHex(out, static_cast<unsigned int>(*value), width, flags | UPPERDIGITS);
                ++arg;
                break;
            }
        case 'x':
            {
                CHECK_ARGS(0);
                cell_t *value;
                CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                AddHex(out, static_cast<unsigned int>(*value), width, flags);
                ++arg;
                break;
            }
        case 'l':
            {
                CHECK_ARGS(0);
                ch = *iter++;

                switch (ch)
                {
                case 'b':
                    {
                        cell_t *value;
                        CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                        AddBinary(out, *reinterpret_cast<std::uint64_t*>(value), width, flags);
                        ++arg;
                        break;
                    }
                case 'd':
                case 'i':
                    {
                        cell_t *value;
                        CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                        AddInt(out, *reinterpret_cast<std::int64_t*>(value), width, flags);
                        ++arg;
                        break;
                    }
                case 'u':
                    {
                        cell_t *value;
                        CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                        AddUInt(out, *reinterpret_cast<std::uint64_t*>(value), width, flags);
                        ++arg;
                        break;
                    }
                case 'X':
                    {
                        cell_t *value;
                        CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                        AddHex(out, *reinterpret_cast<std::uint64_t*>(value), width, flags | UPPERDIGITS);
                        ++arg;
                        break;
                    }
                case 'x':
                    {
                        cell_t *value;
                        CTX_LOCAL_TO_PHYS_ADDR(params[arg], &value);

                        AddHex(out, *reinterpret_cast<std::uint64_t*>(value), width, flags);
                        ++arg;
                        break;
                    }
                default:
                    ThrowError("{}", "Invalid formatter. Only %lb, %ld, %li, %lu, %lX, %lx are allowed.");
                }
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
                *param = arg;
                return out;
            }
        default:
            {
                out.push_back(ch);
                break;
            }
        }
    }

    *param = arg;
    return out;
}


}       // namespace Log4sp
