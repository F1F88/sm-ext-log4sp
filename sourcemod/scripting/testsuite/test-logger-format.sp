#include <log4sp>

#include "test_sink"

/**
 * Fix: "Not enough space on the heap"
 * 似乎是由于 TestSink.GetLastLogMsg 的调用堆栈过深
 */
#pragma dynamic 131072

#pragma semicolon 1
#pragma newdecls required

/**
 * Full syntax
 *      %[flags][width][.precision]specifier
 * flags
 *      [-] / [0]
 * NOTE
 *      SM 1.13.0.7198 修复了左对齐溢出的 BUG
 *      SM 的 %s 总是左对齐
 *      SM 的 %0[width]d 在传递负数时, 负号会添加在填充符 0 之后 '-1' --> '000-1'
 *      Log4sp 不支持 %t（会使用全局替代）
 *      二者 Float 类型左对齐时都只会在后方添加 ' ' (不会添加 '0')
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

    logger.InfoAmxTpl("'%% %%- %%. %%0 %%7 %%07 %%F %%c %%d'");
    AssertStrEq("AmxTpl", sink.GetLastLogMsg(), "'% %- %. %0 %7 %07 %F %c %d'");

    logger.InfoEx("'%% %%- %%. %%0 %%7 %%07 %%F %%c %%d'");
    AssertStrEq("Ex", sink.GetLastLogMsg(), "'% %- %. %0 %7 %07 %F %c %d'");

    logger.Close();
    sink.Close();
}


void TestString()
{
    TestStringAmxTpl();
    TestStringEx();
}

void TestStringAmxTpl()
{
    // %[width]s
    SetTestContext("Test String AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%s'", NULL_STRING);
    AssertStrEq("%s", sink.GetLastLogMsg(), "''");

    logger.InfoAmxTpl("'%s'", TEST_STRING_TEXT);
    AssertStrEq("%s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoAmxTpl("'%-s'", TEST_STRING_TEXT);
    AssertStrEq("%-s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoAmxTpl("'%10s'", TEST_STRING_TEXT);
    AssertStrEq("%10s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    // ! Warn 总是左对齐
    logger.InfoAmxTpl("'%20s'", TEST_STRING_TEXT);
    AssertStrEq("%20s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "    '");

    logger.InfoAmxTpl("'%-10s'", TEST_STRING_TEXT);
    AssertStrEq("%-10s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoAmxTpl("'%-20s'", TEST_STRING_TEXT);
    AssertStrEq("%-20s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "    '");

    logger.Close();
    sink.Close();
}

void TestStringEx()
{
    // flags: [-]
    // %[flags][width]s
    SetTestContext("Test String Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoEx("'%s'", NULL_STRING);
    AssertStrEq("%s", sink.GetLastLogMsg(), "''");

    logger.InfoEx("'%s'", TEST_STRING_TEXT);
    AssertStrEq("%s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoEx("'%-s'", TEST_STRING_TEXT);
    AssertStrEq("%-s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoEx("'%10s'", TEST_STRING_TEXT);
    AssertStrEq("%10s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoEx("'%20s'", TEST_STRING_TEXT);
    AssertStrEq("%20s", sink.GetLastLogMsg(), "'    " ... TEST_STRING_TEXT ... "'");

    logger.InfoEx("'%-10s'", TEST_STRING_TEXT);
    AssertStrEq("%-10s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "'");

    logger.InfoEx("'%-20s'", TEST_STRING_TEXT);
    AssertStrEq("%-20s", sink.GetLastLogMsg(), "'" ... TEST_STRING_TEXT ... "    '");

    logger.Close();
    sink.Close();
}

void TestFloat()
{
    TestFloatAmxTpl();
    TestFloatEx();
}

void TestFloatAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width][.precision]f
    SetTestContext("Test Float AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%f'", 12345.968750);
    AssertStrEq("%f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%f'", 0.0);
    AssertStrEq("%f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%f'", -12345.968750);
    AssertStrEq("%f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%-f'", 12345.968750);
    AssertStrEq("%-f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%-f'", 0.0);
    AssertStrEq("%-f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%-f'", -12345.968750);
    AssertStrEq("%-f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%0f'", 12345.968750);
    AssertStrEq("%0f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%0f'", 0.0);
    // AssertStrEq("%0f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%0f'", -12345.968750);
    AssertStrEq("%0f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%-0f'", 12345.968750);
    AssertStrEq("%-0f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%-0f'", 0.0);
    AssertStrEq("%-0f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%-0f'", -12345.968750);
    AssertStrEq("%-0f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%0-f'", 12345.968750);
    AssertStrEq("%0-f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%0-f'", 0.0);
    AssertStrEq("%0-f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%0-f'", -12345.968750);
    AssertStrEq("%0-f", sink.GetLastLogMsg(), "'-12345.968750'");

    // width
    logger.InfoAmxTpl("'%3f'", 12345.968750);
    AssertStrEq("%3f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%3f'", 0.0);
    AssertStrEq("%3f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%3f'", -12345.968750);
    AssertStrEq("%3f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%20f'", 12345.968750);
    AssertStrEq("%20f", sink.GetLastLogMsg(), "'        12345.968750'");
    logger.InfoAmxTpl("'%20f'", 0.0);
    AssertStrEq("%20f", sink.GetLastLogMsg(), "'            0.000000'");
    logger.InfoAmxTpl("'%20f'", -12345.968750);
    AssertStrEq("%20f", sink.GetLastLogMsg(), "'       -12345.968750'");

    // flag & width
    logger.InfoAmxTpl("'%-3f'", 12345.968750);
    AssertStrEq("%-3f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%-3f'", 0.0);
    AssertStrEq("%-3f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%-3f'", -12345.968750);
    AssertStrEq("%-3f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%-20f'", 12345.968750);
    AssertStrEq("%-20f", sink.GetLastLogMsg(), "'12345.968750        '");
    logger.InfoAmxTpl("'%-20f'", 0.0);
    AssertStrEq("%-20f", sink.GetLastLogMsg(), "'0.000000            '");
    logger.InfoAmxTpl("'%-20f'", -12345.968750);
    AssertStrEq("%-20f", sink.GetLastLogMsg(), "'-12345.968750       '");

    logger.InfoAmxTpl("'%03f'", 12345.968750);
    AssertStrEq("%03f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%03f'", 0.0);
    AssertStrEq("%03f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%03f'", -12345.968750);
    AssertStrEq("%03f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%020f'", 12345.968750);
    AssertStrEq("%020f", sink.GetLastLogMsg(), "'0000000012345.968750'");
    logger.InfoAmxTpl("'%020f'", 0.0);
    AssertStrEq("%020f", sink.GetLastLogMsg(), "'0000000000000.000000'");
    logger.InfoAmxTpl("'%020f'", -12345.968750);
    AssertStrEq("%020f", sink.GetLastLogMsg(), "'-000000012345.968750'");

    logger.InfoAmxTpl("'%-03f'", 12345.968750);
    AssertStrEq("%-03f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%-03f'", 0.0);
    AssertStrEq("%-03f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%-03f'", -12345.968750);
    AssertStrEq("%-03f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%-020f'", 12345.968750);
    AssertStrEq("%-020f", sink.GetLastLogMsg(), "'12345.968750        '");
    logger.InfoAmxTpl("'%-020f'", 0.0);
    AssertStrEq("%-020f", sink.GetLastLogMsg(), "'0.000000            '");
    logger.InfoAmxTpl("'%-020f'", -12345.968750);
    AssertStrEq("%-020f", sink.GetLastLogMsg(), "'-12345.968750       '");

    logger.InfoAmxTpl("'%0-3f'", 12345.968750);
    AssertStrEq("%0-3f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoAmxTpl("'%0-3f'", 0.0);
    AssertStrEq("%0-3f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoAmxTpl("'%0-3f'", -12345.968750);
    AssertStrEq("%0-3f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoAmxTpl("'%0-20f'", 12345.968750);
    AssertStrEq("%0-20f", sink.GetLastLogMsg(), "'12345.968750        '");
    logger.InfoAmxTpl("'%0-20f'", 0.0);
    AssertStrEq("%0-20f", sink.GetLastLogMsg(), "'0.000000            '");
    logger.InfoAmxTpl("'%0-20f'", -12345.968750);
    AssertStrEq("%0-20f", sink.GetLastLogMsg(), "'-12345.968750       '");

    // prec
    logger.InfoAmxTpl("'%.f'", 12345.968750);
    AssertStrEq("%.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%.f'", 0.0);
    AssertStrEq("%.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%.f'", -12345.968750);
    AssertStrEq("%.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%.0f'", 12345.968750);
    AssertStrEq("%.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%.0f'", 0.0);
    AssertStrEq("%.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%.0f'", -12345.968750);
    AssertStrEq("%.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%.2f'", 12345.968750);
    AssertStrEq("%.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%.2f'", 0.0);
    AssertStrEq("%.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%.2f'", -12345.968750);
    AssertStrEq("%.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%.7f'", 12345.968750);
    AssertStrEq("%.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%.7f'", 0.0);
    AssertStrEq("%.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%.7f'", -12345.968750);
    AssertStrEq("%.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    // flag & prec
    logger.InfoAmxTpl("'%-.f'", 12345.968750);
    AssertStrEq("%-.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-.f'", 0.0);
    AssertStrEq("%-.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-.f'", -12345.968750);
    AssertStrEq("%-.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-.0f'", 12345.968750);
    AssertStrEq("%-.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-.0f'", 0.0);
    AssertStrEq("%-.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-.0f'", -12345.968750);
    AssertStrEq("%-.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-.2f'", 12345.968750);
    AssertStrEq("%-.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%-.2f'", 0.0);
    AssertStrEq("%-.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%-.2f'", -12345.968750);
    AssertStrEq("%-.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%-.7f'", 12345.968750);
    AssertStrEq("%-.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%-.7f'", 0.0);
    AssertStrEq("%-.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%-.7f'", -12345.968750);
    AssertStrEq("%-.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoAmxTpl("'%0.f'", 12345.968750);
    AssertStrEq("%0.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%0.f'", 0.0);
    AssertStrEq("%0.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0.f'", -12345.968750);
    AssertStrEq("%0.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%0.0f'", 12345.968750);
    AssertStrEq("%0.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%0.0f'", 0.0);
    AssertStrEq("%0.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0.0f'", -12345.968750);
    AssertStrEq("%0.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%0.2f'", 12345.968750);
    AssertStrEq("%0.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%0.2f'", 0.0);
    AssertStrEq("%0.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%0.2f'", -12345.968750);
    AssertStrEq("%0.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%0.7f'", 12345.968750);
    AssertStrEq("%0.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%0.7f'", 0.0);
    AssertStrEq("%0.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%0.7f'", -12345.968750);
    AssertStrEq("%0.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoAmxTpl("'%0-.f'", 12345.968750);
    AssertStrEq("%0-.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%0-.f'", 0.0);
    AssertStrEq("%0-.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0-.f'", -12345.968750);
    AssertStrEq("%0-.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%0-.0f'", 12345.968750);
    AssertStrEq("%0-.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%0-.0f'", 0.0);
    AssertStrEq("%0-.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0-.0f'", -12345.968750);
    AssertStrEq("%0-.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%0-.2f'", 12345.968750);
    AssertStrEq("%0-.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%0-.2f'", 0.0);
    AssertStrEq("%0-.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%0-.2f'", -12345.968750);
    AssertStrEq("%0-.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%0-.7f'", 12345.968750);
    AssertStrEq("%0-.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%0-.7f'", 0.0);
    AssertStrEq("%0-.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%0-.7f'", -12345.968750);
    AssertStrEq("%0-.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoAmxTpl("'%-0.f'", 12345.968750);
    AssertStrEq("%-0.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-0.f'", 0.0);
    AssertStrEq("%-0.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-0.f'", -12345.968750);
    AssertStrEq("%-0.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-0.0f'", 12345.968750);
    AssertStrEq("%-0.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-0.0f'", 0.0);
    AssertStrEq("%-0.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-0.0f'", -12345.968750);
    AssertStrEq("%-0.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-0.2f'", 12345.968750);
    AssertStrEq("%-0.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%-0.2f'", 0.0);
    AssertStrEq("%-0.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%-0.2f'", -12345.968750);
    AssertStrEq("%-0.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%-0.7f'", 12345.968750);
    AssertStrEq("%-0.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%-0.7f'", 0.0);
    AssertStrEq("%-0.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%-0.7f'", -12345.968750);
    AssertStrEq("%-0.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    // flag & width & prec
    logger.InfoAmxTpl("'%-3.f'", 12345.968750);
    AssertStrEq("%-3.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-3.f'", 0.0);
    AssertStrEq("%-3.f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoAmxTpl("'%-3.f'", -12345.968750);
    AssertStrEq("%-3.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-3.0f'", 12345.968750);
    AssertStrEq("%-3.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-3.0f'", 0.0);
    AssertStrEq("%-3.0f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoAmxTpl("'%-3.0f'", -12345.968750);
    AssertStrEq("%-3.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-3.2f'", 12345.968750);
    AssertStrEq("%-3.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%-3.2f'", 0.0);
    AssertStrEq("%-3.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%-3.2f'", -12345.968750);
    AssertStrEq("%-3.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%-3.7f'", 12345.968750);
    AssertStrEq("%-3.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%-3.7f'", 0.0);
    AssertStrEq("%-3.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%-3.7f'", -12345.968750);
    AssertStrEq("%-3.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoAmxTpl("'%-03.f'", 12345.968750);
    AssertStrEq("%-03.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-03.f'", 0.0);
    AssertStrEq("%-03.f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoAmxTpl("'%-03.f'", -12345.968750);
    AssertStrEq("%-03.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-03.0f'", 12345.968750);
    AssertStrEq("%-03.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%-03.0f'", 0.0);
    AssertStrEq("%-03.0f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoAmxTpl("'%-03.0f'", -12345.968750);
    AssertStrEq("%-03.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%-03.2f'", 12345.968750);
    AssertStrEq("%-03.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%-03.2f'", 0.0);
    AssertStrEq("%-03.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%-03.2f'", -12345.968750);
    AssertStrEq("%-03.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%-03.7f'", 12345.968750);
    AssertStrEq("%-03.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%-03.7f'", 0.0);
    AssertStrEq("%-03.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%-03.7f'", -12345.968750);
    AssertStrEq("%-03.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoAmxTpl("'%0-3.f'", 12345.968750);
    AssertStrEq("%0-3.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%0-3.f'", 0.0);
    AssertStrEq("%0-3.f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoAmxTpl("'%0-3.f'", -12345.968750);
    AssertStrEq("%0-3.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%0-3.0f'", 12345.968750);
    AssertStrEq("%0-3.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoAmxTpl("'%0-3.0f'", 0.0);
    AssertStrEq("%0-3.0f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoAmxTpl("'%0-3.0f'", -12345.968750);
    AssertStrEq("%0-3.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoAmxTpl("'%0-3.2f'", 12345.968750);
    AssertStrEq("%0-3.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoAmxTpl("'%0-3.2f'", 0.0);
    AssertStrEq("%0-3.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoAmxTpl("'%0-3.2f'", -12345.968750);
    AssertStrEq("%0-3.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoAmxTpl("'%0-3.7f'", 12345.968750);
    AssertStrEq("%0-3.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoAmxTpl("'%0-3.7f'", 0.0);
    AssertStrEq("%0-3.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoAmxTpl("'%0-3.7f'", -12345.968750);
    AssertStrEq("%0-3.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoAmxTpl("'%-20.f'", 12345.968750);
    AssertStrEq("%-20.f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoAmxTpl("'%-20.f'", 0.0);
    AssertStrEq("%-20.f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoAmxTpl("'%-20.f'", -12345.968750);
    AssertStrEq("%-20.f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoAmxTpl("'%-20.0f'", 12345.968750);
    AssertStrEq("%-20.0f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoAmxTpl("'%-20.0f'", 0.0);
    AssertStrEq("%-20.0f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoAmxTpl("'%-20.0f'", -12345.968750);
    AssertStrEq("%-20.0f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoAmxTpl("'%-20.2f'", 12345.968750);
    AssertStrEq("%-20.2f", sink.GetLastLogMsg(), "'12345.96            '");
    logger.InfoAmxTpl("'%-20.2f'", 0.0);
    AssertStrEq("%-20.2f", sink.GetLastLogMsg(), "'0.00                '");
    logger.InfoAmxTpl("'%-20.2f'", -12345.968750);
    AssertStrEq("%-20.2f", sink.GetLastLogMsg(), "'-12345.96           '");

    logger.InfoAmxTpl("'%-20.7f'", 12345.968750);
    AssertStrEq("%-20.7f", sink.GetLastLogMsg(), "'12345.9687500       '");
    logger.InfoAmxTpl("'%-20.7f'", 0.0);
    AssertStrEq("%-20.7f", sink.GetLastLogMsg(), "'0.0000000           '");
    logger.InfoAmxTpl("'%-20.7f'", -12345.968750);
    AssertStrEq("%-20.7f", sink.GetLastLogMsg(), "'-12345.9687500      '");

    logger.InfoAmxTpl("'%020.f'", 12345.968750);
    AssertStrEq("%020.f", sink.GetLastLogMsg(), "'00000000000000012345'");
    logger.InfoAmxTpl("'%020.f'", 0.0);
    AssertStrEq("%020.f", sink.GetLastLogMsg(), "'00000000000000000000'");
    logger.InfoAmxTpl("'%020.f'", -12345.968750);
    AssertStrEq("%020.f", sink.GetLastLogMsg(), "'-0000000000000012345'");

    logger.InfoAmxTpl("'%020.0f'", 12345.968750);
    AssertStrEq("%020.0f", sink.GetLastLogMsg(), "'00000000000000012345'");
    logger.InfoAmxTpl("'%020.0f'", 0.0);
    AssertStrEq("%020.0f", sink.GetLastLogMsg(), "'00000000000000000000'");
    logger.InfoAmxTpl("'%020.0f'", -12345.968750);
    AssertStrEq("%020.0f", sink.GetLastLogMsg(), "'-0000000000000012345'");

    logger.InfoAmxTpl("'%020.2f'", 12345.968750);
    AssertStrEq("%020.2f", sink.GetLastLogMsg(), "'00000000000012345.96'");
    logger.InfoAmxTpl("'%020.2f'", 0.0);
    AssertStrEq("%020.2f", sink.GetLastLogMsg(), "'00000000000000000.00'");
    logger.InfoAmxTpl("'%020.2f'", -12345.968750);
    AssertStrEq("%020.2f", sink.GetLastLogMsg(), "'-0000000000012345.96'");

    logger.InfoAmxTpl("'%020.7f'", 12345.968750);
    AssertStrEq("%020.7f", sink.GetLastLogMsg(), "'000000012345.9687500'");
    logger.InfoAmxTpl("'%020.7f'", 0.0);
    AssertStrEq("%020.7f", sink.GetLastLogMsg(), "'000000000000.0000000'");
    logger.InfoAmxTpl("'%020.7f'", -12345.968750);
    AssertStrEq("%020.7f", sink.GetLastLogMsg(), "'-00000012345.9687500'");

    logger.InfoAmxTpl("'%0-20.f'", 12345.968750);
    AssertStrEq("%0-20.f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoAmxTpl("'%0-20.f'", 0.0);
    AssertStrEq("%0-20.f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoAmxTpl("'%0-20.f'", -12345.968750);
    AssertStrEq("%0-20.f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoAmxTpl("'%0-20.0f'", 12345.968750);
    AssertStrEq("%0-20.0f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoAmxTpl("'%0-20.0f'", 0.0);
    AssertStrEq("%0-20.0f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoAmxTpl("'%0-20.0f'", -12345.968750);
    AssertStrEq("%0-20.0f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoAmxTpl("'%0-20.2f'", 12345.968750);
    AssertStrEq("%0-20.2f", sink.GetLastLogMsg(), "'12345.96            '");
    logger.InfoAmxTpl("'%0-20.2f'", 0.0);
    AssertStrEq("%0-20.2f", sink.GetLastLogMsg(), "'0.00                '");
    logger.InfoAmxTpl("'%0-20.2f'", -12345.968750);
    AssertStrEq("%0-20.2f", sink.GetLastLogMsg(), "'-12345.96           '");

    logger.InfoAmxTpl("'%0-20.7f'", 12345.968750);
    AssertStrEq("%0-20.7f", sink.GetLastLogMsg(), "'12345.9687500       '");
    logger.InfoAmxTpl("'%0-20.7f'", 0.0);
    AssertStrEq("%0-20.7f", sink.GetLastLogMsg(), "'0.0000000           '");
    logger.InfoAmxTpl("'%0-20.7f'", -12345.968750);
    AssertStrEq("%0-20.7f", sink.GetLastLogMsg(), "'-12345.9687500      '");

    logger.InfoAmxTpl("'%-020.f'", 12345.968750);
    AssertStrEq("%-020.f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoAmxTpl("'%-020.f'", 0.0);
    AssertStrEq("%-020.f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoAmxTpl("'%-020.f'", -12345.968750);
    AssertStrEq("%-020.f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoAmxTpl("'%-020.0f'", 12345.968750);
    AssertStrEq("%-020.0f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoAmxTpl("'%-020.0f'", 0.0);
    AssertStrEq("%-020.0f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoAmxTpl("'%-020.0f'", -12345.968750);
    AssertStrEq("%-020.0f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoAmxTpl("'%-020.2f'", 12345.968750);
    AssertStrEq("%-020.2f", sink.GetLastLogMsg(), "'12345.96            '");
    logger.InfoAmxTpl("'%-020.2f'", 0.0);
    AssertStrEq("%-020.2f", sink.GetLastLogMsg(), "'0.00                '");
    logger.InfoAmxTpl("'%-020.2f'", -12345.968750);
    AssertStrEq("%-020.2f", sink.GetLastLogMsg(), "'-12345.96           '");

    logger.InfoAmxTpl("'%-020.7f'", 12345.968750);
    AssertStrEq("%-020.7f", sink.GetLastLogMsg(), "'12345.9687500       '");
    logger.InfoAmxTpl("'%-020.7f'", 0.0);
    AssertStrEq("%-020.7f", sink.GetLastLogMsg(), "'0.0000000           '");
    logger.InfoAmxTpl("'%-020.7f'", -12345.968750);
    AssertStrEq("%-020.7f", sink.GetLastLogMsg(), "'-12345.9687500      '");

    logger.Close();
    sink.Close();
}

void TestFloatEx()
{
    // flags: [-] / [0]
    // %[flags][width][.precision]f
    SetTestContext("Test Float Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%f'", 12345.968750);
    AssertStrEq("%f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%f'", 0.0);
    AssertStrEq("%f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%f'", -12345.968750);
    AssertStrEq("%f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%-f'", 12345.968750);
    AssertStrEq("%-f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%-f'", 0.0);
    AssertStrEq("%-f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%-f'", -12345.968750);
    AssertStrEq("%-f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%0f'", 12345.968750);
    AssertStrEq("%0f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%0f'", 0.0);
    AssertStrEq("%0f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%0f'", -12345.968750);
    AssertStrEq("%0f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%-0f'", 12345.968750);
    AssertStrEq("%-0f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%-0f'", 0.0);
    AssertStrEq("%-0f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%-0f'", -12345.968750);
    AssertStrEq("%-0f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%0-f'", 12345.968750);
    AssertStrEq("%0-f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%0-f'", 0.0);
    AssertStrEq("%0-f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%0-f'", -12345.968750);
    AssertStrEq("%0-f", sink.GetLastLogMsg(), "'-12345.968750'");

    // width
    logger.InfoEx("'%3f'", 12345.968750);
    AssertStrEq("%3f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%3f'", 0.0);
    AssertStrEq("%3f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%3f'", -12345.968750);
    AssertStrEq("%3f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%20f'", 12345.968750);
    AssertStrEq("%20f", sink.GetLastLogMsg(), "'        12345.968750'");
    logger.InfoEx("'%20f'", 0.0);
    AssertStrEq("%20f", sink.GetLastLogMsg(), "'            0.000000'");
    logger.InfoEx("'%20f'", -12345.968750);
    AssertStrEq("%20f", sink.GetLastLogMsg(), "'       -12345.968750'");

    // flag & width
    logger.InfoEx("'%-3f'", 12345.968750);
    AssertStrEq("%-3f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%-3f'", 0.0);
    AssertStrEq("%-3f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%-3f'", -12345.968750);
    AssertStrEq("%-3f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%-20f'", 12345.968750);
    AssertStrEq("%-20f", sink.GetLastLogMsg(), "'12345.968750        '");
    logger.InfoEx("'%-20f'", 0.0);
    AssertStrEq("%-20f", sink.GetLastLogMsg(), "'0.000000            '");
    logger.InfoEx("'%-20f'", -12345.968750);
    AssertStrEq("%-20f", sink.GetLastLogMsg(), "'-12345.968750       '");

    logger.InfoEx("'%03f'", 12345.968750);
    AssertStrEq("%03f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%03f'", 0.0);
    AssertStrEq("%03f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%03f'", -12345.968750);
    AssertStrEq("%03f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%020f'", 12345.968750);
    AssertStrEq("%020f", sink.GetLastLogMsg(), "'0000000012345.968750'");
    logger.InfoEx("'%020f'", 0.0);
    AssertStrEq("%020f", sink.GetLastLogMsg(), "'0000000000000.000000'");
    logger.InfoEx("'%020f'", -12345.968750);
    AssertStrEq("%020f", sink.GetLastLogMsg(), "'-000000012345.968750'");

    logger.InfoEx("'%-03f'", 12345.968750);
    AssertStrEq("%-03f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%-03f'", 0.0);
    AssertStrEq("%-03f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%-03f'", -12345.968750);
    AssertStrEq("%-03f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%-020f'", 12345.968750);
    AssertStrEq("%-020f", sink.GetLastLogMsg(), "'12345.968750        '");
    logger.InfoEx("'%-020f'", 0.0);
    AssertStrEq("%-020f", sink.GetLastLogMsg(), "'0.000000            '");
    logger.InfoEx("'%-020f'", -12345.968750);
    AssertStrEq("%-020f", sink.GetLastLogMsg(), "'-12345.968750       '");

    logger.InfoEx("'%0-3f'", 12345.968750);
    AssertStrEq("%0-3f", sink.GetLastLogMsg(), "'12345.968750'");
    logger.InfoEx("'%0-3f'", 0.0);
    AssertStrEq("%0-3f", sink.GetLastLogMsg(), "'0.000000'");
    logger.InfoEx("'%0-3f'", -12345.968750);
    AssertStrEq("%0-3f", sink.GetLastLogMsg(), "'-12345.968750'");

    logger.InfoEx("'%0-20f'", 12345.968750);
    AssertStrEq("%0-20f", sink.GetLastLogMsg(), "'12345.968750        '");
    logger.InfoEx("'%0-20f'", 0.0);
    AssertStrEq("%0-20f", sink.GetLastLogMsg(), "'0.000000            '");
    logger.InfoEx("'%0-20f'", -12345.968750);
    AssertStrEq("%0-20f", sink.GetLastLogMsg(), "'-12345.968750       '");

    // prec
    logger.InfoEx("'%.f'", 12345.968750);
    AssertStrEq("%.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%.f'", 0.0);
    AssertStrEq("%.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%.f'", -12345.968750);
    AssertStrEq("%.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%.0f'", 12345.968750);
    AssertStrEq("%.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%.0f'", 0.0);
    AssertStrEq("%.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%.0f'", -12345.968750);
    AssertStrEq("%.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%.2f'", 12345.968750);
    AssertStrEq("%.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%.2f'", 0.0);
    AssertStrEq("%.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%.2f'", -12345.968750);
    AssertStrEq("%.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%.7f'", 12345.968750);
    AssertStrEq("%.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%.7f'", 0.0);
    AssertStrEq("%.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%.7f'", -12345.968750);
    AssertStrEq("%.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    // flag & prec
    logger.InfoEx("'%-.f'", 12345.968750);
    AssertStrEq("%-.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-.f'", 0.0);
    AssertStrEq("%-.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-.f'", -12345.968750);
    AssertStrEq("%-.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-.0f'", 12345.968750);
    AssertStrEq("%-.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-.0f'", 0.0);
    AssertStrEq("%-.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-.0f'", -12345.968750);
    AssertStrEq("%-.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-.2f'", 12345.968750);
    AssertStrEq("%-.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%-.2f'", 0.0);
    AssertStrEq("%-.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%-.2f'", -12345.968750);
    AssertStrEq("%-.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%-.7f'", 12345.968750);
    AssertStrEq("%-.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%-.7f'", 0.0);
    AssertStrEq("%-.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%-.7f'", -12345.968750);
    AssertStrEq("%-.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoEx("'%0.f'", 12345.968750);
    AssertStrEq("%0.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%0.f'", 0.0);
    AssertStrEq("%0.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0.f'", -12345.968750);
    AssertStrEq("%0.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%0.0f'", 12345.968750);
    AssertStrEq("%0.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%0.0f'", 0.0);
    AssertStrEq("%0.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0.0f'", -12345.968750);
    AssertStrEq("%0.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%0.2f'", 12345.968750);
    AssertStrEq("%0.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%0.2f'", 0.0);
    AssertStrEq("%0.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%0.2f'", -12345.968750);
    AssertStrEq("%0.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%0.7f'", 12345.968750);
    AssertStrEq("%0.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%0.7f'", 0.0);
    AssertStrEq("%0.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%0.7f'", -12345.968750);
    AssertStrEq("%0.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoEx("'%0-.f'", 12345.968750);
    AssertStrEq("%0-.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%0-.f'", 0.0);
    AssertStrEq("%0-.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0-.f'", -12345.968750);
    AssertStrEq("%0-.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%0-.0f'", 12345.968750);
    AssertStrEq("%0-.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%0-.0f'", 0.0);
    AssertStrEq("%0-.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0-.0f'", -12345.968750);
    AssertStrEq("%0-.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%0-.2f'", 12345.968750);
    AssertStrEq("%0-.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%0-.2f'", 0.0);
    AssertStrEq("%0-.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%0-.2f'", -12345.968750);
    AssertStrEq("%0-.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%0-.7f'", 12345.968750);
    AssertStrEq("%0-.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%0-.7f'", 0.0);
    AssertStrEq("%0-.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%0-.7f'", -12345.968750);
    AssertStrEq("%0-.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoEx("'%-0.f'", 12345.968750);
    AssertStrEq("%-0.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-0.f'", 0.0);
    AssertStrEq("%-0.f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-0.f'", -12345.968750);
    AssertStrEq("%-0.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-0.0f'", 12345.968750);
    AssertStrEq("%-0.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-0.0f'", 0.0);
    AssertStrEq("%-0.0f", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-0.0f'", -12345.968750);
    AssertStrEq("%-0.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-0.2f'", 12345.968750);
    AssertStrEq("%-0.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%-0.2f'", 0.0);
    AssertStrEq("%-0.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%-0.2f'", -12345.968750);
    AssertStrEq("%-0.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%-0.7f'", 12345.968750);
    AssertStrEq("%-0.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%-0.7f'", 0.0);
    AssertStrEq("%-0.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%-0.7f'", -12345.968750);
    AssertStrEq("%-0.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    // flag & width & prec
    logger.InfoEx("'%-3.f'", 12345.968750);
    AssertStrEq("%-3.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-3.f'", 0.0);
    AssertStrEq("%-3.f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoEx("'%-3.f'", -12345.968750);
    AssertStrEq("%-3.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-3.0f'", 12345.968750);
    AssertStrEq("%-3.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-3.0f'", 0.0);
    AssertStrEq("%-3.0f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoEx("'%-3.0f'", -12345.968750);
    AssertStrEq("%-3.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-3.2f'", 12345.968750);
    AssertStrEq("%-3.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%-3.2f'", 0.0);
    AssertStrEq("%-3.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%-3.2f'", -12345.968750);
    AssertStrEq("%-3.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%-3.7f'", 12345.968750);
    AssertStrEq("%-3.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%-3.7f'", 0.0);
    AssertStrEq("%-3.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%-3.7f'", -12345.968750);
    AssertStrEq("%-3.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoEx("'%-03.f'", 12345.968750);
    AssertStrEq("%-03.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-03.f'", 0.0);
    AssertStrEq("%-03.f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoEx("'%-03.f'", -12345.968750);
    AssertStrEq("%-03.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-03.0f'", 12345.968750);
    AssertStrEq("%-03.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%-03.0f'", 0.0);
    AssertStrEq("%-03.0f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoEx("'%-03.0f'", -12345.968750);
    AssertStrEq("%-03.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%-03.2f'", 12345.968750);
    AssertStrEq("%-03.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%-03.2f'", 0.0);
    AssertStrEq("%-03.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%-03.2f'", -12345.968750);
    AssertStrEq("%-03.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%-03.7f'", 12345.968750);
    AssertStrEq("%-03.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%-03.7f'", 0.0);
    AssertStrEq("%-03.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%-03.7f'", -12345.968750);
    AssertStrEq("%-03.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoEx("'%0-3.f'", 12345.968750);
    AssertStrEq("%0-3.f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%0-3.f'", 0.0);
    AssertStrEq("%0-3.f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoEx("'%0-3.f'", -12345.968750);
    AssertStrEq("%0-3.f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%0-3.0f'", 12345.968750);
    AssertStrEq("%0-3.0f", sink.GetLastLogMsg(), "'12345'");
    logger.InfoEx("'%0-3.0f'", 0.0);
    AssertStrEq("%0-3.0f", sink.GetLastLogMsg(), "'0  '");
    logger.InfoEx("'%0-3.0f'", -12345.968750);
    AssertStrEq("%0-3.0f", sink.GetLastLogMsg(), "'-12345'");

    logger.InfoEx("'%0-3.2f'", 12345.968750);
    AssertStrEq("%0-3.2f", sink.GetLastLogMsg(), "'12345.96'");
    logger.InfoEx("'%0-3.2f'", 0.0);
    AssertStrEq("%0-3.2f", sink.GetLastLogMsg(), "'0.00'");
    logger.InfoEx("'%0-3.2f'", -12345.968750);
    AssertStrEq("%0-3.2f", sink.GetLastLogMsg(), "'-12345.96'");

    logger.InfoEx("'%0-3.7f'", 12345.968750);
    AssertStrEq("%0-3.7f", sink.GetLastLogMsg(), "'12345.9687500'");
    logger.InfoEx("'%0-3.7f'", 0.0);
    AssertStrEq("%0-3.7f", sink.GetLastLogMsg(), "'0.0000000'");
    logger.InfoEx("'%0-3.7f'", -12345.968750);
    AssertStrEq("%0-3.7f", sink.GetLastLogMsg(), "'-12345.9687500'");

    logger.InfoEx("'%-20.f'", 12345.968750);
    AssertStrEq("%-20.f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoEx("'%-20.f'", 0.0);
    AssertStrEq("%-20.f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoEx("'%-20.f'", -12345.968750);
    AssertStrEq("%-20.f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoEx("'%-20.0f'", 12345.968750);
    AssertStrEq("%-20.0f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoEx("'%-20.0f'", 0.0);
    AssertStrEq("%-20.0f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoEx("'%-20.0f'", -12345.968750);
    AssertStrEq("%-20.0f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoEx("'%-20.2f'", 12345.968750);
    AssertStrEq("%-20.2f", sink.GetLastLogMsg(), "'12345.96            '");
    logger.InfoEx("'%-20.2f'", 0.0);
    AssertStrEq("%-20.2f", sink.GetLastLogMsg(), "'0.00                '");
    logger.InfoEx("'%-20.2f'", -12345.968750);
    AssertStrEq("%-20.2f", sink.GetLastLogMsg(), "'-12345.96           '");

    logger.InfoEx("'%-20.7f'", 12345.968750);
    AssertStrEq("%-20.7f", sink.GetLastLogMsg(), "'12345.9687500       '");
    logger.InfoEx("'%-20.7f'", 0.0);
    AssertStrEq("%-20.7f", sink.GetLastLogMsg(), "'0.0000000           '");
    logger.InfoEx("'%-20.7f'", -12345.968750);
    AssertStrEq("%-20.7f", sink.GetLastLogMsg(), "'-12345.9687500      '");

    logger.InfoEx("'%020.f'", 12345.968750);
    AssertStrEq("%020.f", sink.GetLastLogMsg(), "'00000000000000012345'");
    logger.InfoEx("'%020.f'", 0.0);
    AssertStrEq("%020.f", sink.GetLastLogMsg(), "'00000000000000000000'");
    logger.InfoEx("'%020.f'", -12345.968750);
    AssertStrEq("%020.f", sink.GetLastLogMsg(), "'-0000000000000012345'");

    logger.InfoEx("'%020.0f'", 12345.968750);
    AssertStrEq("%020.0f", sink.GetLastLogMsg(), "'00000000000000012345'");
    logger.InfoEx("'%020.0f'", 0.0);
    AssertStrEq("%020.0f", sink.GetLastLogMsg(), "'00000000000000000000'");
    logger.InfoEx("'%020.0f'", -12345.968750);
    AssertStrEq("%020.0f", sink.GetLastLogMsg(), "'-0000000000000012345'");

    logger.InfoEx("'%020.2f'", 12345.968750);
    AssertStrEq("%020.2f", sink.GetLastLogMsg(), "'00000000000012345.96'");
    logger.InfoEx("'%020.2f'", 0.0);
    AssertStrEq("%020.2f", sink.GetLastLogMsg(), "'00000000000000000.00'");
    logger.InfoEx("'%020.2f'", -12345.968750);
    AssertStrEq("%020.2f", sink.GetLastLogMsg(), "'-0000000000012345.96'");

    logger.InfoEx("'%020.7f'", 12345.968750);
    AssertStrEq("%020.7f", sink.GetLastLogMsg(), "'000000012345.9687500'");
    logger.InfoEx("'%020.7f'", 0.0);
    AssertStrEq("%020.7f", sink.GetLastLogMsg(), "'000000000000.0000000'");
    logger.InfoEx("'%020.7f'", -12345.968750);
    AssertStrEq("%020.7f", sink.GetLastLogMsg(), "'-00000012345.9687500'");

    logger.InfoEx("'%0-20.f'", 12345.968750);
    AssertStrEq("%0-20.f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoEx("'%0-20.f'", 0.0);
    AssertStrEq("%0-20.f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoEx("'%0-20.f'", -12345.968750);
    AssertStrEq("%0-20.f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoEx("'%0-20.0f'", 12345.968750);
    AssertStrEq("%0-20.0f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoEx("'%0-20.0f'", 0.0);
    AssertStrEq("%0-20.0f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoEx("'%0-20.0f'", -12345.968750);
    AssertStrEq("%0-20.0f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoEx("'%0-20.2f'", 12345.968750);
    AssertStrEq("%0-20.2f", sink.GetLastLogMsg(), "'12345.96            '");
    logger.InfoEx("'%0-20.2f'", 0.0);
    AssertStrEq("%0-20.2f", sink.GetLastLogMsg(), "'0.00                '");
    logger.InfoEx("'%0-20.2f'", -12345.968750);
    AssertStrEq("%0-20.2f", sink.GetLastLogMsg(), "'-12345.96           '");

    logger.InfoEx("'%0-20.7f'", 12345.968750);
    AssertStrEq("%0-20.7f", sink.GetLastLogMsg(), "'12345.9687500       '");
    logger.InfoEx("'%0-20.7f'", 0.0);
    AssertStrEq("%0-20.7f", sink.GetLastLogMsg(), "'0.0000000           '");
    logger.InfoEx("'%0-20.7f'", -12345.968750);
    AssertStrEq("%0-20.7f", sink.GetLastLogMsg(), "'-12345.9687500      '");

    logger.InfoEx("'%-020.f'", 12345.968750);
    AssertStrEq("%-020.f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoEx("'%-020.f'", 0.0);
    AssertStrEq("%-020.f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoEx("'%-020.f'", -12345.968750);
    AssertStrEq("%-020.f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoEx("'%-020.0f'", 12345.968750);
    AssertStrEq("%-020.0f", sink.GetLastLogMsg(), "'12345               '");
    logger.InfoEx("'%-020.0f'", 0.0);
    AssertStrEq("%-020.0f", sink.GetLastLogMsg(), "'0                   '");
    logger.InfoEx("'%-020.0f'", -12345.968750);
    AssertStrEq("%-020.0f", sink.GetLastLogMsg(), "'-12345              '");

    logger.InfoEx("'%-020.2f'", 12345.968750);
    AssertStrEq("%-020.2f", sink.GetLastLogMsg(), "'12345.96            '");
    logger.InfoEx("'%-020.2f'", 0.0);
    AssertStrEq("%-020.2f", sink.GetLastLogMsg(), "'0.00                '");
    logger.InfoEx("'%-020.2f'", -12345.968750);
    AssertStrEq("%-020.2f", sink.GetLastLogMsg(), "'-12345.96           '");

    logger.InfoEx("'%-020.7f'", 12345.968750);
    AssertStrEq("%-020.7f", sink.GetLastLogMsg(), "'12345.9687500       '");
    logger.InfoEx("'%-020.7f'", 0.0);
    AssertStrEq("%-020.7f", sink.GetLastLogMsg(), "'0.0000000           '");
    logger.InfoEx("'%-020.7f'", -12345.968750);
    AssertStrEq("%-020.7f", sink.GetLastLogMsg(), "'-12345.9687500      '");

    logger.Close();
    sink.Close();
}


void TestBinary()
{
    TestBinaryAmxTpl();
    TestBinaryEx();
}

void TestBinaryAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]b
    SetTestContext("Test Binary AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%b", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0b", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-b", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-b", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0-b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-0b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-0b", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-0b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-0b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");
#endif

    // width
    logger.InfoAmxTpl("'%5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%5b", sink.GetLastLogMsg(), "'    0'");
    logger.InfoAmxTpl("'%5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%35b", sink.GetLastLogMsg(), "'                                  0'");
    logger.InfoAmxTpl("'%35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%35b", sink.GetLastLogMsg(), "'    " ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%35b", sink.GetLastLogMsg(), "'    " ... TEST_BINARY_VALUE3 ... "'");

    // flag & width
    logger.InfoAmxTpl("'%05b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%05b", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%05b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%05b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%035b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%035b", sink.GetLastLogMsg(), "'0000" ... TEST_BINARY_VALUE1 ... "'");
    logger.InfoAmxTpl("'%035b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%035b", sink.GetLastLogMsg(), "'0000" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%035b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%035b", sink.GetLastLogMsg(), "'0000" ... TEST_BINARY_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-5b", sink.GetLastLogMsg(), "'0    '");
    logger.InfoAmxTpl("'%-5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-35b", sink.GetLastLogMsg(), "'0                                  '");
    logger.InfoAmxTpl("'%-35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "    '");
    logger.InfoAmxTpl("'%-35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "    '");

    logger.InfoAmxTpl("'%-05b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-05b", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%-05b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-05b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-035b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-035b", sink.GetLastLogMsg(), "'00000000000000000000000000000000000'");
    logger.InfoAmxTpl("'%-035b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-035b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "0000'");
    logger.InfoAmxTpl("'%-035b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-035b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "0000'");

    logger.InfoAmxTpl("'%0-5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-5b", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%0-5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-35b", sink.GetLastLogMsg(), "'00000000000000000000000000000000000'");
    logger.InfoAmxTpl("'%0-35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "0000'");
    logger.InfoAmxTpl("'%0-35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "0000'");
#endif

    logger.Close();
    sink.Close();
}

void TestBinaryEx()
{
    // flags: [-] / [0]
    // %[flags][width]b
    SetTestContext("Test Binary Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%b", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%0b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0b", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%0b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%-b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-b", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%-b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%0-b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-b", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0-b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%0-b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%-0b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-0b", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-0b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%-0b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-0b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    // width
    logger.InfoEx("'%5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%5b", sink.GetLastLogMsg(), "'    0'");
    logger.InfoEx("'%5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%35b", sink.GetLastLogMsg(), "'                                  0'");
    logger.InfoEx("'%35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%35b", sink.GetLastLogMsg(), "'    " ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%35b", sink.GetLastLogMsg(), "'    " ... TEST_BINARY_VALUE3 ... "'");

    // flag & width
    logger.InfoEx("'%05b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%05b", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%05b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%05b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%035b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%035b", sink.GetLastLogMsg(), "'0000" ... TEST_BINARY_VALUE1 ... "'");
    logger.InfoEx("'%035b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%035b", sink.GetLastLogMsg(), "'0000" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%035b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%035b", sink.GetLastLogMsg(), "'0000" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%-5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-5b", sink.GetLastLogMsg(), "'0    '");
    logger.InfoEx("'%-5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%-5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%-35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-35b", sink.GetLastLogMsg(), "'0                                  '");
    logger.InfoEx("'%-35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "    '");
    logger.InfoEx("'%-35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "    '");

    logger.InfoEx("'%-05b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-05b", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%-05b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%-05b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-05b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%-035b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%-035b", sink.GetLastLogMsg(), "'00000000000000000000000000000000000'");
    logger.InfoEx("'%-035b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%-035b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "0000'");
    logger.InfoEx("'%-035b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%-035b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "0000'");

    logger.InfoEx("'%0-5b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-5b", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%0-5b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "'");
    logger.InfoEx("'%0-5b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-5b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "'");

    logger.InfoEx("'%0-35b'", StringToInt(TEST_BINARY_VALUE1, 2));
    AssertStrEq("%0-35b", sink.GetLastLogMsg(), "'00000000000000000000000000000000000'");
    logger.InfoEx("'%0-35b'", StringToInt(TEST_BINARY_VALUE2, 2));
    AssertStrEq("%0-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE2 ... "0000'");
    logger.InfoEx("'%0-35b'", StringToInt(TEST_BINARY_VALUE3, 2));
    AssertStrEq("%0-35b", sink.GetLastLogMsg(), "'" ... TEST_BINARY_VALUE3 ... "0000'");

    logger.Close();
    sink.Close();
}


void TestUInt()
{
    TestUIntAmxTpl();
    TestUIntEx();
}

void TestUIntAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]u
    SetTestContext("Test UInt AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%0u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%-u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%0-u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-0u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%-0u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-0u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");
#endif

    // width
    logger.InfoAmxTpl("'%5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%5u", sink.GetLastLogMsg(), "'    0'");
    logger.InfoAmxTpl("'%5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%15u", sink.GetLastLogMsg(), "'              0'");
    logger.InfoAmxTpl("'%15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%15u", sink.GetLastLogMsg(), "'     " ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%15u", sink.GetLastLogMsg(), "'     " ... TEST_UINT_VALUE3 ... "'");

    // flag & width
    logger.InfoAmxTpl("'%05u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%05u", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%05u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%05u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%015u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%015u", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%015u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%015u", sink.GetLastLogMsg(), "'00000" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%015u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%015u", sink.GetLastLogMsg(), "'00000" ... TEST_UINT_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-5u", sink.GetLastLogMsg(), "'0    '");
    logger.InfoAmxTpl("'%-5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-15u", sink.GetLastLogMsg(), "'0              '");
    logger.InfoAmxTpl("'%-15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "     '");
    logger.InfoAmxTpl("'%-15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "     '");

    logger.InfoAmxTpl("'%-05u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-05u", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%-05u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-05u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-015u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-015u", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%-015u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-015u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "00000'");
    logger.InfoAmxTpl("'%-015u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-015u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "00000'");

    logger.InfoAmxTpl("'%0-5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-5u", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%0-5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-15u", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%0-15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "00000'");
    logger.InfoAmxTpl("'%0-15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "00000'");
#endif

    logger.Close();
    sink.Close();
}

void TestUIntEx()
{
    // flags: [-] / [0]
    // %[flags][width]u
    SetTestContext("Test UInt Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoEx("'%u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%0u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoEx("'%0u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%0u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%-u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoEx("'%-u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%-u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%0-u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoEx("'%0-u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%0-u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%-0u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE1 ... "'");
    logger.InfoEx("'%-0u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%-0u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-0u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    // width
    logger.InfoEx("'%5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%5u", sink.GetLastLogMsg(), "'    0'");
    logger.InfoEx("'%5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%15u", sink.GetLastLogMsg(), "'              0'");
    logger.InfoEx("'%15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%15u", sink.GetLastLogMsg(), "'     " ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%15u", sink.GetLastLogMsg(), "'     " ... TEST_UINT_VALUE3 ... "'");

    // flag & width
    logger.InfoEx("'%05u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%05u", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%05u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%05u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%015u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%015u", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%015u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%015u", sink.GetLastLogMsg(), "'00000" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%015u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%015u", sink.GetLastLogMsg(), "'00000" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%-5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-5u", sink.GetLastLogMsg(), "'0    '");
    logger.InfoEx("'%-5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%-5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%-15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-15u", sink.GetLastLogMsg(), "'0              '");
    logger.InfoEx("'%-15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "     '");
    logger.InfoEx("'%-15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "     '");

    logger.InfoEx("'%-05u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-05u", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%-05u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%-05u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-05u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%-015u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%-015u", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%-015u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%-015u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "00000'");
    logger.InfoEx("'%-015u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%-015u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "00000'");

    logger.InfoEx("'%0-5u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-5u", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%0-5u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "'");
    logger.InfoEx("'%0-5u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-5u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "'");

    logger.InfoEx("'%0-15u'", StringToInt(TEST_UINT_VALUE1));
    AssertStrEq("%0-15u", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%0-15u'", StringToInt(TEST_UINT_VALUE2));
    AssertStrEq("%0-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE2 ... "00000'");
    logger.InfoEx("'%0-15u'", StringToInt(TEST_UINT_VALUE3));
    AssertStrEq("%0-15u", sink.GetLastLogMsg(), "'" ... TEST_UINT_VALUE3 ... "00000'");

    logger.Close();
    sink.Close();
}


void TestInt()
{
    TestIntAmxTpl();
    TestIntEx();
}

void TestIntAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]d
    SetTestContext("Test Int AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%0d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%-d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%0-d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-0d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoAmxTpl("'%-0d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-0d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");
#endif

    // width
    logger.InfoAmxTpl("'%5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%5d", sink.GetLastLogMsg(), "'    0'");
    logger.InfoAmxTpl("'%5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%15d", sink.GetLastLogMsg(), "'              0'");
    logger.InfoAmxTpl("'%15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%15d", sink.GetLastLogMsg(), "'     " ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%15d", sink.GetLastLogMsg(), "'    " ... TEST_INT_VALUE3 ... "'");

    // flag & width
    logger.InfoAmxTpl("'%05d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%05d", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%05d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%05d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%015d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%015d", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%015d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%015d", sink.GetLastLogMsg(), "'00000" ... TEST_INT_VALUE2 ... "'");
    // ! Warn 填充字符在符号之前
    logger.InfoAmxTpl("'%015d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%015d", sink.GetLastLogMsg(), "'0000-2147483648'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-5d", sink.GetLastLogMsg(), "'0    '");
    logger.InfoAmxTpl("'%-5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-15d", sink.GetLastLogMsg(), "'0              '");
    logger.InfoAmxTpl("'%-15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "     '");
    logger.InfoAmxTpl("'%-15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "    '");

    logger.InfoAmxTpl("'%-05d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-05d", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%-05d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-05d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-015d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-015d", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%-015d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-015d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "00000'");
    logger.InfoAmxTpl("'%-015d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-015d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "0000'");

    logger.InfoAmxTpl("'%0-5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-5d", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%0-5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-15d", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%0-15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "00000'");
    logger.InfoAmxTpl("'%0-15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "0000'");
#endif


    logger.Close();
    sink.Close();
}

void TestIntEx()
{
    // flags: [-] / [0]
    // %[flags][width]d
    SetTestContext("Test Int Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoEx("'%d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%0d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoEx("'%0d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%0d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%-d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoEx("'%-d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%-d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%0-d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoEx("'%0-d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%0-d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%-0d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE1 ... "'");
    logger.InfoEx("'%-0d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%-0d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-0d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    // width
    logger.InfoEx("'%5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%5d", sink.GetLastLogMsg(), "'    0'");
    logger.InfoEx("'%5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%15d", sink.GetLastLogMsg(), "'              0'");
    logger.InfoEx("'%15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%15d", sink.GetLastLogMsg(), "'     " ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%15d", sink.GetLastLogMsg(), "'    " ... TEST_INT_VALUE3 ... "'");

    // flag & width
    logger.InfoEx("'%05d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%05d", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%05d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%05d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%015d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%015d", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%015d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%015d", sink.GetLastLogMsg(), "'00000" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%015d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%015d", sink.GetLastLogMsg(), "'-00002147483648'");

    logger.InfoEx("'%-5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-5d", sink.GetLastLogMsg(), "'0    '");
    logger.InfoEx("'%-5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%-5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%-15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-15d", sink.GetLastLogMsg(), "'0              '");
    logger.InfoEx("'%-15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "     '");
    logger.InfoEx("'%-15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "    '");

    logger.InfoEx("'%-05d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-05d", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%-05d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%-05d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-05d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%-015d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%-015d", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%-015d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%-015d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "00000'");
    logger.InfoEx("'%-015d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%-015d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "0000'");

    logger.InfoEx("'%0-5d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-5d", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%0-5d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "'");
    logger.InfoEx("'%0-5d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-5d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "'");

    logger.InfoEx("'%0-15d'", StringToInt(TEST_INT_VALUE1));
    AssertStrEq("%0-15d", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%0-15d'", StringToInt(TEST_INT_VALUE2));
    AssertStrEq("%0-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE2 ... "00000'");
    logger.InfoEx("'%0-15d'", StringToInt(TEST_INT_VALUE3));
    AssertStrEq("%0-15d", sink.GetLastLogMsg(), "'" ... TEST_INT_VALUE3 ... "0000'");

    logger.Close();
    sink.Close();
}


void TestHex()
{
    TestHexUpperAmxTpl();
    TestHexUpperEx();

    TestHexLowerAmxTpl();
    TestHexLowerEx();
}

void TestHexUpperAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]X
    SetTestContext("Test Hex Upper AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%X", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0X", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-X", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-X", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-0X", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");
#endif

    // width
    logger.InfoAmxTpl("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%5X", sink.GetLastLogMsg(), "'    0'");
    logger.InfoAmxTpl("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%15X", sink.GetLastLogMsg(), "'              0'");
    logger.InfoAmxTpl("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%15X", sink.GetLastLogMsg(), "'          " ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%15X", sink.GetLastLogMsg(), "'       " ... TEST_HEX_UPPER_VALUE3 ... "'");

    // flag & width
    logger.InfoAmxTpl("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%05X", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%015X", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%015X", sink.GetLastLogMsg(), "'0000000000" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%015X", sink.GetLastLogMsg(), "'0000000" ... TEST_HEX_UPPER_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-5X", sink.GetLastLogMsg(), "'0    '");
    logger.InfoAmxTpl("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-15X", sink.GetLastLogMsg(), "'0              '");
    logger.InfoAmxTpl("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "          '");
    logger.InfoAmxTpl("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "       '");

    logger.InfoAmxTpl("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-05X", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-015X", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-015X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "0000000000'");
    logger.InfoAmxTpl("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-015X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "0000000'");

    logger.InfoAmxTpl("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-5X", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-15X", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "0000000000'");
    logger.InfoAmxTpl("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "0000000'");
#endif

    logger.Close();
    sink.Close();
}

void TestHexUpperEx()
{
    // flags: [-] / [0]
    // %[flags][width]X
    SetTestContext("Test Hex Upper Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%X", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0X", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-X", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-X", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-0X", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-0X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    // width
    logger.InfoEx("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%5X", sink.GetLastLogMsg(), "'    0'");
    logger.InfoEx("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%15X", sink.GetLastLogMsg(), "'              0'");
    logger.InfoEx("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%15X", sink.GetLastLogMsg(), "'          " ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%15X", sink.GetLastLogMsg(), "'       " ... TEST_HEX_UPPER_VALUE3 ... "'");

    // flag & width
    logger.InfoEx("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%05X", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%015X", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%015X", sink.GetLastLogMsg(), "'0000000000" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%015X", sink.GetLastLogMsg(), "'0000000" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-5X", sink.GetLastLogMsg(), "'0    '");
    logger.InfoEx("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-15X", sink.GetLastLogMsg(), "'0              '");
    logger.InfoEx("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "          '");
    logger.InfoEx("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "       '");

    logger.InfoEx("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-05X", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-05X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%-015X", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%-015X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "0000000000'");
    logger.InfoEx("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%-015X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "0000000'");

    logger.InfoEx("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-5X", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "'");
    logger.InfoEx("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-5X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "'");

    logger.InfoEx("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE1, 16));
    AssertStrEq("%0-15X", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE2, 16));
    AssertStrEq("%0-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE2 ... "0000000000'");
    logger.InfoEx("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE3, 16));
    AssertStrEq("%0-15X", sink.GetLastLogMsg(), "'" ... TEST_HEX_UPPER_VALUE3 ... "0000000'");

    logger.Close();
    sink.Close();
}


void TestHexLowerAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]x
    SetTestContext("Test Hex Lower AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%x", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0x", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-x", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-x", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-0x", sink.GetLastLogMsg(), "'0'");
    logger.InfoAmxTpl("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");
#endif

    // width
    logger.InfoAmxTpl("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%5x", sink.GetLastLogMsg(), "'    0'");
    logger.InfoAmxTpl("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%15x", sink.GetLastLogMsg(), "'              0'");
    logger.InfoAmxTpl("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%15x", sink.GetLastLogMsg(), "'          " ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%15x", sink.GetLastLogMsg(), "'       " ... TEST_HEX_LOWER_VALUE3 ... "'");

    // flag & width
    logger.InfoAmxTpl("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%05x", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%015x", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%015x", sink.GetLastLogMsg(), "'0000000000" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%015x", sink.GetLastLogMsg(), "'0000000" ... TEST_HEX_LOWER_VALUE3 ... "'");

#if SOURCEMOD_V_MINOR >= 13
    // ! Warn https://github.com/alliedmodders/sourcemod/pull/2255
    logger.InfoAmxTpl("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-5x", sink.GetLastLogMsg(), "'0    '");
    logger.InfoAmxTpl("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-15x", sink.GetLastLogMsg(), "'0              '");
    logger.InfoAmxTpl("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "          '");
    logger.InfoAmxTpl("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "       '");

    logger.InfoAmxTpl("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-05x", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-015x", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-015x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "0000000000'");
    logger.InfoAmxTpl("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-015x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "0000000'");

    logger.InfoAmxTpl("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-5x", sink.GetLastLogMsg(), "'00000'");
    logger.InfoAmxTpl("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoAmxTpl("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoAmxTpl("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-15x", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoAmxTpl("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "0000000000'");
    logger.InfoAmxTpl("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "0000000'");
#endif

    logger.Close();
    sink.Close();
}

void TestHexLowerEx()
{
    // flags: [-] / [0]
    // %[flags][width]x
    SetTestContext("Test Hex Lower Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%x", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0x", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-x", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-x", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-0x", sink.GetLastLogMsg(), "'0'");
    logger.InfoEx("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-0x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    // width
    logger.InfoEx("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%5x", sink.GetLastLogMsg(), "'    0'");
    logger.InfoEx("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%15x", sink.GetLastLogMsg(), "'              0'");
    logger.InfoEx("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%15x", sink.GetLastLogMsg(), "'          " ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%15x", sink.GetLastLogMsg(), "'       " ... TEST_HEX_LOWER_VALUE3 ... "'");

    // flag & width
    logger.InfoEx("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%05x", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%015x", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%015x", sink.GetLastLogMsg(), "'0000000000" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%015x", sink.GetLastLogMsg(), "'0000000" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-5x", sink.GetLastLogMsg(), "'0    '");
    logger.InfoEx("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-15x", sink.GetLastLogMsg(), "'0              '");
    logger.InfoEx("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "          '");
    logger.InfoEx("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "       '");

    logger.InfoEx("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-05x", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-05x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%-015x", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%-015x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "0000000000'");
    logger.InfoEx("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%-015x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "0000000'");

    logger.InfoEx("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-5x", sink.GetLastLogMsg(), "'00000'");
    logger.InfoEx("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "'");
    logger.InfoEx("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-5x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "'");

    logger.InfoEx("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE1, 16));
    AssertStrEq("%0-15x", sink.GetLastLogMsg(), "'000000000000000'");
    logger.InfoEx("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE2, 16));
    AssertStrEq("%0-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE2 ... "0000000000'");
    logger.InfoEx("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE3, 16));
    AssertStrEq("%0-15x", sink.GetLastLogMsg(), "'" ... TEST_HEX_LOWER_VALUE3 ... "0000000'");

    logger.Close();
    sink.Close();
}


void TestTranslates()
{
    TestTranslatesAmxTpl();
    TestTranslatesEx();
}

void TestTranslatesAmxTpl()
{
    // %T, LANGID
    SetTestContext("Test Translates AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY, GetLanguageByCode("en"));
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE ... "'");

    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY1, GetLanguageByCode("en"), TEST_TRANSLATES_DATA1);
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE1 ... "'");

    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY2, GetLanguageByCode("en"), StringToInt(TEST_TRANSLATES_DATA2));
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE2 ... "'");

    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY3, GetLanguageByCode("en"), TEST_TRANSLATES_DATA3_1, TEST_TRANSLATES_DATA3_2);
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE3 ... "'");

    logger.Close();
    sink.Close();
}

void TestTranslatesEx()
{
    // 内部是先获取 memory_buf 字符串，然后直接 append
    // %T, LANGID
    SetTestContext("Test Translates Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoEx("'%T'", TEST_TRANSLATES_KEY, GetLanguageByCode("en"));
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE ... "'");

    logger.InfoEx("'%T'", TEST_TRANSLATES_KEY1, GetLanguageByCode("en"), TEST_TRANSLATES_DATA1);
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE1 ... "'");

    logger.InfoEx("'%T'", TEST_TRANSLATES_KEY2, GetLanguageByCode("en"), StringToInt(TEST_TRANSLATES_DATA2));
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE2 ... "'");

    logger.InfoEx("'%T'", TEST_TRANSLATES_KEY3, GetLanguageByCode("en"), TEST_TRANSLATES_DATA3_1, TEST_TRANSLATES_DATA3_2);
    AssertStrEq("%T", sink.GetLastLogMsg(), "'" ... TEST_TRANSLATES_VALUE3 ... "'");


    logger.Close();
    sink.Close();
}


void TestSpecial()
{
    TestSpecialAmxTpl();
    TestSpecialEx();
}

void TestSpecialAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Special AmxTpl");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%N'", 0);
    AssertStrEq("%N", sink.GetLastLogMsg(), "'Console'");

    logger.InfoAmxTpl("'%L'", 0);
    AssertStrEq("%L", sink.GetLastLogMsg(), "'Console<0><Console><Console>'");

    logger.Close();
    sink.Close();
}

void TestSpecialEx()
{
    // 内部是先获取字符串，然后调用 AddString
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Special Ex");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoEx("'%N'", 0);
    AssertStrEq("%N", sink.GetLastLogMsg(), "'Console'");

    logger.InfoEx("'%L'", 0);
    AssertStrEq("%L", sink.GetLastLogMsg(), "'Console<0><Console><Console>'");

    logger.Close();
    sink.Close();
}
