#pragma semicolon 1
#pragma newdecls required
#pragma dynamic 32768

#if !defined DEBUG
    #define  DEBUG
#endif

#if !defined _DEBUG
    #define  _DEBUG
#endif

#if defined NDEBUG
    #undef  NDEBUG
#endif

#if !defined SP_MAX_EXEC_PARAMS
    #define  SP_MAX_EXEC_PARAMS             32
#endif

#if !defined RANDOM_ITERATIONS
    #define  RANDOM_ITERATIONS              0
#endif

#if !defined ASSERT_OPTION_FLOAT_THRESHOLD
    #define  ASSERT_OPTION_FLOAT_THRESHOLD  0.000002
#endif

#if !defined EXPECTED_MAX_LENGTH
    #define  EXPECTED_MAX_LENGTH            128
#endif

#include <sourcemod>

#if RANDOM_ITERATIONS > 0
#include <sdktools>
#endif

#include <log4sp>

#include "../assert"
#include "../test_sink"
#include "../test_utils"


/**
 * %[flags][width][.precision]specifier
 *  flags: none, "0",  "-", "-0", "0-"
 *  width: none, "0",       mini, midi, maxi
 *  .prec: none, ".0", ".", mini, midi, maxi
 *  specifier: %, c, b, d, i, u, f, L, N, E, s, T, t, X, x
 *            lb, ld, li, lu, lX, lx
 *
 *   A      (            - 1.13.0.7198)     Overflow when left-aligned and padded with '0'
 *   B      (            - 1.13.0.7269)     Supports floating-point number formatting "Inf"
 *   C      (            - 1.13.0.7270)     The minus sign is misaligned when dealing with negative integers
 *   D      (            - 1.13.0.7271)     "%s" always being left justify
 *   E      (            - 1.13.0.7276)     Add format specifier "%E"
 *   F      (            - 1.13.0.7326)     Add format specifier "%lu" & "%li" & "%ld"
 *   G      (            - 1.13.0.7330)     (-Inf) is formatted as "Inf"
 *   H      [1.13.0.7270 - 1.13.0.7331)     Width overflow occurs when processing negative integers
 *   I      (            - 1.13.0.7342)     Add format specifier "%lb" & "%lX" & "%lx"
 *   J      (            -            )     "%c" does not support advanced formatting, and truncate when the value is 0
 *   K      (            -            )     "%T" & "%t" does not support advanced formatting
 */


enum Expected
{
    Exp_None_None_None, Exp_None_None_Dot, Exp_None_None_Zero, Exp_None_None_Mini, Exp_None_None_Midi, Exp_None_None_Maxi,
    Exp_None_Zero_None, Exp_None_Zero_Dot, Exp_None_Zero_Zero, Exp_None_Zero_Mini, Exp_None_Zero_Midi, Exp_None_Zero_Maxi,
    Exp_None_Mini_None, Exp_None_Mini_Dot, Exp_None_Mini_Zero, Exp_None_Mini_Mini, Exp_None_Mini_Midi, Exp_None_Mini_Maxi,
    Exp_None_Midi_None, Exp_None_Midi_Dot, Exp_None_Midi_Zero, Exp_None_Midi_Mini, Exp_None_Midi_Midi, Exp_None_Midi_Maxi,
    Exp_None_Maxi_None, Exp_None_Maxi_Dot, Exp_None_Maxi_Zero, Exp_None_Maxi_Mini, Exp_None_Maxi_Midi, Exp_None_Maxi_Maxi,

    Exp_Zero_None_None, Exp_Zero_None_Dot, Exp_Zero_None_Zero, Exp_Zero_None_Mini, Exp_Zero_None_Midi, Exp_Zero_None_Maxi,
    Exp_Zero_Zero_None, Exp_Zero_Zero_Dot, Exp_Zero_Zero_Zero, Exp_Zero_Zero_Mini, Exp_Zero_Zero_Midi, Exp_Zero_Zero_Maxi,
    Exp_Zero_Mini_None, Exp_Zero_Mini_Dot, Exp_Zero_Mini_Zero, Exp_Zero_Mini_Mini, Exp_Zero_Mini_Midi, Exp_Zero_Mini_Maxi,
    Exp_Zero_Midi_None, Exp_Zero_Midi_Dot, Exp_Zero_Midi_Zero, Exp_Zero_Midi_Mini, Exp_Zero_Midi_Midi, Exp_Zero_Midi_Maxi,
    Exp_Zero_Maxi_None, Exp_Zero_Maxi_Dot, Exp_Zero_Maxi_Zero, Exp_Zero_Maxi_Mini, Exp_Zero_Maxi_Midi, Exp_Zero_Maxi_Maxi,

    Exp_Dash_None_None, Exp_Dash_None_Dot, Exp_Dash_None_Zero, Exp_Dash_None_Mini, Exp_Dash_None_Midi, Exp_Dash_None_Maxi,
    Exp_Dash_Zero_None, Exp_Dash_Zero_Dot, Exp_Dash_Zero_Zero, Exp_Dash_Zero_Mini, Exp_Dash_Zero_Midi, Exp_Dash_Zero_Maxi,
    Exp_Dash_Mini_None, Exp_Dash_Mini_Dot, Exp_Dash_Mini_Zero, Exp_Dash_Mini_Mini, Exp_Dash_Mini_Midi, Exp_Dash_Mini_Maxi,
    Exp_Dash_Midi_None, Exp_Dash_Midi_Dot, Exp_Dash_Midi_Zero, Exp_Dash_Midi_Mini, Exp_Dash_Midi_Midi, Exp_Dash_Midi_Maxi,
    Exp_Dash_Maxi_None, Exp_Dash_Maxi_Dot, Exp_Dash_Maxi_Zero, Exp_Dash_Maxi_Mini, Exp_Dash_Maxi_Midi, Exp_Dash_Maxi_Maxi,

    Exp_Daro_None_None, Exp_Daro_None_Dot, Exp_Daro_None_Zero, Exp_Daro_None_Mini, Exp_Daro_None_Midi, Exp_Daro_None_Maxi,
    Exp_Daro_Zero_None, Exp_Daro_Zero_Dot, Exp_Daro_Zero_Zero, Exp_Daro_Zero_Mini, Exp_Daro_Zero_Midi, Exp_Daro_Zero_Maxi,
    Exp_Daro_Mini_None, Exp_Daro_Mini_Dot, Exp_Daro_Mini_Zero, Exp_Daro_Mini_Mini, Exp_Daro_Mini_Midi, Exp_Daro_Mini_Maxi,
    Exp_Daro_Midi_None, Exp_Daro_Midi_Dot, Exp_Daro_Midi_Zero, Exp_Daro_Midi_Mini, Exp_Daro_Midi_Midi, Exp_Daro_Midi_Maxi,
    Exp_Daro_Maxi_None, Exp_Daro_Maxi_Dot, Exp_Daro_Maxi_Zero, Exp_Daro_Maxi_Mini, Exp_Daro_Maxi_Midi, Exp_Daro_Maxi_Maxi,

