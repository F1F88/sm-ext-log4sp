#include <cassert>

#include "log4sp/common.h"

namespace log4sp {

namespace fmt_lib = spdlog::fmt_lib;
using spdlog::filename_t;
using spdlog::level::level_enum;
using spdlog::memory_buf_t;
using spdlog::pattern_time_type;
using spdlog::source_loc;

[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno) {
    spdlog::throw_spdlog_ex(msg, last_errno);
}

[[noreturn]] void throw_log4sp_ex(std::string msg) {
    spdlog::throw_spdlog_ex(std::move(msg));
}

// src_helper
[[nodiscard]] source_loc src_helper::get() const noexcept {
    if (!loc_.empty()) {
        return loc_;
    } else if (ctx_) {
        loc_ = get_from_plugin_ctx(ctx_); // 缓存以用于下一个节点(sink)也出错时
        return loc_;
    }

    assert(false);
    return source_loc{};
}

[[nodiscard]] source_loc src_helper::get_from_plugin_ctx(SourcePawn::IPluginContext *ctx) noexcept {
    assert(ctx);

    unsigned int line{0};
    const char *file{nullptr};
    const char *func{nullptr};

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    do {
        if (iter->IsScriptedFrame()) {
            line = iter->LineNumber();
            file = iter->FilePath();
            func = iter->FunctionName();
            break;
        }
        iter->Next();
    } while (!iter->Done());
    ctx->DestroyFrameIterator(iter);

    return source_loc(file, static_cast<int>(line), func);
}

[[nodiscard]] std::vector<std::string> src_helper::get_stack_trace(SourcePawn::IPluginContext *ctx) noexcept {
    assert(ctx);

    SourcePawn::IFrameIterator *iter = ctx->CreateFrameIterator();
    if (iter->Done()) {
        ctx->DestroyFrameIterator(iter);
        return {};
    }

    std::vector<std::string> trace{"Call stack trace:"};

    for (int index = 0; !iter->Done(); iter->Next(), ++index) {
        if (iter->IsNativeFrame()) {
            const char *func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            trace.emplace_back(fmt_lib::format("  [{}] {}", index, func));
        } else if (iter->IsScriptedFrame()) {
            const char *func = iter->FunctionName();
            if (!func) {
                func = "<unknown function>";
            }

            const char *file = iter->FilePath();
            if (!file) {
                func = "<unknown>";
            }

            trace.emplace_back(fmt_lib::format("  [{}] Line {}, {}::{}", index, iter->LineNumber(), file, func));
        }
    }

    ctx->DestroyFrameIterator(iter);
    return trace;
}

// err_helper
void err_helper::handle_ex(const std::string &origin, const src_helper &src, const std::exception &ex) const noexcept {
    try {
        const source_loc loc{src.get()};
        if (custom_error_handler_) {
            custom_error_handler_->PushString(ex.what());
            custom_error_handler_->PushString(origin.c_str());
            custom_error_handler_->PushString(loc.filename);
            custom_error_handler_->PushCell(loc.line);
            custom_error_handler_->PushString(loc.funcname);
#ifndef DEBUG
            custom_error_handler_->Execute();
#else
            assert(custom_error_handler_->Execute() == SP_ERROR_NONE);
#endif
            return;
        }
        smutils->LogError(myself, "[%s::%d] [%s] %s", get_path_filename(loc.filename), loc.line, origin.c_str(), ex.what());
    } catch (const std::exception &handler_ex) {
        smutils->LogError(myself, "[%s] caught exception during error handler: %s", origin.c_str(), handler_ex.what());
    } catch (...) {
        smutils->LogError(myself, "[%s] caught unknown exception during error handler", origin.c_str());
    }
}

void err_helper::handle_unknown_ex(const std::string &origin, const src_helper &src) const noexcept {
    handle_ex(origin, src, std::runtime_error("unknown exception"));
}

void err_helper::set_err_handler(SourceMod::IChangeableForward *handler) noexcept {
    assert(handler);
    release_forward();
    custom_error_handler_ = handler;
}

err_helper::~err_helper() noexcept {
    release_forward();
}

void err_helper::release_forward() noexcept {
    if (custom_error_handler_) {
        forwards->ReleaseForward(custom_error_handler_);
        custom_error_handler_ = nullptr;
    }
}


[[nodiscard]] spdlog::filename_t unbuild_path(SourceMod::PathType type, const filename_t &filename) noexcept
{
    const char *base = nullptr;
    switch (type)
    {
    case SourceMod::PathType::Path_Game:
        base = smutils->GetGamePath();
        break;
    case SourceMod::PathType::Path_SM:
        base = smutils->GetSourceModPath();
        break;
    case SourceMod::PathType::Path_SM_Rel:
        // TODO
        break;
    default:
        break;
    }

    if (base)
    {
        return filename.substr(std::strlen(base) + 1);
    }
    return filename;
}


[[nodiscard]] std::string format_cell_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param) {
    assert(ctx && params);

    char *format;
    ctx->LocalToString(params[param], &format);
    unsigned int lparam{param + 1};
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
    for (unsigned int i = 0; i < pTrans->fmt_count; ++i) {
        new_params[i] = params[pTrans->fmt_order[i]];
    }
    memcpy(params, new_params, pTrans->fmt_count * sizeof(cell_t));
}

