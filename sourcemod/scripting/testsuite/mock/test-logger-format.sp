#pragma semicolon 1
#pragma newdecls required

/**
 * Fix: "Not enough space on the heap"
 * 似乎是由于 TestSink.GetLastLogMsg 的调用堆栈过深
 */
#pragma dynamic 131072

#include <sourcemod>
#include <testing>
#include <log4sp>

#include "../test_sink"

/**
 * Full syntax
 *      %[flags][width][.precision]specifier
 * flags
 *      [-] / [0]
 * NOTE
 *      SM 1.13.0.7198 修复了左对齐溢出的 BUG
 *      SM 的 %s 总是左对齐 (pr-2332 暂未合并)
 *      SM 的 %0[width]d 在传递负数时, 负号会添加在填充符 0 之后 '-1' --> '000-1' (pr-2329 暂未合并)
 *      SM 的 %f 在传递 inf 值时，不会格式化为 "Inf" (pr-2324 暂未合并)
 *      二者 Float 类型左对齐时都只会在后方添加 ' ' (不会添加 '0')
 *        这应该是 SM 刻意这么做的 (commit - fcb362da09dc3e0e11e03d11866b574b19419648)
 */

#define LOGGER_NAME             "test-format"

// 不要忘记检查期望值
#define TEST_STRING_TEXT        "Some String Text"

#define TEST_BINARY_VALUE1      "0000000000000000000000000000000"
#define TEST_BINARY_VALUE2      "1110111011101110111011101110111"
#define TEST_BINARY_VALUE3      "1111111111111111111111111111111"

#define TEST_UINT_VALUE1        "0"
#define TEST_UINT_VALUE2        "2147483647"
#define TEST_UINT_VALUE3        "4294967295"

#define TEST_INT_VALUE1         "0"
#define TEST_INT_VALUE2         "2147483647"
#define TEST_INT_VALUE3         "-2147483648"

#define TEST_HEX_UPPER_VALUE1   "0"
#define TEST_HEX_UPPER_VALUE2   "F1F88"
#define TEST_HEX_UPPER_VALUE3   "FFFFFFFF"

#define TEST_HEX_LOWER_VALUE1   "0"
#define TEST_HEX_LOWER_VALUE2   "f1f88"
#define TEST_HEX_LOWER_VALUE3   "ffffffff"

#define TEST_TRANSLATES_KEY     "Unable to target"
#define TEST_TRANSLATES_VALUE   "You cannot target this player."

#define TEST_TRANSLATES_DATA1   0
#define TEST_TRANSLATES_KEY1    "Chat admins"
#define TEST_TRANSLATES_VALUE1  "(ADMINS) Console"

#define TEST_TRANSLATES_DATA2   "77777777"
#define TEST_TRANSLATES_KEY2    "Vote Delay Seconds"
#define TEST_TRANSLATES_VALUE2  "You must wait " ... TEST_TRANSLATES_DATA2 ... " seconds before starting another vote."

#define TEST_TRANSLATES_DATA3_1 "player1"
#define TEST_TRANSLATES_DATA3_2 "option2"
#define TEST_TRANSLATES_KEY3    "Vote Select"
#define TEST_TRANSLATES_VALUE3  TEST_TRANSLATES_DATA3_1 ... " has chosen " ... TEST_TRANSLATES_DATA3_2 ... "."


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_format", Command_Test);

    LoadTranslations("common.phrases");
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST LOG ARGS FORMAT ----");

    TestChar();

    TestString();

    TestFloat();

    TestBinary();

    TestUInt();

    TestInt();

    TestHex();

    TestTranslates();

    TestSpecial();

#if SOURCEMOD_V_MINOR >= 13
    TestBinary64();

    TestInt64();

    TestUInt64();

    TestHex64();
#endif

    PrintToServer("---- STOP TEST LOG ARGS FORMAT ----");
    return Plugin_Handled;
}


void TestChar()
{
    // %c
    SetTestContext("Test Char");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoF("'%% %%- %%. %%0 %%7 %%07 %%F %%c %%d'");
    AssertStrEq("Ex", sink.DrainLastMsgFast().msg, "'% %- %. %0 %7 %07 %F %c %d'");

    logger.Close();
    sink.Close();
}