    Exp_Zesh_None_None, Exp_Zesh_None_Dot, Exp_Zesh_None_Zero, Exp_Zesh_None_Mini, Exp_Zesh_None_Midi, Exp_Zesh_None_Maxi,
    Exp_Zesh_Zero_None, Exp_Zesh_Zero_Dot, Exp_Zesh_Zero_Zero, Exp_Zesh_Zero_Mini, Exp_Zesh_Zero_Midi, Exp_Zesh_Zero_Maxi,
    Exp_Zesh_Mini_None, Exp_Zesh_Mini_Dot, Exp_Zesh_Mini_Zero, Exp_Zesh_Mini_Mini, Exp_Zesh_Mini_Midi, Exp_Zesh_Mini_Maxi,
    Exp_Zesh_Midi_None, Exp_Zesh_Midi_Dot, Exp_Zesh_Midi_Zero, Exp_Zesh_Midi_Mini, Exp_Zesh_Midi_Midi, Exp_Zesh_Midi_Maxi,
    Exp_Zesh_Maxi_None, Exp_Zesh_Maxi_Dot, Exp_Zesh_Maxi_Zero, Exp_Zesh_Maxi_Mini, Exp_Zesh_Maxi_Midi, Exp_Zesh_Maxi_Maxi,

    Exp_All
};


// Native 虽然可以处理更多可变参数, 但处理不同类型时较为麻烦, 尤其对于 %T & %t
// 宏函数可以很好的替换所有数据类型, 最多允许10个参数
// void AssertFmt(const char[] expected, const char[] specifier, any ...);
#define ASSERT_FMT1(%0,%1,%2)                       ASSERT_FMT8(%0,%1,%2, 0, 0, 0, 0, 0, 0, 0)
#define ASSERT_FMT2(%0,%1,%2,%3)                    ASSERT_FMT8(%0,%1,%2,%3, 0, 0, 0, 0, 0, 0)
#define ASSERT_FMT3(%0,%1,%2,%3,%4)                 ASSERT_FMT8(%0,%1,%2,%3,%4, 0, 0, 0, 0, 0)
#define ASSERT_FMT4(%0,%1,%2,%3,%4,%5)              ASSERT_FMT8(%0,%1,%2,%3,%4,%5, 0, 0, 0, 0)
#define ASSERT_FMT5(%0,%1,%2,%3,%4,%5,%6)           ASSERT_FMT8(%0,%1,%2,%3,%4,%5,%6, 0, 0, 0)
#define ASSERT_FMT6(%0,%1,%2,%3,%4,%5,%6)           ASSERT_FMT8(%0,%1,%2,%3,%4,%5,%6,%7, 0, 0)
#define ASSERT_FMT7(%0,%1,%2,%3,%4,%5,%6)           ASSERT_FMT8(%0,%1,%2,%3,%4,%5,%6,%7,%8, 0)
#define ASSERT_FMT8(%0,%1,%2,%3,%4,%5,%6,%7,%8,%9) {                                                \
    TestSink __sink = new TestSink();                                                               \
    Logger __logger = new Logger();                                                                 \
    __logger.AddSink(__sink);                                                                       \
    __logger.SetPattern("%v");                                                                      \
                                                                                                    \
    char __fmt[32];                                                                                 \
    int __len = FormatEx(__fmt, sizeof(__fmt), "%%%s", %1) - 1;                                     \
                                                                                                    \
    if (__fmt[__len] == 'c' || __fmt[__len] == 'b' || __fmt[__len] == 'd' || __fmt[__len] == 'i' || \
        __fmt[__len] == 'u' || __fmt[__len] == 'L' || __fmt[__len] == 'N' || __fmt[__len] == 'E' || \
        __fmt[__len] == 's' || __fmt[__len] == 'X' || __fmt[__len] == 'x')                          \
    {                                                                                               \
        __logger.InfoF(__fmt, %2);                                                                  \
        AssertStrEq(__fmt, __sink.DrainOldest().msg, %0);                                           \
    }                                                                                               \
    else if (__fmt[__len] == 'f')                                                                   \
    {                                                                                               \
        __logger.InfoF(__fmt, %2);                                                                  \
        AssertFloatEq(__fmt, StringToFloat(__sink.DrainOldest().msg), StringToFloat(%0));           \
    }                                                                                               \
    else if (__fmt[__len] == 'T' || __fmt[__len] == 't')                                            \
    {                                                                                               \
        __logger.InfoF(__fmt, %2,%3,%4,%5,%6,%7,%8,%9);                                             \
        AssertStrEq(__fmt, __sink.DrainOldest().msg, %0);                                           \
    }                                                                                               \
    else                                                                                            \
        ThrowError("Invalid specifier %s.", __fmt);                                                 \
    __logger.Close();                                                                               \
    __sink.Close();                                                                                 \
}

// void AssertFmts(const char expecteds[Exp_All][EXPECTED_MAX_LENGTH],
//                 int width[3], int prec[3], const char[] specifier, any ...);
#define ASSERT_FMTS1(%0,%1,%2,%3,%4)                ASSERT_FMTS6(%0,%1,%2,%3,%4, 0, 0, 0, 0,0)
#define ASSERT_FMTS2(%0,%1,%2,%3,%4,%5)             ASSERT_FMTS6(%0,%1,%2,%3,%4,%5, 0, 0, 0,0)
#define ASSERT_FMTS3(%0,%1,%2,%3,%4,%5,%6)          ASSERT_FMTS6(%0,%1,%2,%3,%4,%5,%6, 0, 0,0)
#define ASSERT_FMTS4(%0,%1,%2,%3,%4,%5,%6,%7)       ASSERT_FMTS6(%0,%1,%2,%3,%4,%5,%6,%7, 0,0)
#define ASSERT_FMTS5(%0,%1,%2,%3,%4,%5,%6,%7,%8)    ASSERT_FMTS6(%0,%1,%2,%3,%4,%5,%6,%7,%8,0)
#define ASSERT_FMTS6(%0,%1,%2,%3,%4,%5,%6,%7,%8,%9) {                                               \
    char __flags[][4]   = {"", "0",  "-", "-0", "0-"};                                              \
    char __widths[][12] = {"", "0",   "",   "",   ""};                                              \
    char __precs[][12]  = {"", ".", ".0",   "",   "", ""};                                          \
                                                                                                    \
    IntToString(%1[0], __widths[2], sizeof(__widths[]));                                            \
    IntToString(%1[1], __widths[3], sizeof(__widths[]));                                            \
    IntToString(%1[2], __widths[4], sizeof(__widths[]));                                            \
    FormatEx(__precs[3], sizeof(__precs[]), ".%d", %2[0]);                                          \
    FormatEx(__precs[4], sizeof(__precs[]), ".%d", %2[1]);                                          \
    FormatEx(__precs[5], sizeof(__precs[]), ".%d", %2[2]);                                          \
                                                                                                    \
    int __fmtSize = sizeof(__widths[]) + sizeof(__precs[]) + strlen(%3);                            \
    char[] __fmt = new char[__fmtSize];                                                             \
                                                                                                    \
    TestSink __sink = new TestSink();                                                               \
    Logger __logger = new Logger();                                                                 \
    __logger.AddSink(__sink);                                                                       \
    __logger.SetPattern("%v");                                                                      \
                                                                                                    \
    for (int __i = 0; __i < sizeof(__flags); ++__i) {                                               \
        for (int __j = 0; __j < sizeof(__widths); ++__j) {                                          \
            for (int __k = 0; __k < sizeof(__precs); ++__k) {                                       \
                FormatEx(__fmt, __fmtSize, "[%%%s%s%s%s]",                                          \
                         __flags[__i],  __widths[__j], __precs[__k], %3);                           \
                __logger.InfoF(__fmt, %4,%5,%6,%7,%8,%9);                                           \
                                                                                                    \
                int __Idx = __i * sizeof(__widths) * sizeof(__precs) + __j * sizeof(__precs) + __k; \
                AssertStrEq(__fmt, __sink.DrainOldest().msg, %0[__Idx]);                            \
            }                                                                                       \
        }                                                                                           \
    }                                                                                               \
    __logger.Close();                                                                               \
    __sink.Close();                                                                                 \
}