static memory_buf_t Translate(SourcePawn::IPluginContext *ctx, const char *key, cell_t target, const cell_t *params, unsigned int *arg) {
    unsigned int langid;
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

    unsigned int max_params = pTrans.fmt_count;

    if (max_params) {
        cell_t new_params[MAX_TRANSLATE_PARAMS];

        /* Check if we're going to over the limit */
        if ((*arg) + (max_params - 1) > static_cast<unsigned int>(params[0])) {
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

static void AddString(memory_buf_t &out, const char *string, unsigned int width, int prec, int flags) noexcept {
    if (string == nullptr) {
        constexpr const char *nlstr{"(null)"};
        constexpr const unsigned int size{sizeof(nlstr)};

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
        unsigned int size{static_cast<unsigned int>(strlen(string))};
        if (prec >= 0 && static_cast<unsigned int>(prec) < size) {
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

static void AddFloat(memory_buf_t &out, double fval, unsigned int width, int prec, int flags) noexcept {
    int digits;                 // non-fraction part digits
    double tmp;                 // temporary
    int val;                    // temporary
    bool sign{false};           // false: positive, true: negative
    unsigned int fieldlength;   // for padding
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
    digits = (std::max)((int)std::log10(fval) + 1, 1);

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

static void AddBinary(memory_buf_t &out, unsigned int val, unsigned int width, int flags) noexcept {
    char text[32];

    int iter{31};
    do {
        text[iter--] = (val & 1) ? '1' : '0';
    } while (val >>= 1);

    const char *begin = text + iter + 1;
    unsigned int digits = 31 - iter;

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

static void AddUInt(memory_buf_t &out, unsigned int val, unsigned int width, int flags) noexcept {
    char text[10];
    unsigned int digits{0};
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
        width = width <= digits ? 0u : width - digits;
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

static void AddInt(memory_buf_t &out, int val, unsigned int width, int flags) noexcept {
    char text[10];
    unsigned int digits{0};

    bool negative = val < 0;
    unsigned int unsignedVal = negative ? abs(val) : val;

    do {
        text[digits++] = '0' + unsignedVal % 10;
    } while (unsignedVal /= 10);

    if (!(flags & LADJUST)) {
        if (flags & ZEROPAD) {
            if (negative) {
                out.push_back('-');
                width = (width >= 1u) ? width - 1u : 0u;
            }

            while (digits < width--) {
                out.push_back('0');
            }
        } else {
            if (negative) {
                width = (width >= 1u) ? width - 1u : 0u;

                while (digits < width--) {
                    out.push_back(' ');
                }

                out.push_back('-');
            } else {
                while (digits < width--) {
                    out.push_back(' ');
                }
            }
        }

        while (digits--) {
            out.push_back(text[digits]);
        }
    } else {
        if (negative) {
            out.push_back('-');
            width = (width >= 1u) ? width - 1u : 0u;
        }

        width = width <= digits ? 0u : width - digits;
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

static void AddHex(memory_buf_t &out, unsigned int val, unsigned int width, int flags) noexcept {
    constexpr const char *hexUpper{"0123456789ABCDEF"};
    constexpr const char *hexlower{"0123456789abcdef"};
    const char *hexAdjust = (flags & UPPERDIGITS) ? hexUpper : hexlower;
    char text[8];
    unsigned int digits{0};

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
        width = width <= digits ? 0u : width - digits;
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

static bool DescribePlayer(int entRef, const char **namep, const char **authp, int *useridp) noexcept {
    constexpr const int ENTREF_MASK = (1 << 31);

    int index{entRef};
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

[[nodiscard]] memory_buf_t format_cell_to_mem_buf(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, unsigned int *param) {
    assert(ctx && format && params && *param > 0);

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
                unsigned int n{0};
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
                AddUInt(out, static_cast<unsigned int>(*value), width, flags);
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
                        throw_log4sp_ex(fmt_lib::format("Client index {} is invalid (arg {})", *value, arg));
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
                        throw_log4sp_ex(fmt_lib::format("Client index {} is invalid (arg {})", *value, arg));
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
                memory_buf_t phrase = Translate(ctx, key, *target, params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 't': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                char *key;
                ctx->LocalToString(params[arg++], &key);
                memory_buf_t phrase = Translate(ctx, key, static_cast<cell_t>(translator->GetGlobalTarget()), params, &arg);
                out.append(phrase.begin(), phrase.end());
                break;
            }
        case 'X': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
                }

                cell_t *value;
                ctx->LocalToPhysAddr(params[arg], &value);
                AddHex(out, static_cast<unsigned int>(*value), width, flags | UPPERDIGITS);
                ++arg;
                break;
            }
        case 'x': {
                if (arg > args) {
                    throw_log4sp_ex(fmt_lib::format("String formatted incorrectly - parameter {} (total {})", arg, args));
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