void TestString()
{
    // flags: [-]
    // %[flags][width]s
    SetTestContext("Test String");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoF("'%s'", NULL_STRING);
    AssertStrEq("%s", sink.DrainLastMsgFast().msg, "''");

    logger.InfoF("'%s'", TEST_STRING_TEXT);
    AssertStrEq("%s", sink.DrainLastMsgFast().msg, "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoF("'%-s'", TEST_STRING_TEXT);
    AssertStrEq("%-s", sink.DrainLastMsgFast().msg, "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoF("'%10s'", TEST_STRING_TEXT);
    AssertStrEq("%10s", sink.DrainLastMsgFast().msg, "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoF("'%20s'", TEST_STRING_TEXT);
    AssertStrEq("%20s", sink.DrainLastMsgFast().msg, "'    " ... TEST_STRING_TEXT ... "'");

    logger.InfoF("'%-10s'", TEST_STRING_TEXT);
    AssertStrEq("%-10s", sink.DrainLastMsgFast().msg, "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoF("'%-20s'", TEST_STRING_TEXT);
    AssertStrEq("%-20s", sink.DrainLastMsgFast().msg, "'" ... TEST_STRING_TEXT ... "    '");

    logger.Close();
    sink.Close();
}

void TestFloat()
{
    // flags: [-] / [0]
    // %[flags][width][.precision]f
    SetTestContext("Test Float");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoF("'%f'", 12345.968750);
    AssertStrEq("%f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%f'", 0.0);
    AssertStrEq("%f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%f'", -12345.968750);
    AssertStrEq("%f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%-f'", 12345.968750);
    AssertStrEq("%-f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%-f'", 0.0);
    AssertStrEq("%-f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%-f'", -12345.968750);
    AssertStrEq("%-f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%0f'", 12345.968750);
    AssertStrEq("%0f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%0f'", 0.0);
    AssertStrEq("%0f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%0f'", -12345.968750);
    AssertStrEq("%0f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%-0f'", 12345.968750);
    AssertStrEq("%-0f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%-0f'", 0.0);
    AssertStrEq("%-0f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%-0f'", -12345.968750);
    AssertStrEq("%-0f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%0-f'", 12345.968750);
    AssertStrEq("%0-f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%0-f'", 0.0);
    AssertStrEq("%0-f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%0-f'", -12345.968750);
    AssertStrEq("%0-f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    // width
    logger.InfoF("'%3f'", 12345.968750);
    AssertStrEq("%3f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%3f'", 0.0);
    AssertStrEq("%3f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%3f'", -12345.968750);
    AssertStrEq("%3f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%20f'", 12345.968750);
    AssertStrEq("%20f", sink.DrainLastMsgFast().msg, "'        12345.968750'");
    logger.InfoF("'%20f'", 0.0);
    AssertStrEq("%20f", sink.DrainLastMsgFast().msg, "'            0.000000'");
    logger.InfoF("'%20f'", -12345.968750);
    AssertStrEq("%20f", sink.DrainLastMsgFast().msg, "'       -12345.968750'");

    // flag & width
    logger.InfoF("'%-3f'", 12345.968750);
    AssertStrEq("%-3f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%-3f'", 0.0);
    AssertStrEq("%-3f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%-3f'", -12345.968750);
    AssertStrEq("%-3f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%-20f'", 12345.968750);
    AssertStrEq("%-20f", sink.DrainLastMsgFast().msg, "'12345.968750        '");
    logger.InfoF("'%-20f'", 0.0);
    AssertStrEq("%-20f", sink.DrainLastMsgFast().msg, "'0.000000            '");
    logger.InfoF("'%-20f'", -12345.968750);
    AssertStrEq("%-20f", sink.DrainLastMsgFast().msg, "'-12345.968750       '");

    logger.InfoF("'%03f'", 12345.968750);
    AssertStrEq("%03f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%03f'", 0.0);
    AssertStrEq("%03f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%03f'", -12345.968750);
    AssertStrEq("%03f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%020f'", 12345.968750);
    AssertStrEq("%020f", sink.DrainLastMsgFast().msg, "'0000000012345.968750'");
    logger.InfoF("'%020f'", 0.0);
    AssertStrEq("%020f", sink.DrainLastMsgFast().msg, "'0000000000000.000000'");
    logger.InfoF("'%020f'", -12345.968750);
    AssertStrEq("%020f", sink.DrainLastMsgFast().msg, "'-000000012345.968750'");

    logger.InfoF("'%-03f'", 12345.968750);
    AssertStrEq("%-03f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%-03f'", 0.0);
    AssertStrEq("%-03f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%-03f'", -12345.968750);
    AssertStrEq("%-03f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%-020f'", 12345.968750);
    AssertStrEq("%-020f", sink.DrainLastMsgFast().msg, "'12345.968750        '");
    logger.InfoF("'%-020f'", 0.0);
    AssertStrEq("%-020f", sink.DrainLastMsgFast().msg, "'0.000000            '");
    logger.InfoF("'%-020f'", -12345.968750);
    AssertStrEq("%-020f", sink.DrainLastMsgFast().msg, "'-12345.968750       '");

    logger.InfoF("'%0-3f'", 12345.968750);
    AssertStrEq("%0-3f", sink.DrainLastMsgFast().msg, "'12345.968750'");
    logger.InfoF("'%0-3f'", 0.0);
    AssertStrEq("%0-3f", sink.DrainLastMsgFast().msg, "'0.000000'");
    logger.InfoF("'%0-3f'", -12345.968750);
    AssertStrEq("%0-3f", sink.DrainLastMsgFast().msg, "'-12345.968750'");

    logger.InfoF("'%0-20f'", 12345.968750);
    AssertStrEq("%0-20f", sink.DrainLastMsgFast().msg, "'12345.968750        '");
    logger.InfoF("'%0-20f'", 0.0);
    AssertStrEq("%0-20f", sink.DrainLastMsgFast().msg, "'0.000000            '");
    logger.InfoF("'%0-20f'", -12345.968750);
    AssertStrEq("%0-20f", sink.DrainLastMsgFast().msg, "'-12345.968750       '");

    // prec
    logger.InfoF("'%.f'", 12345.968750);
    AssertStrEq("%.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%.f'", 0.0);
    AssertStrEq("%.f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%.f'", -12345.968750);
    AssertStrEq("%.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%.0f'", 12345.968750);
    AssertStrEq("%.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%.0f'", 0.0);
    AssertStrEq("%.0f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%.0f'", -12345.968750);
    AssertStrEq("%.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%.2f'", 12345.968750);
    AssertStrEq("%.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%.2f'", 0.0);
    AssertStrEq("%.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%.2f'", -12345.968750);
    AssertStrEq("%.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%.7f'", 12345.968750);
    AssertStrEq("%.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%.7f'", 0.0);
    AssertStrEq("%.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%.7f'", -12345.968750);
    AssertStrEq("%.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    // flag & prec
    logger.InfoF("'%-.f'", 12345.968750);
    AssertStrEq("%-.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-.f'", 0.0);
    AssertStrEq("%-.f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-.f'", -12345.968750);
    AssertStrEq("%-.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-.0f'", 12345.968750);
    AssertStrEq("%-.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-.0f'", 0.0);
    AssertStrEq("%-.0f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-.0f'", -12345.968750);
    AssertStrEq("%-.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-.2f'", 12345.968750);
    AssertStrEq("%-.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%-.2f'", 0.0);
    AssertStrEq("%-.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%-.2f'", -12345.968750);
    AssertStrEq("%-.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%-.7f'", 12345.968750);
    AssertStrEq("%-.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%-.7f'", 0.0);
    AssertStrEq("%-.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%-.7f'", -12345.968750);
    AssertStrEq("%-.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    logger.InfoF("'%0.f'", 12345.968750);
    AssertStrEq("%0.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%0.f'", 0.0);
    AssertStrEq("%0.f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0.f'", -12345.968750);
    AssertStrEq("%0.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%0.0f'", 12345.968750);
    AssertStrEq("%0.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%0.0f'", 0.0);
    AssertStrEq("%0.0f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0.0f'", -12345.968750);
    AssertStrEq("%0.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%0.2f'", 12345.968750);
    AssertStrEq("%0.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%0.2f'", 0.0);
    AssertStrEq("%0.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%0.2f'", -12345.968750);
    AssertStrEq("%0.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%0.7f'", 12345.968750);
    AssertStrEq("%0.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%0.7f'", 0.0);
    AssertStrEq("%0.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%0.7f'", -12345.968750);
    AssertStrEq("%0.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    logger.InfoF("'%0-.f'", 12345.968750);
    AssertStrEq("%0-.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%0-.f'", 0.0);
    AssertStrEq("%0-.f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0-.f'", -12345.968750);
    AssertStrEq("%0-.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%0-.0f'", 12345.968750);
    AssertStrEq("%0-.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%0-.0f'", 0.0);
    AssertStrEq("%0-.0f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0-.0f'", -12345.968750);
    AssertStrEq("%0-.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%0-.2f'", 12345.968750);
    AssertStrEq("%0-.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%0-.2f'", 0.0);
    AssertStrEq("%0-.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%0-.2f'", -12345.968750);
    AssertStrEq("%0-.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%0-.7f'", 12345.968750);
    AssertStrEq("%0-.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%0-.7f'", 0.0);
    AssertStrEq("%0-.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%0-.7f'", -12345.968750);
    AssertStrEq("%0-.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    logger.InfoF("'%-0.f'", 12345.968750);
    AssertStrEq("%-0.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-0.f'", 0.0);
    AssertStrEq("%-0.f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-0.f'", -12345.968750);
    AssertStrEq("%-0.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-0.0f'", 12345.968750);
    AssertStrEq("%-0.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-0.0f'", 0.0);
    AssertStrEq("%-0.0f", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-0.0f'", -12345.968750);
    AssertStrEq("%-0.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-0.2f'", 12345.968750);
    AssertStrEq("%-0.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%-0.2f'", 0.0);
    AssertStrEq("%-0.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%-0.2f'", -12345.968750);
    AssertStrEq("%-0.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%-0.7f'", 12345.968750);
    AssertStrEq("%-0.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%-0.7f'", 0.0);
    AssertStrEq("%-0.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%-0.7f'", -12345.968750);
    AssertStrEq("%-0.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    // flag & width & prec
    logger.InfoF("'%-3.f'", 12345.968750);
    AssertStrEq("%-3.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-3.f'", 0.0);
    AssertStrEq("%-3.f", sink.DrainLastMsgFast().msg, "'0  '");
    logger.InfoF("'%-3.f'", -12345.968750);
    AssertStrEq("%-3.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-3.0f'", 12345.968750);
    AssertStrEq("%-3.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-3.0f'", 0.0);
    AssertStrEq("%-3.0f", sink.DrainLastMsgFast().msg, "'0  '");
    logger.InfoF("'%-3.0f'", -12345.968750);
    AssertStrEq("%-3.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-3.2f'", 12345.968750);
    AssertStrEq("%-3.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%-3.2f'", 0.0);
    AssertStrEq("%-3.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%-3.2f'", -12345.968750);
    AssertStrEq("%-3.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%-3.7f'", 12345.968750);
    AssertStrEq("%-3.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%-3.7f'", 0.0);
    AssertStrEq("%-3.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%-3.7f'", -12345.968750);
    AssertStrEq("%-3.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    logger.InfoF("'%-03.f'", 12345.968750);
    AssertStrEq("%-03.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-03.f'", 0.0);
    AssertStrEq("%-03.f", sink.DrainLastMsgFast().msg, "'0  '");
    logger.InfoF("'%-03.f'", -12345.968750);
    AssertStrEq("%-03.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-03.0f'", 12345.968750);
    AssertStrEq("%-03.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%-03.0f'", 0.0);
    AssertStrEq("%-03.0f", sink.DrainLastMsgFast().msg, "'0  '");
    logger.InfoF("'%-03.0f'", -12345.968750);
    AssertStrEq("%-03.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%-03.2f'", 12345.968750);
    AssertStrEq("%-03.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%-03.2f'", 0.0);
    AssertStrEq("%-03.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%-03.2f'", -12345.968750);
    AssertStrEq("%-03.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%-03.7f'", 12345.968750);
    AssertStrEq("%-03.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%-03.7f'", 0.0);
    AssertStrEq("%-03.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%-03.7f'", -12345.968750);
    AssertStrEq("%-03.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    logger.InfoF("'%0-3.f'", 12345.968750);
    AssertStrEq("%0-3.f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%0-3.f'", 0.0);
    AssertStrEq("%0-3.f", sink.DrainLastMsgFast().msg, "'0  '");
    logger.InfoF("'%0-3.f'", -12345.968750);
    AssertStrEq("%0-3.f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%0-3.0f'", 12345.968750);
    AssertStrEq("%0-3.0f", sink.DrainLastMsgFast().msg, "'12345'");
    logger.InfoF("'%0-3.0f'", 0.0);
    AssertStrEq("%0-3.0f", sink.DrainLastMsgFast().msg, "'0  '");
    logger.InfoF("'%0-3.0f'", -12345.968750);
    AssertStrEq("%0-3.0f", sink.DrainLastMsgFast().msg, "'-12345'");

    logger.InfoF("'%0-3.2f'", 12345.968750);
    AssertStrEq("%0-3.2f", sink.DrainLastMsgFast().msg, "'12345.96'");
    logger.InfoF("'%0-3.2f'", 0.0);
    AssertStrEq("%0-3.2f", sink.DrainLastMsgFast().msg, "'0.00'");
    logger.InfoF("'%0-3.2f'", -12345.968750);
    AssertStrEq("%0-3.2f", sink.DrainLastMsgFast().msg, "'-12345.96'");

    logger.InfoF("'%0-3.7f'", 12345.968750);
    AssertStrEq("%0-3.7f", sink.DrainLastMsgFast().msg, "'12345.9687500'");
    logger.InfoF("'%0-3.7f'", 0.0);
    AssertStrEq("%0-3.7f", sink.DrainLastMsgFast().msg, "'0.0000000'");
    logger.InfoF("'%0-3.7f'", -12345.968750);
    AssertStrEq("%0-3.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500'");

    logger.InfoF("'%-20.f'", 12345.968750);
    AssertStrEq("%-20.f", sink.DrainLastMsgFast().msg, "'12345               '");
    logger.InfoF("'%-20.f'", 0.0);
    AssertStrEq("%-20.f", sink.DrainLastMsgFast().msg, "'0                   '");
    logger.InfoF("'%-20.f'", -12345.968750);
    AssertStrEq("%-20.f", sink.DrainLastMsgFast().msg, "'-12345              '");

    logger.InfoF("'%-20.0f'", 12345.968750);
    AssertStrEq("%-20.0f", sink.DrainLastMsgFast().msg, "'12345               '");
    logger.InfoF("'%-20.0f'", 0.0);
    AssertStrEq("%-20.0f", sink.DrainLastMsgFast().msg, "'0                   '");
    logger.InfoF("'%-20.0f'", -12345.968750);
    AssertStrEq("%-20.0f", sink.DrainLastMsgFast().msg, "'-12345              '");

    logger.InfoF("'%-20.2f'", 12345.968750);
    AssertStrEq("%-20.2f", sink.DrainLastMsgFast().msg, "'12345.96            '");
    logger.InfoF("'%-20.2f'", 0.0);
    AssertStrEq("%-20.2f", sink.DrainLastMsgFast().msg, "'0.00                '");
    logger.InfoF("'%-20.2f'", -12345.968750);
    AssertStrEq("%-20.2f", sink.DrainLastMsgFast().msg, "'-12345.96           '");

    logger.InfoF("'%-20.7f'", 12345.968750);
    AssertStrEq("%-20.7f", sink.DrainLastMsgFast().msg, "'12345.9687500       '");
    logger.InfoF("'%-20.7f'", 0.0);
    AssertStrEq("%-20.7f", sink.DrainLastMsgFast().msg, "'0.0000000           '");
    logger.InfoF("'%-20.7f'", -12345.968750);
    AssertStrEq("%-20.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500      '");

    logger.InfoF("'%020.f'", 12345.968750);
    AssertStrEq("%020.f", sink.DrainLastMsgFast().msg, "'00000000000000012345'");
    logger.InfoF("'%020.f'", 0.0);
    AssertStrEq("%020.f", sink.DrainLastMsgFast().msg, "'00000000000000000000'");
    logger.InfoF("'%020.f'", -12345.968750);
    AssertStrEq("%020.f", sink.DrainLastMsgFast().msg, "'-0000000000000012345'");

    logger.InfoF("'%020.0f'", 12345.968750);
    AssertStrEq("%020.0f", sink.DrainLastMsgFast().msg, "'00000000000000012345'");
    logger.InfoF("'%020.0f'", 0.0);
    AssertStrEq("%020.0f", sink.DrainLastMsgFast().msg, "'00000000000000000000'");
    logger.InfoF("'%020.0f'", -12345.968750);
    AssertStrEq("%020.0f", sink.DrainLastMsgFast().msg, "'-0000000000000012345'");

    logger.InfoF("'%020.2f'", 12345.968750);
    AssertStrEq("%020.2f", sink.DrainLastMsgFast().msg, "'00000000000012345.96'");
    logger.InfoF("'%020.2f'", 0.0);
    AssertStrEq("%020.2f", sink.DrainLastMsgFast().msg, "'00000000000000000.00'");
    logger.InfoF("'%020.2f'", -12345.968750);
    AssertStrEq("%020.2f", sink.DrainLastMsgFast().msg, "'-0000000000012345.96'");

    logger.InfoF("'%020.7f'", 12345.968750);
    AssertStrEq("%020.7f", sink.DrainLastMsgFast().msg, "'000000012345.9687500'");
    logger.InfoF("'%020.7f'", 0.0);
    AssertStrEq("%020.7f", sink.DrainLastMsgFast().msg, "'000000000000.0000000'");
    logger.InfoF("'%020.7f'", -12345.968750);
    AssertStrEq("%020.7f", sink.DrainLastMsgFast().msg, "'-00000012345.9687500'");

    logger.InfoF("'%0-20.f'", 12345.968750);
    AssertStrEq("%0-20.f", sink.DrainLastMsgFast().msg, "'12345               '");
    logger.InfoF("'%0-20.f'", 0.0);
    AssertStrEq("%0-20.f", sink.DrainLastMsgFast().msg, "'0                   '");
    logger.InfoF("'%0-20.f'", -12345.968750);
    AssertStrEq("%0-20.f", sink.DrainLastMsgFast().msg, "'-12345              '");

    logger.InfoF("'%0-20.0f'", 12345.968750);
    AssertStrEq("%0-20.0f", sink.DrainLastMsgFast().msg, "'12345               '");
    logger.InfoF("'%0-20.0f'", 0.0);
    AssertStrEq("%0-20.0f", sink.DrainLastMsgFast().msg, "'0                   '");
    logger.InfoF("'%0-20.0f'", -12345.968750);
    AssertStrEq("%0-20.0f", sink.DrainLastMsgFast().msg, "'-12345              '");

    logger.InfoF("'%0-20.2f'", 12345.968750);
    AssertStrEq("%0-20.2f", sink.DrainLastMsgFast().msg, "'12345.96            '");
    logger.InfoF("'%0-20.2f'", 0.0);
    AssertStrEq("%0-20.2f", sink.DrainLastMsgFast().msg, "'0.00                '");
    logger.InfoF("'%0-20.2f'", -12345.968750);
    AssertStrEq("%0-20.2f", sink.DrainLastMsgFast().msg, "'-12345.96           '");

    logger.InfoF("'%0-20.7f'", 12345.968750);
    AssertStrEq("%0-20.7f", sink.DrainLastMsgFast().msg, "'12345.9687500       '");
    logger.InfoF("'%0-20.7f'", 0.0);
    AssertStrEq("%0-20.7f", sink.DrainLastMsgFast().msg, "'0.0000000           '");
    logger.InfoF("'%0-20.7f'", -12345.968750);
    AssertStrEq("%0-20.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500      '");

    logger.InfoF("'%-020.f'", 12345.968750);
    AssertStrEq("%-020.f", sink.DrainLastMsgFast().msg, "'12345               '");
    logger.InfoF("'%-020.f'", 0.0);
    AssertStrEq("%-020.f", sink.DrainLastMsgFast().msg, "'0                   '");
    logger.InfoF("'%-020.f'", -12345.968750);
    AssertStrEq("%-020.f", sink.DrainLastMsgFast().msg, "'-12345              '");

    logger.InfoF("'%-020.0f'", 12345.968750);
    AssertStrEq("%-020.0f", sink.DrainLastMsgFast().msg, "'12345               '");
    logger.InfoF("'%-020.0f'", 0.0);
    AssertStrEq("%-020.0f", sink.DrainLastMsgFast().msg, "'0                   '");
    logger.InfoF("'%-020.0f'", -12345.968750);
    AssertStrEq("%-020.0f", sink.DrainLastMsgFast().msg, "'-12345              '");

    logger.InfoF("'%-020.2f'", 12345.968750);
    AssertStrEq("%-020.2f", sink.DrainLastMsgFast().msg, "'12345.96            '");
    logger.InfoF("'%-020.2f'", 0.0);
    AssertStrEq("%-020.2f", sink.DrainLastMsgFast().msg, "'0.00                '");
    logger.InfoF("'%-020.2f'", -12345.968750);
    AssertStrEq("%-020.2f", sink.DrainLastMsgFast().msg, "'-12345.96           '");

    logger.InfoF("'%-020.7f'", 12345.968750);
    AssertStrEq("%-020.7f", sink.DrainLastMsgFast().msg, "'12345.9687500       '");
    logger.InfoF("'%-020.7f'", 0.0);
    AssertStrEq("%-020.7f", sink.DrainLastMsgFast().msg, "'0.0000000           '");
    logger.InfoF("'%-020.7f'", -12345.968750);
    AssertStrEq("%-020.7f", sink.DrainLastMsgFast().msg, "'-12345.9687500      '");

    // Special
    logger.InfoF("'%f'", 0.0 / 0.0);
    AssertStrEq("'%f' - NaN", sink.DrainLastMsgFast().msg, "'NaN'");

    logger.InfoF("'%f'", 1.0 / 0.0);
    AssertStrEq("'%f' - Inf", sink.DrainLastMsgFast().msg, "'Inf'");

    logger.InfoF("'%f'", -1.0 / 0.0);
    AssertStrEq("'%f' - Inf", sink.DrainLastMsgFast().msg, "'-Inf'");

    logger.Close();
    sink.Close();
}


void TestBinary()
{
    // flags: [-] / [0]
    // %[flags][width]b
    SetTestContext("Test Binary");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoF("'%b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%b", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%0b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0b", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%0b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%-b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-b", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%-b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%0-b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-b", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0-b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%0-b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%-0b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-0b", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-0b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-0b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%-0b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-0b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    // width
    logger.InfoF("'%5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%5b", sink.DrainLastMsgFast().msg, "'    0'");
    logger.InfoF("'%5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%5b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%5b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%35b", sink.DrainLastMsgFast().msg, "'                                  0'");
    logger.InfoF("'%35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%35b", sink.DrainLastMsgFast().msg, "'    " ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%35b", sink.DrainLastMsgFast().msg, "'    " ... TEST_BINARY_VALUE3 ... "'");

    // flag & width
    logger.InfoF("'%05b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%05b", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%05b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%05b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%05b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%05b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%035b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%035b", sink.DrainLastMsgFast().msg, "'0000" ... TEST_BINARY_VALUE1 ... "'");
    logger.InfoF("'%035b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%035b", sink.DrainLastMsgFast().msg, "'0000" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%035b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%035b", sink.DrainLastMsgFast().msg, "'0000" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%-5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-5b", sink.DrainLastMsgFast().msg, "'0    '");
    logger.InfoF("'%-5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-5b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%-5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-5b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%-35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-35b", sink.DrainLastMsgFast().msg, "'0                                  '");
    logger.InfoF("'%-35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-35b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "    '");
    logger.InfoF("'%-35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-35b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "    '");

    logger.InfoF("'%-05b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-05b", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%-05b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-05b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%-05b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-05b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%-035b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-035b", sink.DrainLastMsgFast().msg, "'00000000000000000000000000000000000'");
    logger.InfoF("'%-035b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-035b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "0000'");
    logger.InfoF("'%-035b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-035b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "0000'");

    logger.InfoF("'%0-5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-5b", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%0-5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-5b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoF("'%0-5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-5b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoF("'%0-35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-35b", sink.DrainLastMsgFast().msg, "'00000000000000000000000000000000000'");
    logger.InfoF("'%0-35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-35b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE2 ... "0000'");
    logger.InfoF("'%0-35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-35b", sink.DrainLastMsgFast().msg, "'" ... TEST_BINARY_VALUE3 ... "0000'");

    logger.Close();
    sink.Close();
}


void TestUInt()
{
    // flags: [-] / [0]
    // %[flags][width]u
    SetTestContext("Test UInt");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoF("'%u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoF("'%u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%0u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoF("'%0u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%0u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%-u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoF("'%-u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%-u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%0-u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoF("'%0-u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%0-u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%-0u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-0u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoF("'%-0u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-0u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%-0u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-0u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    // width
    logger.InfoF("'%5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%5u", sink.DrainLastMsgFast().msg, "'    0'");
    logger.InfoF("'%5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%5u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%5u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%15u", sink.DrainLastMsgFast().msg, "'              0'");
    logger.InfoF("'%15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%15u", sink.DrainLastMsgFast().msg, "'     " ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%15u", sink.DrainLastMsgFast().msg, "'     " ... TEST_UINT_VALUE3 ... "'");

    // flag & width
    logger.InfoF("'%05u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%05u", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%05u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%05u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%05u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%05u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%015u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%015u", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%015u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%015u", sink.DrainLastMsgFast().msg, "'00000" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%015u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%015u", sink.DrainLastMsgFast().msg, "'00000" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%-5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-5u", sink.DrainLastMsgFast().msg, "'0    '");
    logger.InfoF("'%-5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-5u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%-5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-5u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%-15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-15u", sink.DrainLastMsgFast().msg, "'0              '");
    logger.InfoF("'%-15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-15u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "     '");
    logger.InfoF("'%-15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-15u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "     '");

    logger.InfoF("'%-05u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-05u", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%-05u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-05u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%-05u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-05u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%-015u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-015u", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%-015u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-015u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "00000'");
    logger.InfoF("'%-015u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-015u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "00000'");

    logger.InfoF("'%0-5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-5u", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%0-5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-5u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoF("'%0-5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-5u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoF("'%0-15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-15u", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%0-15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-15u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE2 ... "00000'");
    logger.InfoF("'%0-15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-15u", sink.DrainLastMsgFast().msg, "'" ... TEST_UINT_VALUE3 ... "00000'");

    logger.Close();
    sink.Close();
}


void TestInt()
{
    // flags: [-] / [0]
    // %[flags][width]d
    SetTestContext("Test Int");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoF("'%d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoF("'%d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%0d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoF("'%0d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%0d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%-d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoF("'%-d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%-d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%0-d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoF("'%0-d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%0-d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%-0d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-0d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoF("'%-0d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-0d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%-0d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-0d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    // width
    logger.InfoF("'%5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%5d", sink.DrainLastMsgFast().msg, "'    0'");
    logger.InfoF("'%5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%5d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%5d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%15d", sink.DrainLastMsgFast().msg, "'              0'");
    logger.InfoF("'%15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%15d", sink.DrainLastMsgFast().msg, "'     " ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%15d", sink.DrainLastMsgFast().msg, "'    " ... TEST_INT_VALUE3 ... "'");

    // flag & width
    logger.InfoF("'%05d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%05d", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%05d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%05d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%05d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%05d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%015d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%015d", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%015d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%015d", sink.DrainLastMsgFast().msg, "'00000" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%015d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%015d", sink.DrainLastMsgFast().msg, "'-00002147483648'");

    logger.InfoF("'%-5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-5d", sink.DrainLastMsgFast().msg, "'0    '");
    logger.InfoF("'%-5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-5d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%-5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-5d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%-15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-15d", sink.DrainLastMsgFast().msg, "'0              '");
    logger.InfoF("'%-15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-15d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "     '");
    logger.InfoF("'%-15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-15d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "    '");

    logger.InfoF("'%-05d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-05d", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%-05d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-05d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%-05d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-05d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%-015d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-015d", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%-015d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-015d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "00000'");
    logger.InfoF("'%-015d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-015d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "0000'");

    logger.InfoF("'%0-5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-5d", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%0-5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-5d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoF("'%0-5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-5d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoF("'%0-15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-15d", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%0-15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-15d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE2 ... "00000'");
    logger.InfoF("'%0-15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-15d", sink.DrainLastMsgFast().msg, "'" ... TEST_INT_VALUE3 ... "0000'");

    logger.Close();
    sink.Close();
}


void TestHex()
{
    TestHexUpper();
    TestHexLower();
}

void TestHexUpper()
{
    // flags: [-] / [0]
    // %[flags][width]X
    SetTestContext("Test Hex Upper");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoF("'%X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%X", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0X", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-X", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-X", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-0X", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-0X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-0X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    // width
    logger.InfoF("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%5X", sink.DrainLastMsgFast().msg, "'    0'");
    logger.InfoF("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%5X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%5X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%15X", sink.DrainLastMsgFast().msg, "'              0'");
    logger.InfoF("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%15X", sink.DrainLastMsgFast().msg, "'          " ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%15X", sink.DrainLastMsgFast().msg, "'       " ... TEST_HEX_UPPER_VALUE3 ... "'");

    // flag & width
    logger.InfoF("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%05X", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%05X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%05X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%015X", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%015X", sink.DrainLastMsgFast().msg, "'0000000000" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%015X", sink.DrainLastMsgFast().msg, "'0000000" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-5X", sink.DrainLastMsgFast().msg, "'0    '");
    logger.InfoF("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-5X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-5X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-15X", sink.DrainLastMsgFast().msg, "'0              '");
    logger.InfoF("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-15X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "          '");
    logger.InfoF("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-15X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "       '");

    logger.InfoF("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-05X", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-05X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-05X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-015X", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-015X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "0000000000'");
    logger.InfoF("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-015X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "0000000'");

    logger.InfoF("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-5X", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-5X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoF("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-5X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoF("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-15X", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-15X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE2 ... "0000000000'");
    logger.InfoF("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-15X", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_UPPER_VALUE3 ... "0000000'");

    logger.Close();
    sink.Close();
}

void TestHexLower()
{
    // flags: [-] / [0]
    // %[flags][width]x
    SetTestContext("Test Hex Lower");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoF("'%x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%x", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0x", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-x", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-x", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-0x", sink.DrainLastMsgFast().msg, "'0'");
    logger.InfoF("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-0x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-0x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    // width
    logger.InfoF("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%5x", sink.DrainLastMsgFast().msg, "'    0'");
    logger.InfoF("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%5x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%5x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%15x", sink.DrainLastMsgFast().msg, "'              0'");
    logger.InfoF("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%15x", sink.DrainLastMsgFast().msg, "'          " ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%15x", sink.DrainLastMsgFast().msg, "'       " ... TEST_HEX_LOWER_VALUE3 ... "'");

    // flag & width
    logger.InfoF("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%05x", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%05x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%05x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%015x", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%015x", sink.DrainLastMsgFast().msg, "'0000000000" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%015x", sink.DrainLastMsgFast().msg, "'0000000" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-5x", sink.DrainLastMsgFast().msg, "'0    '");
    logger.InfoF("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-5x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-5x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-15x", sink.DrainLastMsgFast().msg, "'0              '");
    logger.InfoF("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-15x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "          '");
    logger.InfoF("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-15x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "       '");

    logger.InfoF("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-05x", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-05x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-05x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-015x", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-015x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "0000000000'");
    logger.InfoF("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-015x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "0000000'");

    logger.InfoF("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-5x", sink.DrainLastMsgFast().msg, "'00000'");
    logger.InfoF("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-5x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoF("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-5x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoF("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-15x", sink.DrainLastMsgFast().msg, "'000000000000000'");
    logger.InfoF("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-15x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE2 ... "0000000000'");
    logger.InfoF("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-15x", sink.DrainLastMsgFast().msg, "'" ... TEST_HEX_LOWER_VALUE3 ... "0000000'");

    logger.Close();
    sink.Close();
}


void TestTranslates()
{
    AssertEq("Server Language", GetServerLanguage(), GetLanguageByCode("en"));

    // 内部是先获取 memory_buf 字符串，然后直接 append
    // %T, LANGID
    SetTestContext("Test Translates");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoF("'%T'", TEST_TRANSLATES_KEY, LANG_SERVER);
    AssertStrEq("%T", sink.DrainLastMsgFast().msg, "'" ... TEST_TRANSLATES_VALUE ... "'");

    logger.InfoF("'%T'", TEST_TRANSLATES_KEY1, LANG_SERVER, TEST_TRANSLATES_DATA1);
    AssertStrEq("%T", sink.DrainLastMsgFast().msg, "'" ... TEST_TRANSLATES_VALUE1 ... "'");

    logger.InfoF("'%T'", TEST_TRANSLATES_KEY2, LANG_SERVER, StringToInt(TEST_TRANSLATES_DATA2));
    AssertStrEq("%T", sink.DrainLastMsgFast().msg, "'" ... TEST_TRANSLATES_VALUE2 ... "'");

    logger.InfoF("'%T'", TEST_TRANSLATES_KEY3, LANG_SERVER, TEST_TRANSLATES_DATA3_1, TEST_TRANSLATES_DATA3_2);
    AssertStrEq("%T", sink.DrainLastMsgFast().msg, "'" ... TEST_TRANSLATES_VALUE3 ... "'");

    logger.Close();
    sink.Close();
}


void TestSpecial()
{
    // 内部是先获取字符串，然后调用 AddString
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Special");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoF("'%N'", 0);
    AssertStrEq("%N", sink.DrainLastMsgFast().msg, "'Console'");

    logger.InfoF("'%L'", 0);
    AssertStrEq("%L", sink.DrainLastMsgFast().msg, "'Console<0><Console><Console>'");

    logger.Close();
    sink.Close();
}


#if SOURCEMOD_V_MINOR >= 13
void TestBinary64()
{
    SetTestContext("Test Binary64");

    // Base
    AssertFmt64Eq("'%lb'",          0,                      "'0'");
    AssertFmt64Eq("'%lb'",          1234567,                "'100101101011010000111'");
    AssertFmt64Eq("'%lb'",          1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%lb'",          9223372036854775807,    "'111111111111111111111111111111111111111111111111111111111111111'");
    AssertFmt64Eq("'%lb'",          -9223372036854775807-1, "'1000000000000000000000000000000000000000000000000000000000000000'");
    AssertFmt64Eq("'%lb'",          -1,                     "'1111111111111111111111111111111111111111111111111111111111111111'");

    // Precision
    AssertFmt64Eq("'%.0lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%.32lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%.41lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%.48lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");

    // Width
    AssertFmt64Eq("'%0lb'",         1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%32lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%41lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%48lb'",        1234567654321,          "'       10001111101110001111101110110101110110001'");

    // Width & Precision
    AssertFmt64Eq("'%0.0lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.32lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.41lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.48lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%32.32lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%32.41lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%32.48lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%41.32lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%41.41lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%41.48lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%48.32lb'",     1234567654321,          "'       10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%48.41lb'",     1234567654321,          "'       10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%48.48lb'",     1234567654321,          "'       10001111101110001111101110110101110110001'");

    // Flags
    AssertFmt64Eq("'%0lb'",         1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-lb'",         1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0-lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");

    // Flags & Precision
    AssertFmt64Eq("'%0.lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.0lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.32lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.41lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%0.48lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-.lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-.0lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-.32lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-.41lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-.48lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-0.lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-0.0lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-0.32lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-0.41lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-0.48lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");

    // Flags & Width
    AssertFmt64Eq("'%00lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%032lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%041lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%048lb'",       1234567654321,          "'000000010001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-0lb'",        1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-32lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-41lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-48lb'",       1234567654321,          "'10001111101110001111101110110101110110001       '");
    AssertFmt64Eq("'%-00lb'",       1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-032lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-041lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-048lb'",      1234567654321,          "'100011111011100011111011101101011101100010000000'");

    // Flags & Precision & Width
    AssertFmt64Eq("'%032.lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%032.0lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%032.32lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%032.41lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%032.48lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%041.lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%041.0lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%041.32lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%041.41lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%041.48lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%048.lb'",      1234567654321,          "'000000010001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%048.0lb'",     1234567654321,          "'000000010001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%048.32lb'",    1234567654321,          "'000000010001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%048.41lb'",    1234567654321,          "'000000010001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%048.48lb'",    1234567654321,          "'000000010001111101110001111101110110101110110001'");

    AssertFmt64Eq("'%-32.lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-32.0lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-32.32lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-32.41lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-32.48lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-41.lb'",      1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-41.0lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-41.32lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-41.41lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-41.48lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-48.lb'",      1234567654321,          "'10001111101110001111101110110101110110001       '");
    AssertFmt64Eq("'%-48.0lb'",     1234567654321,          "'10001111101110001111101110110101110110001       '");
    AssertFmt64Eq("'%-48.32lb'",    1234567654321,          "'10001111101110001111101110110101110110001       '");
    AssertFmt64Eq("'%-48.41lb'",    1234567654321,          "'10001111101110001111101110110101110110001       '");
    AssertFmt64Eq("'%-48.48lb'",    1234567654321,          "'10001111101110001111101110110101110110001       '");

    AssertFmt64Eq("'%-032.lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-032.0lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-032.32lb'",   1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-032.41lb'",   1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-032.48lb'",   1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-041.lb'",     1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-041.0lb'",    1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-041.32lb'",   1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-041.41lb'",   1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-041.48lb'",   1234567654321,          "'10001111101110001111101110110101110110001'");
    AssertFmt64Eq("'%-048.lb'",     1234567654321,          "'100011111011100011111011101101011101100010000000'");
    AssertFmt64Eq("'%-048.0lb'",    1234567654321,          "'100011111011100011111011101101011101100010000000'");
    AssertFmt64Eq("'%-048.32lb'",   1234567654321,          "'100011111011100011111011101101011101100010000000'");
    AssertFmt64Eq("'%-048.41lb'",   1234567654321,          "'100011111011100011111011101101011101100010000000'");
    AssertFmt64Eq("'%-048.48lb'",   1234567654321,          "'100011111011100011111011101101011101100010000000'");
}


void TestInt64()
{
    SetTestContext("Test Integer64");

    // Base
    AssertFmt64Eq("'%ld'",          0,                      "'0'");
    AssertFmt64Eq("'%ld'",          1234567,                "'1234567'");
    AssertFmt64Eq("'%ld'",          1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%ld'",          9223372036854775807,    "'9223372036854775807'");
    AssertFmt64Eq("'%ld'",          -1234567,               "'-1234567'");
    AssertFmt64Eq("'%ld'",          -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%ld'",          -9223372036854775807-1, "'-9223372036854775808'");

    AssertFmt64Eq("'%li'",          0,                      "'0'");
    AssertFmt64Eq("'%li'",          1234567,                "'1234567'");
    AssertFmt64Eq("'%li'",          1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%li'",          9223372036854775807,    "'9223372036854775807'");
    AssertFmt64Eq("'%li'",          -1234567,               "'-1234567'");
    AssertFmt64Eq("'%li'",          -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%li'",          -9223372036854775807-1, "'-9223372036854775808'");

    // Precision
    AssertFmt64Eq("'%.0ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.7ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.13ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.20ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.0ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%.7ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%.14ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%.20ld'",       -1234567654321,         "'-1234567654321'");

    // Width
    AssertFmt64Eq("'%0ld'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%7ld'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%20ld'",        1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%0ld'",         -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%7ld'",         -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%14ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%20ld'",        -1234567654321,         "'      -1234567654321'");

    // Width & Precision
    AssertFmt64Eq("'%0.0ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.7ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.13ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.20ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.0ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.7ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.14ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.20ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%7.0ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%7.7ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%7.13ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%7.20ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%7.0ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%7.7ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%7.14ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%7.20ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%13.0ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13.7ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13.13ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13.20ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%14.0ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%14.7ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%14.14ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%14.20ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%20.0ld'",      1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%20.7ld'",      1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%20.13ld'",     1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%20.20ld'",     1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%20.0ld'",      -1234567654321,         "'      -1234567654321'");
    AssertFmt64Eq("'%20.7ld'",      -1234567654321,         "'      -1234567654321'");
    AssertFmt64Eq("'%20.14ld'",     -1234567654321,         "'      -1234567654321'");
    AssertFmt64Eq("'%20.20ld'",     -1234567654321,         "'      -1234567654321'");

    // Flags
    AssertFmt64Eq("'%0ld'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-ld'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0-ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0ld'",         -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-ld'",         -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0-ld'",        -1234567654321,         "'-1234567654321'");

    // Flags & Precision
    AssertFmt64Eq("'%0.ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.0ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.7ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.13ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.20ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.0ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.7ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.13ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.20ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.0ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.7ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.13ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.20ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.0ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.7ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.14ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%0.20ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-.ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-.0ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-.7ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-.14ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-.20ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.0ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.7ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.14ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.20ld'",     -1234567654321,         "'-1234567654321'");

    // Flags & Width
    AssertFmt64Eq("'%00ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%020ld'",       1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%-0ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7ld'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-20ld'",       1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-00ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-020ld'",      1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%00ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%07ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%014ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%020ld'",       -1234567654321,         "'-0000001234567654321'");
    AssertFmt64Eq("'%-0ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-7ld'",        -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-14ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-20ld'",       -1234567654321,         "'-1234567654321      '");
    AssertFmt64Eq("'%-00ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-07ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-014ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-020ld'",      -1234567654321,         "'-1234567654321000000'");

    // Flags & Precision & Width
    AssertFmt64Eq("'%00.ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.0ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.7ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.13ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.20ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.0ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.7ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.13ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.20ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.0ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.7ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.13ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.20ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%020.ld'",      1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.0ld'",     1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.7ld'",     1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.13ld'",    1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.20ld'",    1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%00.ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%00.0ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%00.7ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%00.14ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%00.20ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%07.ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%07.0ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%07.7ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%07.14ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%07.20ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%014.ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%014.0ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%014.7ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%014.14ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%014.20ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%020.ld'",      -1234567654321,         "'-0000001234567654321'");
    AssertFmt64Eq("'%020.0ld'",     -1234567654321,         "'-0000001234567654321'");
    AssertFmt64Eq("'%020.7ld'",     -1234567654321,         "'-0000001234567654321'");
    AssertFmt64Eq("'%020.14ld'",    -1234567654321,         "'-0000001234567654321'");
    AssertFmt64Eq("'%020.20ld'",    -1234567654321,         "'-0000001234567654321'");

    AssertFmt64Eq("'%-0.ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.0ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.7ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.13ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.20ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.ld'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.0ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.7ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.13ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.20ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.0ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.7ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.13ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.20ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-20.ld'",      1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.0ld'",     1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.7ld'",     1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.13ld'",    1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.20ld'",    1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-0.ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.0ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.7ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.14ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-0.20ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-7.ld'",       -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-7.0ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-7.7ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-7.14ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-7.20ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-14.ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-14.0ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-14.7ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-14.14ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-14.20ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-20.ld'",      -1234567654321,         "'-1234567654321      '");
    AssertFmt64Eq("'%-20.0ld'",     -1234567654321,         "'-1234567654321      '");
    AssertFmt64Eq("'%-20.7ld'",     -1234567654321,         "'-1234567654321      '");
    AssertFmt64Eq("'%-20.14ld'",    -1234567654321,         "'-1234567654321      '");
    AssertFmt64Eq("'%-20.20ld'",    -1234567654321,         "'-1234567654321      '");

    AssertFmt64Eq("'%-00.ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.0ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.7ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.13ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.20ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.ld'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.0ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.7ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.13ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.20ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.ld'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.0ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.7ld'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.13ld'",   1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.20ld'",   1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-020.ld'",     1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.0ld'",    1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.7ld'",    1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.13ld'",   1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.20ld'",   1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-00.ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-00.0ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-00.7ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-00.14ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-00.20ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-07.ld'",      -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-07.0ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-07.7ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-07.14ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-07.20ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-014.ld'",     -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-014.0ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-014.7ld'",    -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-014.14ld'",   -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-014.20ld'",   -1234567654321,         "'-1234567654321'");
    AssertFmt64Eq("'%-020.ld'",     -1234567654321,         "'-1234567654321000000'");
    AssertFmt64Eq("'%-020.0ld'",    -1234567654321,         "'-1234567654321000000'");
    AssertFmt64Eq("'%-020.7ld'",    -1234567654321,         "'-1234567654321000000'");
    AssertFmt64Eq("'%-020.14ld'",   -1234567654321,         "'-1234567654321000000'");
    AssertFmt64Eq("'%-020.20ld'",   -1234567654321,         "'-1234567654321000000'");
}


void TestUInt64()
{
    SetTestContext("Test Unsigned Integer64");

    // Base
    AssertFmt64Eq("'%lu'",          0,                      "'0'");
    AssertFmt64Eq("'%lu'",          1234567,                "'1234567'");
    AssertFmt64Eq("'%lu'",          1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%lu'",          9223372036854775807,    "'9223372036854775807'");
    AssertFmt64Eq("'%lu'",          -9223372036854775807-1, "'9223372036854775808'");
    AssertFmt64Eq("'%lu'",          -1,                     "'18446744073709551615'");

    // Precision
    AssertFmt64Eq("'%.0lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.7lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.13lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%.20lu'",       1234567654321,          "'1234567654321'");

    // Width
    AssertFmt64Eq("'%0lu'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%7lu'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%20lu'",        1234567654321,          "'       1234567654321'");

    // Width & Precision
    AssertFmt64Eq("'%0.7lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.13lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.20lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13.7lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13.13lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%13.20lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%20.7lu'",      1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%20.13lu'",     1234567654321,          "'       1234567654321'");
    AssertFmt64Eq("'%20.20lu'",     1234567654321,          "'       1234567654321'");

    // Flags
    AssertFmt64Eq("'%0lu'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-lu'",         1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0-lu'",        1234567654321,          "'1234567654321'");

    // Flags & Precision
    AssertFmt64Eq("'%0.lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.0lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.7lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.13lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%0.20lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.0lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.7lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.13lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-.20lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.0lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.7lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.13lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.20lu'",     1234567654321,          "'1234567654321'");

    // Flags & Width
    AssertFmt64Eq("'%00lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%020lu'",       1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%-0lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7lu'",        1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-20lu'",       1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-00lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-020lu'",      1234567654321,          "'12345676543210000000'");

    // Flags & Precision & Width
    AssertFmt64Eq("'%00.lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.0lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.7lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.13lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%00.20lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.0lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.7lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.13lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%07.20lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.0lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.7lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.13lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%013.20lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%020.lu'",      1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.0lu'",     1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.7lu'",     1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.13lu'",    1234567654321,          "'00000001234567654321'");
    AssertFmt64Eq("'%020.20lu'",    1234567654321,          "'00000001234567654321'");

    AssertFmt64Eq("'%-0.lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.0lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.7lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.13lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-0.20lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.lu'",       1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.0lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.7lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.13lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-7.20lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.0lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.7lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.13lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-13.20lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-20.lu'",      1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.0lu'",     1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.7lu'",     1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.13lu'",    1234567654321,          "'1234567654321       '");
    AssertFmt64Eq("'%-20.20lu'",    1234567654321,          "'1234567654321       '");

    AssertFmt64Eq("'%-00.lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.0lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.7lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.13lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-00.20lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.lu'",      1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.0lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.7lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.13lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-07.20lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.lu'",     1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.0lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.7lu'",    1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.13lu'",   1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-013.20lu'",   1234567654321,          "'1234567654321'");
    AssertFmt64Eq("'%-020.lu'",     1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.0lu'",    1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.7lu'",    1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.13lu'",   1234567654321,          "'12345676543210000000'");
    AssertFmt64Eq("'%-020.20lu'",   1234567654321,          "'12345676543210000000'");
}


void TestHex64()
{
    SetTestContext("Test Hexadecimal64");

    // Base
    AssertFmt64Eq("'%lX'",          0,                      "'0'");
    AssertFmt64Eq("'%lX'",          1234567,                "'12D687'");
    AssertFmt64Eq("'%lX'",          1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%lX'",          9223372036854775807,    "'7FFFFFFFFFFFFFFF'");
    AssertFmt64Eq("'%lX'",          -9223372036854775807-1, "'8000000000000000'");

    AssertFmt64Eq("'%lx'",          0,                      "'0'");
    AssertFmt64Eq("'%lx'",          1234567,                "'12d687'");
    AssertFmt64Eq("'%lx'",          1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%lx'",          9223372036854775807,    "'7fffffffffffffff'");
    AssertFmt64Eq("'%lX'",          -9223372036854775807-1, "'8000000000000000'");

    // Precision
    AssertFmt64Eq("'%.0lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%.7lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%.11lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%.20lX'",       1234567654321,          "'11F71F76BB1'");

    AssertFmt64Eq("'%.0lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%.7lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%.11lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%.20lx'",       1234567654321,          "'11f71f76bb1'");

    // Width
    AssertFmt64Eq("'%0lX'",         1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%7lX'",         1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%11lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%20lX'",        1234567654321,          "'         11F71F76BB1'");

    AssertFmt64Eq("'%0lx'",         1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%7lx'",         1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%11lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%20lx'",        1234567654321,          "'         11f71f76bb1'");

    // Width & Precision
    AssertFmt64Eq("'%0.0lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0.7lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0.11lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0.20lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%7.0lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%7.7lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%7.11lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%7.20lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%11.0lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%11.7lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%11.11lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%11.20lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%20.0lX'",      1234567654321,          "'         11F71F76BB1'");
    AssertFmt64Eq("'%20.7lX'",      1234567654321,          "'         11F71F76BB1'");
    AssertFmt64Eq("'%20.11lX'",     1234567654321,          "'         11F71F76BB1'");
    AssertFmt64Eq("'%20.20lX'",     1234567654321,          "'         11F71F76BB1'");

    AssertFmt64Eq("'%0.0lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0.7lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0.11lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0.20lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%7.0lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%7.7lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%7.11lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%7.20lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%11.0lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%11.7lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%11.11lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%11.20lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%20.0lx'",      1234567654321,          "'         11f71f76bb1'");
    AssertFmt64Eq("'%20.7lx'",      1234567654321,          "'         11f71f76bb1'");
    AssertFmt64Eq("'%20.11lx'",     1234567654321,          "'         11f71f76bb1'");
    AssertFmt64Eq("'%20.20lx'",     1234567654321,          "'         11f71f76bb1'");

    // Flags
    AssertFmt64Eq("'%0lX'",         1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-lX'",         1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0-lX'",        1234567654321,          "'11F71F76BB1'");

    AssertFmt64Eq("'%0lx'",         1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-lx'",         1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0-lx'",        1234567654321,          "'11f71f76bb1'");

    // Flags & Precision
    AssertFmt64Eq("'%0.0lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0.7lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0.11lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%0.20lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-.0lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-.7lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-.11lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-.20lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.0lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.7lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.11lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.20lX'",     1234567654321,          "'11F71F76BB1'");

    AssertFmt64Eq("'%0.0lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0.7lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0.11lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%0.20lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-.0lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-.7lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-.11lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-.20lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.0lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.7lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.11lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.20lx'",     1234567654321,          "'11f71f76bb1'");

    // Flags & Width
    AssertFmt64Eq("'%00lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%07lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%011lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%020lX'",       1234567654321,          "'00000000011F71F76BB1'");
    AssertFmt64Eq("'%-0lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-7lX'",        1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-11lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-20lX'",       1234567654321,          "'11F71F76BB1         '");
    AssertFmt64Eq("'%-00lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-07lX'",       1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-011lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-020lX'",      1234567654321,          "'11F71F76BB1000000000'");

    AssertFmt64Eq("'%00lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%07lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%011lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%020lx'",       1234567654321,          "'00000000011f71f76bb1'");
    AssertFmt64Eq("'%-0lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-7lx'",        1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-11lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-20lx'",       1234567654321,          "'11f71f76bb1         '");
    AssertFmt64Eq("'%-00lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-07lx'",       1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-011lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-020lx'",      1234567654321,          "'11f71f76bb1000000000'");

    // Flags & Precision & Width
    AssertFmt64Eq("'%00.0lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%00.7lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%00.11lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%00.20lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%07.0lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%07.7lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%07.11lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%07.20lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%011.0lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%011.7lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%011.11lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%011.20lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%020.0lX'",     1234567654321,          "'00000000011F71F76BB1'");
    AssertFmt64Eq("'%020.7lX'",     1234567654321,          "'00000000011F71F76BB1'");
    AssertFmt64Eq("'%020.11lX'",    1234567654321,          "'00000000011F71F76BB1'");
    AssertFmt64Eq("'%020.20lX'",    1234567654321,          "'00000000011F71F76BB1'");

    AssertFmt64Eq("'%00.0lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%00.7lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%00.11lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%00.20lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%07.0lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%07.7lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%07.11lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%07.20lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%011.0lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%011.7lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%011.11lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%011.20lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%020.0lx'",     1234567654321,          "'00000000011f71f76bb1'");
    AssertFmt64Eq("'%020.7lx'",     1234567654321,          "'00000000011f71f76bb1'");
    AssertFmt64Eq("'%020.11lx'",    1234567654321,          "'00000000011f71f76bb1'");
    AssertFmt64Eq("'%020.20lx'",    1234567654321,          "'00000000011f71f76bb1'");

    AssertFmt64Eq("'%-0.0lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.7lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.11lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-0.20lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-7.0lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-7.7lX'",      1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-7.11lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-7.20lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-11.0lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-11.7lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-11.11lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-11.20lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-20.0lX'",     1234567654321,          "'11F71F76BB1         '");
    AssertFmt64Eq("'%-20.7lX'",     1234567654321,          "'11F71F76BB1         '");
    AssertFmt64Eq("'%-20.11lX'",    1234567654321,          "'11F71F76BB1         '");
    AssertFmt64Eq("'%-20.20lX'",    1234567654321,          "'11F71F76BB1         '");

    AssertFmt64Eq("'%-0.0lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.7lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.11lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-0.20lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-7.0lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-7.7lx'",      1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-7.11lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-7.20lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-11.0lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-11.7lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-11.11lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-11.20lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-20.0lx'",     1234567654321,          "'11f71f76bb1         '");
    AssertFmt64Eq("'%-20.7lx'",     1234567654321,          "'11f71f76bb1         '");
    AssertFmt64Eq("'%-20.11lx'",    1234567654321,          "'11f71f76bb1         '");
    AssertFmt64Eq("'%-20.20lx'",    1234567654321,          "'11f71f76bb1         '");

    AssertFmt64Eq("'%-00.0lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-00.7lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-00.11lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-00.20lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-07.0lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-07.7lX'",     1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-07.11lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-07.20lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-011.0lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-011.7lX'",    1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-011.11lX'",   1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-011.20lX'",   1234567654321,          "'11F71F76BB1'");
    AssertFmt64Eq("'%-020.0lX'",    1234567654321,          "'11F71F76BB1000000000'");
    AssertFmt64Eq("'%-020.7lX'",    1234567654321,          "'11F71F76BB1000000000'");
    AssertFmt64Eq("'%-020.11lX'",   1234567654321,          "'11F71F76BB1000000000'");
    AssertFmt64Eq("'%-020.20lX'",   1234567654321,          "'11F71F76BB1000000000'");

    AssertFmt64Eq("'%-00.0lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-00.7lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-00.11lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-00.20lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-07.0lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-07.7lx'",     1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-07.11lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-07.20lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-011.0lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-011.7lx'",    1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-011.11lx'",   1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-011.20lx'",   1234567654321,          "'11f71f76bb1'");
    AssertFmt64Eq("'%-020.0lx'",    1234567654321,          "'11f71f76bb1000000000'");
    AssertFmt64Eq("'%-020.7lx'",    1234567654321,          "'11f71f76bb1000000000'");
    AssertFmt64Eq("'%-020.11lx'",   1234567654321,          "'11f71f76bb1000000000'");
    AssertFmt64Eq("'%-020.20lx'",   1234567654321,          "'11f71f76bb1000000000'");
}


stock void AssertFmt64Eq(const char[] fmt, const int64 value, const char[] expected)
{
    char buffer[512];
    FormatF(buffer, sizeof(buffer), fmt, value);
    AssertStrEq(fmt, buffer, expected);
}
#endif