// 确保 worldspawn 生成后再开始测试
public void OnMapStart()
{
    RequestFrame(Test);
    RegServerCmd("sm_log4sp_test_format", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
#if defined LOG4SP_HEADER_ONLY
    static int major = -1, minor, patch, build;
    if (major == -1)
    {
        FetchSourceModVersionFromCommand(major, minor, patch, build);
        RequestFrame(Test);
        return;
    }

    if ((major <= 1 && minor < 13) || (major <= 1 && minor == 13 && build < 7342))
    {
        PrintToServer(
            "[Warn] Ignore format test (headler-only) because the SM version (%d.%d.%d.%d) is lower than 1.13.7342.",
            major, minor, patch, build);
        return;
    }
#endif

    PrintToServer("---------- Started testing Logger-Format ---------");

    TestChar();

    TestBinary();

    TestInt();

    TestUInt();

    TestFloat();

    TestSpecial();

    TestString();

    TestTranslates();

    TestHex();

#if defined SM_INT64_SUPPORTED
    TestBinary64();

    TestInt64();

    TestUInt64();

    TestHex64();
#endif

    PrintToServer("------------ Test Logger-Format ended ------------");
}

void TestChar()
{
    SetTestContext("Format Character");

    char specifier[] = "c";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];

    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[a]");
        int widths[3] = {1, 2, 3}, precs[3] = {4, 5, 6};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 'a')
    }

    int iters = RANDOM_ITERATIONS - 32;
    if (iters > 126)
        iters = 126;

    for (int i = 32; i <= iters; ++i)
    {
        char value = view_as<char>(i);
        char expected[8];
        FormatEx(expected, sizeof(expected), "%c", value);

        ASSERT_FMT1(expected, specifier, value)
    }
}


void TestBinary()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Binary");

    char specifier[] = "b";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[  0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[  0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[  0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[  0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[  0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[  0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0  ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[000]");
        int widths[3] = {0, 1, 3}, precs[3] = {5, 7, 9};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000100101101011010000111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[100101101011010000111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[100101101011010000111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[100101101011010000111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[100101101011010000111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[100101101011010000111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[100101101011010000111    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1001011010110100001110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1001011010110100001110000]");
        int widths[3] = {17, 21, 25}, precs[3] = {18, 32, 36};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    1111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    1111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    1111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    1111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    1111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    1111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00001111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00001111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00001111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00001111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00001111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00001111111111111111111111111111111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111110000]");
        int widths[3] = {17, 31, 35}, precs[3] = {18, 32, 36};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 2147483647)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[         11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[         11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[         11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[         11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[         11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[         11111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111111         ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111111         ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111111         ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111111         ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111111         ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111111         ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111111000000000]");
        int widths[3] = {15, 32, 41}, precs[3] = {15, 21, 30};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0xFFFFFFFF)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[           11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[           11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[           11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[           11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[           11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[           11111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111111           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111111           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111111           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111111           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111111           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111111           ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111100000000000]");
        int widths[3] = {15, 32, 43}, precs[3] = {17, 23, 33};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0xFFFFFFFF)
    }
}


void TestInt()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Integer");

    char specifier[2][] = {"d", "i"};
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-2147483648]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            -2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            -2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            -2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            -2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            -2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            -2147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-0000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-0000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-0000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-0000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-0000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000000000002147483648]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-2147483648            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-2147483648            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-2147483648            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-2147483648            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-2147483648            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-2147483648            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-2147483648000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-2147483648000000000000]");
        int widths[3] = {7, 11, 23}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], -2147483648)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], -2147483648)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-1234567]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-1234567000000000000]");
        int widths[3] = {3, 8, 20}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], -1234567)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], -1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ -1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ -1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ -1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ -1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ -1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ -1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-01]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-01]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-01]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-01]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-01]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-01]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-1 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-10]");
        int widths[3] = {1, 2, 3}, precs[3] = {4, 5, 6};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 0xFFFFFFFF)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 0xFFFFFFFF)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        int widths[3] = {0, 1, 2}, precs[3] = {3, 4, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 0)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ 1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ 1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ 1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ 1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ 1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ 1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[01]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[01]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[01]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[01]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[01]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[01]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[10]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[10]");
        int widths[3] = {0, 1, 2}, precs[3] = {3, 4, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 1)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1234567]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        int widths[3] = {0, 7, 19}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 1234567)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[2147483647]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[2147483647000]");
        int widths[3] = {7, 10, 13}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 2147483647)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 2147483647)
    }

    // Random
    for (int i = 0; i < RANDOM_ITERATIONS; ++i)
    {
        int value = GetRandomInt(-123456789, 123456789);
        char expected[12];
        IntToString(value, expected, sizeof(expected));

        ASSERT_FMT1(expected, specifier[i & 1], value)
    }
}

void TestUInt()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Unsigned Integer");

    char specifier[] = "u";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[2147483648]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[             2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[             2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[             2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[             2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[             2147483648]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[             2147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000000002147483648]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000000002147483648]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[2147483648             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[2147483648             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[2147483648             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[2147483648             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[2147483648             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[2147483648             ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[21474836480000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[21474836480000000000000]");
        int widths[3] = {7, 10, 23}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -2147483648)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[4293732729]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            4293732729]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            4293732729]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            4293732729]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            4293732729]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            4293732729]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            4293732729]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000004293732729]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000004293732729]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000004293732729]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000004293732729]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000004293732729]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000004293732729]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[4293732729            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[4293732729            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[4293732729            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[4293732729            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[4293732729            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[4293732729            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[4293732729000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[4293732729000000000000]");
        int widths[3] = {3, 10, 22}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[4294967295]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ 4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ 4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ 4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ 4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ 4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ 4294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[04294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[04294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[04294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[04294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[04294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[04294967295]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[4294967295 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[4294967295 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[4294967295 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[4294967295 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[4294967295 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[4294967295 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[42949672950]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[42949672950]");
        int widths[3] = {3, 10, 11}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0xFFFFFFFF)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        int widths[3] = {0, 1, 2}, precs[3] = {3, 4, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        int widths[3] = {0, 1, 13}, precs[3] = {3, 4, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 1)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1234567]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        int widths[3] = {0, 7, 19}, precs[3] = {3, 4, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[2147483647]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   2147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0002147483647]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[2147483647   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[2147483647000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[2147483647000]");
        int widths[3] = {7, 10, 13}, precs[3] = {5, 7, 9};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 2147483647)
    }

    // Random
    for (int i = 0; i < RANDOM_ITERATIONS; ++i)
    {
        int value = GetRandomInt(0, 123456789);
        char expected[16];
        IntToString(value, expected, sizeof(expected));

        ASSERT_FMT1(expected, specifier, value)
    }
}


void TestFloat()
{
    // Ref sprintf: right-padding only with spaces, ZEROPAD is ignored
    // 尾部填充 '0' 会被替换为 ' '
    SetTestContext("Format Float");

    char specifier[] = "f";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-123456.125000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[      -123456]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[      -123456]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[  -123456.125]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       -123456.125000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              -123456]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              -123456]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          -123456.125]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       -123456.125000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    -123456.125000000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[-000000123456]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[-000000123456]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[-00123456.125]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-0000000123456.125000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-00000000000000123456]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-00000000000000123456]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-0000000000123456.125]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-0000000123456.125000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000123456.125000000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[-123456      ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[-123456      ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[-123456.125  ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-123456.125000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-123456              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-123456              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-123456.125          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-123456.125000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-123456.125000000    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[-123456      ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[-123456      ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[-123456.125  ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-123456.125000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-123456              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-123456              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-123456.125          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-123456.125000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-123456.125000000    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[-123456]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[-123456.125]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[-123456      ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[-123456      ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[-123456.125  ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[-123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-123456.125000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-123456              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-123456              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-123456.125          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-123456.125000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-123456.125000000    ]");
        int widths[3] = {5, 13, 21}, precs[3] = {3, 6, 9};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -123456.125000)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-3.625000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[       -3]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[       -3]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[   -3.625]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       -3.625000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              -3]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              -3]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          -3.625]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       -3.625000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[      -3.6250000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[-00000003]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[-00000003]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[-0003.625]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-00000003.625000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-000000000000003]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-000000000000003]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-00000000003.625]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-00000003.625000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000003.6250000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[-3       ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[-3       ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[-3.625   ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-3.625000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-3              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-3              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-3.625          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-3.625000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-3.6250000      ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[-3       ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[-3       ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[-3.625   ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-3.625000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-3              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-3              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-3.625          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-3.625000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-3.6250000      ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[-3]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[-3.625]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[-3       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[-3       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[-3.625   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[-3.6250000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-3.625000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-3              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-3              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-3.625          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-3.625000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-3.6250000      ]");
        int widths[3] = {1, 9, 16}, precs[3] = {3, 6, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -3.625000)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-1.000000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[       -1]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[       -1]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[   -1.000]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       -1.000000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              -1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              -1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          -1.000]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       -1.000000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[      -1.0000000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[-00000001]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[-00000001]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[-0001.000]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-00000001.000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-000000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-000000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-00000000001.000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-00000001.000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000001.0000000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[-1       ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[-1       ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[-1.000   ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-1.000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-1              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-1              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-1.000          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-1.000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-1.0000000      ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[-1       ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[-1       ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[-1.000   ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-1.000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-1              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-1              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-1.000          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-1.000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-1.0000000      ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[-1.000]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[-1       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[-1       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[-1.000   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[-1.0000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-1.000000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-1              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-1              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-1.000          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-1.000000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-1.0000000      ]");
        int widths[3] = {1, 9, 16}, precs[3] = {3, 6, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -1.0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0.000000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[       0]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[       0]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[   0.000]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       0.000000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          0.000]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       0.000000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[      0.0000000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[00000000]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[00000000]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[0000.000]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000.000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000.000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000.000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000.0000000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[0       ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[0       ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[0.000   ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0.000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0.000          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0.000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0.0000000      ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[0       ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[0       ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[0.000   ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[0.000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[0              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[0              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[0.000          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[0.000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[0.0000000      ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[0.000]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[0       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[0       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[0.000   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[0.0000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[0.000000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[0              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[0              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[0.000          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[0.000000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[0.0000000      ]");
        int widths[3] = {1, 8, 15}, precs[3] = {3, 6, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1.000000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[       1]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[       1]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[   1.000]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       1.000000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          1.000]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       1.000000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[      1.0000000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[00000001]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[00000001]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[0001.000]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000001.000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000001.000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000001.000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000001.0000000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[1       ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[1       ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[1.000   ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1.000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1.000          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1.000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1.0000000      ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[1       ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[1       ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[1.000   ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1.000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1.000          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1.000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1.0000000      ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[1.000]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[1       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[1       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[1.000   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[1.0000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1.000000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1.000          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1.000000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1.0000000      ]");
        int widths[3] = {1, 8, 15}, precs[3] = {3, 6, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 1.0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[3.625000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[       3]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[       3]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[   3.625]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       3.625000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              3]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              3]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          3.625]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       3.625000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[      3.6250000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[00000003]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[00000003]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[0003.625]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000003.625000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000003]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000003]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000003.625]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000003.625000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000003.6250000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[3       ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[3       ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[3.625   ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[3.625000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[3              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[3              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[3.625          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[3.625000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[3.6250000      ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[3       ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[3       ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[3.625   ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[3.625000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[3              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[3              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[3.625          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[3.625000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[3.6250000      ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[3]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[3.625]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[3       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[3       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[3.625   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[3.6250000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[3.625000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[3              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[3              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[3.625          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[3.625000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[3.6250000      ]");
        int widths[3] = {1, 8, 15}, precs[3] = {3, 6, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 3.625000)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[123456.125000]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_None_None_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_None_Zero_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_None_Mini_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[      123456]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[      123456]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[  123456.125]");
        strcopy(expecteds[Exp_None_Midi_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       123456.125000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              123456]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              123456]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          123456.125]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       123456.125000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    123456.125000000]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Zero_None_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Zero_Zero_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Zero_Mini_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[000000123456]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[000000123456]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[00123456.125]");
        strcopy(expecteds[Exp_Zero_Midi_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000123456.125000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000000123456]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000000123456]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000123456.125]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000123456.125000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000123456.125000000]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Dash_None_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Dash_Zero_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Dash_Mini_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[123456      ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[123456      ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[123456.125  ]");
        strcopy(expecteds[Exp_Dash_Midi_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[123456.125000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[123456              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[123456              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[123456.125          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[123456.125000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[123456.125000000    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Daro_None_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Daro_Zero_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Daro_Mini_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[123456      ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[123456      ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[123456.125  ]");
        strcopy(expecteds[Exp_Daro_Midi_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[123456.125000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[123456              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[123456              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[123456.125          ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[123456.125000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[123456.125000000    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Zesh_None_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Zesh_Zero_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[123456]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[123456.125]");
        strcopy(expecteds[Exp_Zesh_Mini_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[123456      ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[123456      ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[123456.125  ]");
        strcopy(expecteds[Exp_Zesh_Midi_Maxi], sizeof(expecteds[]), "[123456.125000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[123456.125000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[123456              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[123456              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[123456.125          ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[123456.125000       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[123456.125000000    ]");
        int widths[3] = {5, 12, 20}, precs[3] = {3, 6, 9};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 123456.125000)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[NaN]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[  N]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[  NaN]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    N]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[  NaN]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[  NaN]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[  N]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[  NaN]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[    N]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[  NaN]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[  NaN]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[N  ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[N    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[N  ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[N    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[N]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[N  ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[N    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[NaN  ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[NaN  ]");
        int widths[3] = {1, 3, 5}, precs[3] = {1, 3, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0.0 / 0.0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[Inf]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[ In]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    Inf]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[     In]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    Inf]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    Inf]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[ In]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[    Inf]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[     In]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[    Inf]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[    Inf]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[In ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[In     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[In ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[In     ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[ ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[In]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[In ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[In     ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[Inf    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[Inf    ]");
        int widths[3] = {1, 3, 7}, precs[3] = {2, 3, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 1.0 / 0.0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-Inf]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[ -In]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    -Inf]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[     -In]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    -Inf]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    -Inf]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[ -In]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[    -Inf]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[     -In]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[    -Inf]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[    -Inf]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[-In ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-In     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[-In ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-In     ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[-In]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[    ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[-In ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[        ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-In     ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-Inf    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-Inf    ]");
        int widths[3] = {3, 4, 8}, precs[3] = {3, 4, 7};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, -1.0 / 0.0)
    }

    // Random
    for (int i = 0; i < RANDOM_ITERATIONS; ++i)
    {
        float value = GetRandomFloat(-99999.9999999, 99999.9999999);
        char expected[16];
        FloatToString(value, expected, sizeof(expected));

        ASSERT_FMT1(expected, specifier, value)
    }
}


void TestSpecial()
{
    // 先获取字符串，后复用 AddString
    SetTestContext("Format Special");

    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        char specifier[] = "N";
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[Console]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[    Con]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    Console]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[        Con]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    Console]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    Console]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[    Con]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[    Console]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[        Con]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[    Console]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[    Console]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[Con    ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[Con        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[Con    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[Con        ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[Con]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[       ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[Con    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[           ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[Con        ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[Console    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[Console    ]");
        int widths[3] = {3, 7, 11}, precs[3] = {3, 7, 11};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0)
    }
    {
        char specifier[] = "L";
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[Console<0><Console><Console>]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[                  Console<0>]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    Console<0><Console><Console>]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                      Console<0>]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    Console<0><Console><Console>]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    Console<0><Console><Console>]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[    Console<0><Console><Console>]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[                  Console<0>]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[                      Console<0>]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[    Console<0><Console><Console>]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[    Console<0><Console><Console>]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[Console<0>                  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[Console<0>                      ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[Console<0>                  ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[Console<0>                      ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[Console<0>]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[                            ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[Console<0>                  ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[                                ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[Console<0>                      ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[Console<0><Console><Console>    ]");
        int widths[3] = {10, 28, 32}, precs[3] = {10, 28, 32};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0)
    }
    {
        char specifier[] = "E";
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[worldspawn]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[       wor]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    worldspawn]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[           wor]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    worldspawn]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    worldspawn]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[       wor]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[    worldspawn]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[           wor]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[    worldspawn]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[    worldspawn]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[wor       ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[wor           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[wor       ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[wor           ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[   ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[wor]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[          ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[wor       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[              ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[wor           ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[worldspawn    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[worldspawn    ]");
        int widths[3] = {3, 10, 14}, precs[3] = {3, 10, 11};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, 0)
    }

    // Random
#if RANDOM_ITERATIONS > 0
    int counter = 0;
    for (int i = 1; i <= MaxClients && counter < RANDOM_ITERATIONS; ++i)
    {
        if (IsValidClient(i))
        {
            char name[MAX_NAME_LENGTH];
            GetClientName(i, name, sizeof(name));
            ASSERT_FMT1(name, "N", i)

            int userid = GetClientUserId(i);

            char auth[MAX_AUTHID_LENGTH];
            GetClientAuthId(i, AuthId_Engine, auth, sizeof(auth));

            char expected[EXPECTED_MAX_LENGTH];
            FormatEx(expected, sizeof(expected), "%s<%d><%s><>", name, userid, auth);
            ASSERT_FMT1(expected, "L", i)
            ++counter;
        }
    }

    int entity = -1;
    while ((entity = FindEntityByClassname(entity, "*")) != -1 && counter++ < RANDOM_ITERATIONS)
    {
        char expected[EXPECTED_MAX_LENGTH];
        GetEntityClassname(entity, expected, sizeof(expected));

        ASSERT_FMT1(expected, "E", entity)
    }
#endif
}


void TestString()
{
    // 永远填充 ' '
    SetTestContext("Format String");
    char specifier[] = "s";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[Hello World!]");
        strcopy(expecteds[Exp_None_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_None_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    Hello World!]");
        strcopy(expecteds[Exp_None_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_None_Zero_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_None_Mini_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_None_Mini_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_None_Mini_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_None_Midi_Dot],  sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_None_Midi_Zero], sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_None_Midi_Mini], sizeof(expecteds[]), "[       Hello]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[           Hello]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    Hello World!]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    Hello World!]");
        strcopy(expecteds[Exp_Zero_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_None_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Zero_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zero_Zero_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Zero_Mini_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zero_Mini_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zero_Mini_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Zero_Midi_Dot],  sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Zero_Midi_Zero], sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Zero_Midi_Mini], sizeof(expecteds[]), "[       Hello]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[    Hello World!]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[           Hello]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[    Hello World!]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[    Hello World!]");
        strcopy(expecteds[Exp_Dash_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_None_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Dash_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Dash_Zero_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Dash_Mini_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Dash_Mini_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Dash_Mini_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Dash_Midi_Dot],  sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Dash_Midi_Zero], sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Dash_Midi_Mini], sizeof(expecteds[]), "[Hello       ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[Hello           ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Daro_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_None_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Daro_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Daro_Zero_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Daro_Mini_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Daro_Mini_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Daro_Mini_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Daro_Midi_Dot],  sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Daro_Midi_Zero], sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Daro_Midi_Mini], sizeof(expecteds[]), "[Hello       ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[Hello           ]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Zesh_None_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_None_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Zesh_Zero_Dot],  sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Zero], sizeof(expecteds[]), "[]");
        strcopy(expecteds[Exp_Zesh_Zero_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Zesh_Mini_Dot],  sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zesh_Mini_Zero], sizeof(expecteds[]), "[     ]");
        strcopy(expecteds[Exp_Zesh_Mini_Mini], sizeof(expecteds[]), "[Hello]");
        strcopy(expecteds[Exp_Zesh_Midi_Dot],  sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Zesh_Midi_Zero], sizeof(expecteds[]), "[            ]");
        strcopy(expecteds[Exp_Zesh_Midi_Mini], sizeof(expecteds[]), "[Hello       ]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[                ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[Hello           ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[Hello World!    ]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[Hello World!    ]");
        int widths[3] = {5, 12, 16}, precs[3] = {5, 12, 16};
        ASSERT_FMTS1(expecteds, widths, precs, specifier, "Hello World!")
    }

    // Special
    ASSERT_FMT1("", specifier, "")
    ASSERT_FMT1("", specifier, NULL_STRING)

    // Large
    {
        char expected[EXPECTED_MAX_LENGTH] = "Str...";
        for (int i = 6; i < sizeof(expected) - 1; ++i)
            expected[i] = ' ';
        expected[sizeof(expected) - 1] = '\0';

        char fmt[32];
        FormatEx(fmt, sizeof(fmt), "-%d%s", EXPECTED_MAX_LENGTH - 1, specifier);
        ASSERT_FMT1(expected, fmt, "Str...")
    }

    // Random
    {
        char buffer[EXPECTED_MAX_LENGTH];
        for (int i = 0; i < RANDOM_ITERATIONS; ++i)
        {
            int length = GetRandomInt(0, sizeof(buffer) - 1);
            for (int j = 0; j < length; ++j)
                buffer[j] = view_as<char>(GetRandomInt(32, 126));
            buffer[length] = '\0';

            ASSERT_FMT1(buffer, specifier, buffer)
        }
    }
}


void TestTranslates()
{
    LoadTranslations("common.phrases");

    // 内部是先获取 memory_buf 字符串，然后直接 append, 不会处理高级格式化
    // %T, LANGID
    SetTestContext("Format Translates");

    // Ensure that the global language target is English
    // otherwise it may deviate from the expected value
    AssertEq("Server Language", GetServerLanguage(), GetLanguageByCode("en"));

    char specifier[2][] = {"t", "T"};
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[You cannot target this player.]");
        int widths[3] = {1, 3, 5}, precs[3] = {5, 7, 9};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], "Unable to target")
        ASSERT_FMTS2(expecteds, widths, precs, specifier[1], "Unable to target", LANG_SERVER)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[(ADMINS) Console]");
        int widths[3] = {1, 3, 5}, precs[3] = {5, 7, 9};
        ASSERT_FMTS2(expecteds, widths, precs, specifier[0], "Chat admins", 0)
        ASSERT_FMTS3(expecteds, widths, precs, specifier[1], "Chat admins", LANG_SERVER, 0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[Unable to find cvar: my_cvar]");
        int widths[3] = {1, 3, 5}, precs[3] = {5, 7, 9};
        ASSERT_FMTS2(expecteds, widths, precs, specifier[0], "Unable to find cvar", "my_cvar")
        ASSERT_FMTS3(expecteds, widths, precs, specifier[1], "Unable to find cvar", LANG_SERVER, "my_cvar")
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[player1 has chosen option2.]");
        int widths[3] = {1, 3, 5}, precs[3] = {5, 7, 9};
        ASSERT_FMTS3(expecteds, widths, precs, specifier[0], "Vote Select", "player1", "option2")
        ASSERT_FMTS4(expecteds, widths, precs, specifier[1], "Vote Select", LANG_SERVER, "player1", "option2")
    }
}


void TestHex()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Hexadecimal");

    char specifier[2][] = {"X", "x"};
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        int widths[3] = {0, 1, 2}, precs[3] = {3, 4, 5};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 0)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 0)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        int widths[3] = {0, 1, 13}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 1)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[12D687]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[12D687000000000000]");
        int widths[3] = {0, 6, 18}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[12d687]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[12d6870000000000000]");
        int widths[3] = {1, 6, 19}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFF000]");
        int widths[3] = {7, 8, 11}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 2147483647)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[7fffffff000]");
        int widths[3] = {7, 8, 11}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 2147483647)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[80000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   80000000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   80000000]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   80000000]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   80000000]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   80000000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   80000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00080000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00080000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00080000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00080000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00080000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00080000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[80000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[80000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[80000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[80000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[80000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[80000000   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[80000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[80000000000]");
        int widths[3] = {7, 8, 11}, precs[3] = {5, 9, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], -2147483648)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], -2147483648)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[FFED2979]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            FFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            FFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            FFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            FFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            FFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            FFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000FFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000FFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000FFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000FFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000FFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000FFED2979]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[FFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[FFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[FFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[FFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[FFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[FFED2979            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[FFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[FFED2979000000000000]");
        int widths[3] = {0, 8, 20}, precs[3] = {1, 8, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], -1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[ffed2979]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            ffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            ffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            ffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            ffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            ffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            ffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000ffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000ffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000ffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000ffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000ffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000ffed2979]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[ffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[ffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[ffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[ffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[ffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[ffed2979            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[ffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[ffed2979000000000000]");
        int widths[3] = {0, 8, 20}, precs[3] = {1, 8, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], -1234567)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[FFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    FFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    FFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    FFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    FFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    FFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    FFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000FFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000FFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000FFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000FFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000FFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000FFFFFFFF]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[FFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFF    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFF0000]");
        int widths[3] = {7, 8, 12}, precs[3] = {1, 8, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], 0xFFFFFFFF)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[ffffffff]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    ffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    ffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    ffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    ffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    ffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    ffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000ffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000ffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000ffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000ffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000ffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000ffffffff]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[ffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[ffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[ffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[ffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[ffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[ffffffff    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[ffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[ffffffff0000]");
        int widths[3] = {7, 8, 12}, precs[3] = {1, 8, 21};
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], -1)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], 0xFFFFFFFF)
    }
}


#if defined SM_INT64_SUPPORTED
void TestBinary64()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Binary64");

    char specifier[] = "lb";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[00000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[00000]");
        int widths[3] = {0, 1, 5}, precs[3] = {5, 7, 9};
        int64 value = 0;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[  100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[  100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[  100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[  100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[  100101101011010000111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[  100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00100101101011010000111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00100101101011010000111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[100101101011010000111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[100101101011010000111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[100101101011010000111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[100101101011010000111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[100101101011010000111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[100101101011010000111  ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[10010110101101000011100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[10010110101101000011100]");
        int widths[3] = {17, 21, 23}, precs[3] = {18, 32, 36};
        int64 value = 1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[  11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[  11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[  11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[  11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[  11111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[  11111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0011111111111111111111111111111111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[11111111111111111111111111111111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[11111111111111111111111111111111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[11111111111111111111111111111111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[11111111111111111111111111111111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[11111111111111111111111111111111  ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[11111111111111111111111111111111  ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111100]");
        int widths[3] = {17, 32, 34}, precs[3] = {18, 32, 36};
        int64 value = 4294967295;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[       10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[       10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[       10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[       10001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000010001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000010001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000010001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000010001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000010001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000010001111101110001111101110110101110110001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[10001111101110001111101110110101110110001       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[10001111101110001111101110110101110110001       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[10001111101110001111101110110101110110001       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[10001111101110001111101110110101110110001       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[10001111101110001111101110110101110110001       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[10001111101110001111101110110101110110001       ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[100011111011100011111011101101011101100010000000]");
        int widths[3] = {32, 41, 48}, precs[3] = {18, 32, 36};
        int64 value = 1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[       111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[       111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[       111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[       111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111       ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111110000000]");
        int widths[3] = {32, 63, 70}, precs[3] = {18, 32, 36};
        int64 value = 9223372036854775807;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[       1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[       1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[       1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[       1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[       1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[       1000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000001000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000001000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000001000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000001000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000001000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000001000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000       ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000000000000000000000000000000000000000000000000000000       ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[10000000000000000000000000000000000000000000000000000000000000000000000]");
        int widths[3] = {32, 64, 71}, precs[3] = {18, 32, 36};
        int64 value = -9223372036854775807;
        value -= 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[        1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[        1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[        1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[        1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[        1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[        1111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000001111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000001111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000001111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000001111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000001111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000001111111111111111111111111111111111111111111111111111111111111111]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111        ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1111111111111111111111111111111111111111111111111111111111111111        ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[111111111111111111111111111111111111111111111111111111111111111100000000]");
        int widths[3] = {32, 64, 72}, precs[3] = {18, 32, 36};
        int64 value = -1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
}


void TestInt64()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Integer64");

    char specifier[2][] = {"ld", "li"};
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[          -9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[          -9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[          -9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          -9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[          -9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[          -9223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-00000000009223372036854775808]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-9223372036854775808          ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-92233720368547758080000000000]");
        int widths[3] = {7, 20, 30}, precs[3] = {5, 9, 21};
        int64 value = -9223372036854775807;
        value -= 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                 -1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                 -1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                 -1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                 -1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                 -1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                 -1234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-000000000000000001234567654321]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-1234567654321                 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-123456765432100000000000000000]");
        int widths[3] = {7, 14, 31}, precs[3] = {5, 9, 21};
        int64 value = -1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-4294967295]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            -4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            -4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            -4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            -4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            -4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            -4294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000000000004294967295]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-4294967295            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-4294967295000000000000]");
        int widths[3] = {7, 11, 23}, precs[3] = {5, 9, 21};
        int64 value = -4294967295;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-1234567]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            -1234567]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000000000001234567]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-1234567            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-1234567000000000000]");
        int widths[3] = {7, 8, 20}, precs[3] = {5, 9, 21};
        int64 value = -1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[-1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            -1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            -1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            -1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            -1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            -1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            -1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[-0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[-0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[-0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[-0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[-0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[-0000000000001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[-1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[-1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[-1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[-1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[-1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[-1            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[-1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[-1000000000000]");
        int widths[3] = {1, 2, 14}, precs[3] = {5, 9, 21};
        int64 value = -1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[000000]");
        int widths[3] = {0, 1, 6}, precs[3] = {5, 7, 9};
        int64 value = 0;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        int widths[3] = {0, 1, 13}, precs[3] = {5, 9, 21};
        int64 value = 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1234567]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        int widths[3] = {3, 7, 19}, precs[3] = {5, 9, 21};
        int64 value = 1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[4294967295]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[4294967295000000000000]");
        int widths[3] = {7, 10, 22}, precs[3] = {5, 9, 21};
        int64 value = 4294967295;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        int widths[3] = {7, 13, 30}, precs[3] = {5, 9, 21};
        int64 value = 1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        int widths[3] = {7, 19, 29}, precs[3] = {5, 9, 21};
        int64 value = 9223372036854775807;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }

    // Random
    for (int i = 0; i < RANDOM_ITERATIONS; ++i)
    {
        int value = GetRandomInt(-123456789, 123456789);
        char expected[12];
        IntToString(value, expected, sizeof(expected));

        int64 value64 = value;
        ASSERT_FMT1(expected, specifier[i & 1], value64)
    }
}


void TestUInt64()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Unsigned Integer64");

    char specifier[] = "lu";
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[          9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[          9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[          9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[          9223372036854775808]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[          9223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000009223372036854775808]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000009223372036854775808]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[9223372036854775808          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[9223372036854775808          ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[92233720368547758080000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[92233720368547758080000000000]");
        int widths[3] = {7, 19, 29}, precs[3] = {5, 9, 21};
        int64 value = -9223372036854775807;
        value -= 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[18446742839141897295]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                 18446742839141897295]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                 18446742839141897295]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                 18446742839141897295]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                 18446742839141897295]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                 18446742839141897295]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                 18446742839141897295]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000000000018446742839141897295]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000000000018446742839141897295]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000000000018446742839141897295]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000000000018446742839141897295]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000000000018446742839141897295]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000000000018446742839141897295]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[18446742839141897295                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[18446742839141897295                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[18446742839141897295                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[18446742839141897295                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[18446742839141897295                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[18446742839141897295                 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1844674283914189729500000000000000000]");
        int widths[3] = {7, 20, 37}, precs[3] = {5, 9, 21};
        int64 value = -1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[18446744069414584321]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            18446744069414584321]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            18446744069414584321]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            18446744069414584321]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            18446744069414584321]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            18446744069414584321]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            18446744069414584321]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000000018446744069414584321]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000018446744069414584321]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000018446744069414584321]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000018446744069414584321]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000000018446744069414584321]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000000018446744069414584321]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[18446744069414584321            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[18446744069414584321            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[18446744069414584321            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[18446744069414584321            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[18446744069414584321            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[18446744069414584321            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[18446744069414584321000000000000]");
        int widths[3] = {7, 20, 32}, precs[3] = {5, 9, 21};
        int64 value = -4294967295;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[18446744073708317049]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            18446744073708317049]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            18446744073708317049]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            18446744073708317049]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            18446744073708317049]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            18446744073708317049]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            18446744073708317049]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000000018446744073708317049]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000018446744073708317049]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000018446744073708317049]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000018446744073708317049]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000000018446744073708317049]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000000018446744073708317049]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[18446744073708317049            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[18446744073708317049            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[18446744073708317049            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[18446744073708317049            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[18446744073708317049            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[18446744073708317049            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[18446744073708317049000000000000]");
        int widths[3] = {7, 20, 32}, precs[3] = {5, 9, 21};
        int64 value = -1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[18446744073709551615]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            18446744073709551615]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            18446744073709551615]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            18446744073709551615]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            18446744073709551615]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            18446744073709551615]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            18446744073709551615]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000000018446744073709551615]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000018446744073709551615]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000018446744073709551615]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000018446744073709551615]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000000018446744073709551615]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000000018446744073709551615]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[18446744073709551615            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[18446744073709551615            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[18446744073709551615            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[18446744073709551615            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[18446744073709551615            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[18446744073709551615            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[18446744073709551615000000000000]");
        int widths[3] = {1, 20, 32}, precs[3] = {5, 9, 21};
        int64 value = -1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[     0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0     ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[000000]");
        int widths[3] = {0, 1, 6}, precs[3] = {5, 7, 9};
        int64 value = 0;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        int widths[3] = {0, 1, 13}, precs[3] = {5, 9, 21};
        int64 value = 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1234567]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1234567]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001234567]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1234567            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1234567000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1234567000000000000]");
        int widths[3] = {3, 7, 19}, precs[3] = {5, 9, 21};
        int64 value = 1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[4294967295]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            4294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000004294967295]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[4294967295            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[4294967295000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[4294967295000000000000]");
        int widths[3] = {7, 10, 22}, precs[3] = {5, 9, 21};
        int64 value = 4294967295;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                 1234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000000001234567654321]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1234567654321                 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[123456765432100000000000000000]");
        int widths[3] = {7, 13, 30}, precs[3] = {5, 9, 21};
        int64 value = 1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[          9223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000009223372036854775807]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[9223372036854775807          ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[92233720368547758070000000000]");
        int widths[3] = {7, 19, 29}, precs[3] = {5, 9, 21};
        int64 value = 9223372036854775807;
        ASSERT_FMTS1(expecteds, widths, precs, specifier, value)
    }

    // Random
    for (int i = 0; i < RANDOM_ITERATIONS; ++i)
    {
        int value = GetRandomInt(0, 123456789);
        char expected[12];
        IntToString(value, expected, sizeof(expected));

        int64 value64 = value;
        ASSERT_FMT1(expected, specifier, value64)
    }
}


void TestHex64()
{
    // prec 没有传递给目标函数
    // 尾部填充 '0' 不会被替换为 ' '
    SetTestContext("Format Hexadecimal64");

    char specifier[2][] = {"lX", "lx"};
    char expecteds[Exp_All][EXPECTED_MAX_LENGTH];
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[0]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[ 0]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[0 ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[00]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[00]");
        int widths[3] = {0, 1, 2}, precs[3] = {3, 4, 5};
        int64 value = 0;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000001]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[1            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[1000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[1000000000000]");
        int widths[3] = {0, 1, 13}, precs[3] = {5, 9, 21};
        int64 value = 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[12D687]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            12D687]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[00000000000012D687]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[12D687            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[12D687000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[12D687000000000000]");
        int widths[3] = {0, 6, 18}, precs[3] = {5, 9, 21};
        int64 value = 1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[12d687]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[             12d687]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000012d687]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[12d687             ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[12d6870000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[12d6870000000000000]");
        int widths[3] = {1, 6, 19}, precs[3] = {5, 9, 21};
        int64 value = 1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[11F71F76BB1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                11F71F76BB1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                11F71F76BB1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                11F71F76BB1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                11F71F76BB1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                11F71F76BB1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                11F71F76BB1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000000011F71F76BB1]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000000011F71F76BB1]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000000011F71F76BB1]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000000011F71F76BB1]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000000011F71F76BB1]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000000011F71F76BB1]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[11F71F76BB1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[11F71F76BB1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[11F71F76BB1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[11F71F76BB1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[11F71F76BB1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[11F71F76BB1                ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[11F71F76BB10000000000000000]");
        int widths[3] = {7, 11, 27}, precs[3] = {5, 9, 21};
        int64 value = 1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[11f71f76bb1]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                11f71f76bb1]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                11f71f76bb1]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                11f71f76bb1]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                11f71f76bb1]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                11f71f76bb1]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                11f71f76bb1]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000000011f71f76bb1]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000000011f71f76bb1]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000000011f71f76bb1]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000000011f71f76bb1]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000000011f71f76bb1]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000000011f71f76bb1]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[11f71f76bb1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[11f71f76bb1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[11f71f76bb1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[11f71f76bb1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[11f71f76bb1                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[11f71f76bb1                ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[11f71f76bb10000000000000000]");
        int widths[3] = {7, 11, 27}, precs[3] = {5, 9, 21};
        int64 value = 1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   7FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0007FFFFFFF]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFF   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFF000]");
        int widths[3] = {7, 8, 11}, precs[3] = {5, 9, 21};
        int64 value = 2147483647;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   7fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0007fffffff]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[7fffffff   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[7fffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[7fffffff000]");
        int widths[3] = {7, 8, 11}, precs[3] = {5, 9, 21};
        int64 value = 2147483647;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   7FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0007FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0007FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0007FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0007FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0007FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0007FFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[7FFFFFFFFFFFFFFF000]");
        int widths[3] = {7, 16, 19}, precs[3] = {5, 9, 21};
        int64 value = 9223372036854775807;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[7fffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   7fffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   7fffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   7fffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   7fffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   7fffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   7fffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0007fffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0007fffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0007fffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0007fffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0007fffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0007fffffffffffffff]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[7fffffffffffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[7fffffffffffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[7fffffffffffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[7fffffffffffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[7fffffffffffffff   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[7fffffffffffffff   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[7fffffffffffffff000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[7fffffffffffffff000]");
        int widths[3] = {7, 16, 19}, precs[3] = {5, 9, 21};
        int64 value = 9223372036854775807;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[8000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[   8000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[   8000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[   8000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[   8000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[   8000000000000000]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[   8000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0008000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0008000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0008000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0008000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0008000000000000000]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0008000000000000000]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[8000000000000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[8000000000000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[8000000000000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[8000000000000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[8000000000000000   ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[8000000000000000   ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[8000000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[8000000000000000000]");
        int widths[3] = {7, 16, 19}, precs[3] = {5, 9, 21};
        int64 value = -9223372036854775807;
        value -= 1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000FFFFFFFFFFED2979]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[FFFFFFFFFFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFFFFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFFFFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFFFFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFFFFED2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFFFFED2979            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFFFFED2979000000000000]");
        int widths[3] = {0, 16, 28}, precs[3] = {1, 8, 21};
        int64 value = -1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[ffffffffffed2979]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[            ffffffffffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[            ffffffffffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[            ffffffffffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[            ffffffffffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[            ffffffffffed2979]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[            ffffffffffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[000000000000ffffffffffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[000000000000ffffffffffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[000000000000ffffffffffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[000000000000ffffffffffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[000000000000ffffffffffed2979]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[000000000000ffffffffffed2979]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[ffffffffffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[ffffffffffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[ffffffffffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[ffffffffffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[ffffffffffed2979            ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[ffffffffffed2979            ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[ffffffffffed2979000000000000]");
        int widths[3] = {0, 16, 28}, precs[3] = {1, 8, 21};
        int64 value = -1234567;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000000000FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000000000FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000000000FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000000000FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000000000FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000000000FFFFFEE08E08944F]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[FFFFFEE08E08944F                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFEE08E08944F                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[FFFFFEE08E08944F                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[FFFFFEE08E08944F                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[FFFFFEE08E08944F                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFEE08E08944F                ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFEE08E08944F0000000000000000]");
        int widths[3] = {7, 16, 32}, precs[3] = {5, 9, 21};
        int64 value = -1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[fffffee08e08944f]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[                fffffee08e08944f]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[                fffffee08e08944f]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[                fffffee08e08944f]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[                fffffee08e08944f]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[                fffffee08e08944f]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[                fffffee08e08944f]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000000000000000fffffee08e08944f]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000000000000000fffffee08e08944f]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000000000000000fffffee08e08944f]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000000000000000fffffee08e08944f]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000000000000000fffffee08e08944f]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000000000000000fffffee08e08944f]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[fffffee08e08944f                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[fffffee08e08944f                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[fffffee08e08944f                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[fffffee08e08944f                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[fffffee08e08944f                ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[fffffee08e08944f                ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[fffffee08e08944f0000000000000000]");
        int widths[3] = {7, 16, 32}, precs[3] = {5, 9, 21};
        int64 value = -1234567654321;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000FFFFFFFFFFFFFFFF]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[FFFFFFFFFFFFFFFF0000]");
        int widths[3] = {7, 16, 20}, precs[3] = {1, 8, 21};
        int64 value = -1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[0], value)
    }
    {
        FillStrings(expecteds, sizeof(expecteds), sizeof(expecteds[]), "[ffffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_None], sizeof(expecteds[]), "[    ffffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Dot],  sizeof(expecteds[]), "[    ffffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Zero], sizeof(expecteds[]), "[    ffffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Mini], sizeof(expecteds[]), "[    ffffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Midi], sizeof(expecteds[]), "[    ffffffffffffffff]");
        strcopy(expecteds[Exp_None_Maxi_Maxi], sizeof(expecteds[]), "[    ffffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_None], sizeof(expecteds[]), "[0000ffffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Dot],  sizeof(expecteds[]), "[0000ffffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Zero], sizeof(expecteds[]), "[0000ffffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Mini], sizeof(expecteds[]), "[0000ffffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Midi], sizeof(expecteds[]), "[0000ffffffffffffffff]");
        strcopy(expecteds[Exp_Zero_Maxi_Maxi], sizeof(expecteds[]), "[0000ffffffffffffffff]");
        strcopy(expecteds[Exp_Dash_Maxi_None], sizeof(expecteds[]), "[ffffffffffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Dot],  sizeof(expecteds[]), "[ffffffffffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Zero], sizeof(expecteds[]), "[ffffffffffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Mini], sizeof(expecteds[]), "[ffffffffffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Midi], sizeof(expecteds[]), "[ffffffffffffffff    ]");
        strcopy(expecteds[Exp_Dash_Maxi_Maxi], sizeof(expecteds[]), "[ffffffffffffffff    ]");
        strcopy(expecteds[Exp_Daro_Maxi_None], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Dot],  sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Zero], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Mini], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Midi], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Daro_Maxi_Maxi], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_None], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Dot],  sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Zero], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Mini], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Midi], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        strcopy(expecteds[Exp_Zesh_Maxi_Maxi], sizeof(expecteds[]), "[ffffffffffffffff0000]");
        int widths[3] = {7, 16, 20}, precs[3] = {1, 8, 21};
        int64 value = -1;
        ASSERT_FMTS1(expecteds, widths, precs, specifier[1], value)
    }
}
#endif



stock bool IsValidClient(int client)
{
    return client > 0 && client <= MaxClients && IsClientInGame(client);
}

stock void FillStrings(char[][] buffer, int maxlen, int maxStrings, const char[] filler)
{
    for (int i = 0; i < maxlen; ++i)
        strcopy(buffer[i], maxStrings, filler);
}
