#include <log4sp>

#include "test_sink"

#pragma semicolon 1
#pragma newdecls required

/**
 * Full syntax
 *      %[flags][width][.precision]specifier
 * flags
 *      [-] / [0]
 * NOTE
 *      SM 1.13.0.7198 修复了左对齐溢出的 BUG
 *      SM String 总是左对齐
 *      Log4sp 不支持 %t（会使用全局替代）
 *      二者 Float 类型左对齐时都只会在后方添加 ' ' (不会添加 '0')
 */

#define LOGGER_NAME             "test-format"
#define TEST_STRING_TEXT        "Some String Text"
#define TEST_FLOAT_VALUE        -12345.96875
#define TEST_BINARY_VALUE       "1110111011101110111011101110111"
#define TEST_UINT_VALUE         "1234567"
#define TEST_INT_VALUE          "-1234567"
#define TEST_HEX_UPPER_VALUE    "F1F88"
#define TEST_HEX_LOWER_VALUE    "77777777"

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

    // ServerConsoleSink sink = new ServerConsoleSink();
    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%% %%- %%. %%0 %%7 %%07 %%F %%c %%d'");
    logger.InfoEx("'%% %%- %%. %%0 %%7 %%07 %%F %%c %%d'");

    delete logger;
    // delete sink;

    ArrayList expected = GetCharExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);
    TestSink.Destroy();
    delete expected;
}

ArrayList GetCharExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    expected.PushString("'% %- %. %0 %7 %07 %F %c %d'");
    expected.PushString("'% %- %. %0 %7 %07 %F %c %d'");
    return expected;
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

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%s'", NULL_STRING);
    logger.InfoAmxTpl("'%s'", TEST_STRING_TEXT);
    logger.InfoAmxTpl("'%-s'", TEST_STRING_TEXT);
    logger.InfoAmxTpl("'%10s'", TEST_STRING_TEXT);
    logger.InfoAmxTpl("'%20s'", TEST_STRING_TEXT);
    logger.InfoAmxTpl("'%-10s'", TEST_STRING_TEXT);
    logger.InfoAmxTpl("'%-20s'", TEST_STRING_TEXT);

    delete logger;

    ArrayList expected = GetStringExpected(false);
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);
    TestSink.Destroy();
    delete expected;
}

void TestStringEx()
{
    // flags: [-]
    // %[flags][width]s
    SetTestContext("Test String Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoEx("'%s'", NULL_STRING);
    logger.InfoEx("'%s'", TEST_STRING_TEXT);
    logger.InfoEx("'%-s'", TEST_STRING_TEXT);
    logger.InfoEx("'%10s'", TEST_STRING_TEXT);
    logger.InfoEx("'%20s'", TEST_STRING_TEXT);
    logger.InfoEx("'%-10s'", TEST_STRING_TEXT);
    logger.InfoEx("'%-20s'", TEST_STRING_TEXT);

    delete logger;

    ArrayList expected = GetStringExpected(true);
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);
    TestSink.Destroy();
    delete expected;
}

ArrayList GetStringExpected(bool isEx)
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    expected.PushString("''");
    expected.PushString("'" ... TEST_STRING_TEXT ... "'");
    expected.PushString("'" ... TEST_STRING_TEXT ... "'");
    expected.PushString("'" ... TEST_STRING_TEXT ... "'");

    // AmxTpl 总是左对齐
    if (isEx)   expected.PushString("'    " ... TEST_STRING_TEXT ... "'");
    else        expected.PushString("'" ... TEST_STRING_TEXT ... "    '");

    expected.PushString("'" ... TEST_STRING_TEXT ... "'");
    expected.PushString("'" ... TEST_STRING_TEXT ... "    '");
    return expected;
}


void TestFloat()
{
    TestFloatAmxTpl();
    TestFloatEx();
}

void TestFloatAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width][.precision]specifier
    SetTestContext("Test Float AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoAmxTpl("'%f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-f'", TEST_FLOAT_VALUE);
    // width
    logger.InfoAmxTpl("'%5f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%20f'", TEST_FLOAT_VALUE);
    // flag & width
    logger.InfoAmxTpl("'%-5f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-20f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%05f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%020f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-05f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-020f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-5f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-20f'", TEST_FLOAT_VALUE);
    // prec
    logger.InfoAmxTpl("'%.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%.7f'", TEST_FLOAT_VALUE);
    // flag & prec
    logger.InfoAmxTpl("'%-.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-0.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-0.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-0.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-0.7f'", TEST_FLOAT_VALUE);
    // flag & width & prec
    logger.InfoAmxTpl("'%-5.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-5.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-5.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-5.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%05.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%05.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%05.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%05.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-5.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-5.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-5.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-5.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-05.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-05.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-05.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-05.7f'", TEST_FLOAT_VALUE);

    logger.InfoAmxTpl("'%-20.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-20.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-20.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-20.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%020.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%020.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%020.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%020.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-20.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-20.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-20.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%0-20.7f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-020.f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-020.0f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-020.2f'", TEST_FLOAT_VALUE);
    logger.InfoAmxTpl("'%-020.7f'", TEST_FLOAT_VALUE);

    ArrayList expected = GetFloatExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
    // delete sink;
}

void TestFloatEx()
{
    // flags: [-] / [0]
    // %[flags][width][.precision]specifier
    SetTestContext("Test Float Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // flag
    logger.InfoEx("'%f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-f'", TEST_FLOAT_VALUE);
    // width
    logger.InfoEx("'%5f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%20f'", TEST_FLOAT_VALUE);
    // flag & width
    logger.InfoEx("'%-5f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-20f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%05f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%020f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-05f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-020f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-5f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-20f'", TEST_FLOAT_VALUE);
    // prec
    logger.InfoEx("'%.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%.7f'", TEST_FLOAT_VALUE);
    // flag & prec
    logger.InfoEx("'%-.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-0.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-0.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-0.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-0.7f'", TEST_FLOAT_VALUE);
    // flag & width & prec
    logger.InfoEx("'%-5.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-5.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-5.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-5.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%05.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%05.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%05.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%05.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-5.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-5.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-5.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-5.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-05.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-05.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-05.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-05.7f'", TEST_FLOAT_VALUE);

    logger.InfoEx("'%-20.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-20.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-20.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-20.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%020.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%020.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%020.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%020.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-20.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-20.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-20.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%0-20.7f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-020.f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-020.0f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-020.2f'", TEST_FLOAT_VALUE);
    logger.InfoEx("'%-020.7f'", TEST_FLOAT_VALUE);

    ArrayList expected = GetFloatExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
    // delete sink;
}

ArrayList GetFloatExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750'");
    // width
    expected.PushString("'-12345.968750'");
    expected.PushString("'       -12345.968750'");
    // flag & width
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750       '");
    expected.PushString("'-12345.968750'");
    expected.PushString("'-000000012345.968750'");
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750       '");
    expected.PushString("'-12345.968750'");
    expected.PushString("'-12345.968750       '");
    // prec
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    // flag & prec
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    // flag & width & prec
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345'");
    expected.PushString("'-12345.96'");
    expected.PushString("'-12345.9687500'");

    expected.PushString("'-12345              '");
    expected.PushString("'-12345              '");
    expected.PushString("'-12345.96           '");
    expected.PushString("'-12345.9687500      '");
    expected.PushString("'-0000000000000012345'");
    expected.PushString("'-0000000000000012345'");
    expected.PushString("'-0000000000012345.96'");
    expected.PushString("'-00000012345.9687500'");
    expected.PushString("'-12345              '");
    expected.PushString("'-12345              '");
    expected.PushString("'-12345.96           '");
    expected.PushString("'-12345.9687500      '");
    expected.PushString("'-12345              '");
    expected.PushString("'-12345              '");
    expected.PushString("'-12345.96           '");
    expected.PushString("'-12345.9687500      '");
    return expected;
}


void TestBinary()
{
    TestBinaryAmxTpl();
    TestBinaryEx();
}

void TestBinaryAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Binary AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoAmxTpl("'%b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%-b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%0b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%-0b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%0-b'", StringToInt(TEST_BINARY_VALUE, 2));
    // width
    logger.InfoAmxTpl("'%5b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%35b'", StringToInt(TEST_BINARY_VALUE, 2));
    // flag & width
    logger.InfoAmxTpl("'%-5b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%-35b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%05b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%035b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%-05b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%-035b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%0-5b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoAmxTpl("'%0-35b'", StringToInt(TEST_BINARY_VALUE, 2));

    ArrayList expected = GetBinaryAmxTplExpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestBinaryEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Binary Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoEx("'%b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%-b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%0b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%-0b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%0-b'", StringToInt(TEST_BINARY_VALUE, 2));
    // width
    logger.InfoEx("'%5b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%35b'", StringToInt(TEST_BINARY_VALUE, 2));
    // flag & width
    logger.InfoEx("'%-5b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%-35b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%05b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%035b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%-05b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%-035b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%0-5b'", StringToInt(TEST_BINARY_VALUE, 2));
    logger.InfoEx("'%0-35b'", StringToInt(TEST_BINARY_VALUE, 2));

    ArrayList expected = GetBinaryExExpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetBinaryAmxTplExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_BINARY_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_BINARY_VALUE ... "[0]+");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#endif
    // width
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'    " ... TEST_BINARY_VALUE ... "'\\s");

    // flag & width
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_BINARY_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_BINARY_VALUE ... "    '\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'0000" ... TEST_BINARY_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_BINARY_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_BINARY_VALUE ... "0000'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_BINARY_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_BINARY_VALUE ... "0000'\\s");
    return expected;
}

ArrayList GetBinaryExExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");

    // width
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'    " ... TEST_BINARY_VALUE ... "'\\s");

    // flag & width
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "    '\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'0000" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "0000'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_BINARY_VALUE ... "0000'\\s");
    return expected;
}


void TestUInt()
{
    TestUIntAmxTpl();
    TestUIntEx();
}

void TestUIntAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test UInt AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoAmxTpl("'%u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%-u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%0u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%-0u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%0-u'", StringToInt(TEST_UINT_VALUE));
    // width
    logger.InfoAmxTpl("'%5u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%15u'", StringToInt(TEST_UINT_VALUE));
    // flag & width
    logger.InfoAmxTpl("'%-5u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%-15u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%05u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%015u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%-05u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%-015u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%0-5u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoAmxTpl("'%0-15u'", StringToInt(TEST_UINT_VALUE));

    ArrayList expected = GetUIntAmxTplExpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestUIntEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test UInt Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoEx("'%u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%-u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%0u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%-0u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%0-u'", StringToInt(TEST_UINT_VALUE));
    // width
    logger.InfoEx("'%5u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%15u'", StringToInt(TEST_UINT_VALUE));
    // flag & width
    logger.InfoEx("'%-5u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%-15u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%05u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%015u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%-05u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%-015u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%0-5u'", StringToInt(TEST_UINT_VALUE));
    logger.InfoEx("'%0-15u'", StringToInt(TEST_UINT_VALUE));

    ArrayList expected = GetUIntExEpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetUIntAmxTplExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_UINT_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
#endif

    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_UINT_VALUE ... "[0]+");
    expected.PushString("'" ... TEST_UINT_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
#endif

    // width
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'        " ... TEST_UINT_VALUE ... "'\\s");

    // flag & width
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_UINT_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_UINT_VALUE ... "        '\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'00000000" ... TEST_UINT_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_UINT_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_UINT_VALUE ... "00000000'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_UINT_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_UINT_VALUE ... "00000000'\\s");
    return expected;
}

ArrayList GetUIntExEpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");

    // width
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'        " ... TEST_UINT_VALUE ... "'\\s");

    // flag & width
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "        '\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'00000000" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "00000000'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_UINT_VALUE ... "00000000'\\s");
    return expected;
}


void TestInt()
{
    TestIntAmxTpl();
    TestIntEx();
}

void TestIntAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Int AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoAmxTpl("'%d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%-d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%0d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%-0d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%0-d'", StringToInt(TEST_INT_VALUE));
    // width
    logger.InfoAmxTpl("'%5d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%15d'", StringToInt(TEST_INT_VALUE));
    // flag & width
    logger.InfoAmxTpl("'%-5d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%-15d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%05d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%015d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%-05d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%-015d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%0-5d'", StringToInt(TEST_INT_VALUE));
    logger.InfoAmxTpl("'%0-15d'", StringToInt(TEST_INT_VALUE));

    ArrayList expected = GetIntAmxTplExpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestIntEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Int Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoEx("'%d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%-d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%0d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%-0d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%0-d'", StringToInt(TEST_INT_VALUE));
    // width
    logger.InfoEx("'%5d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%15d'", StringToInt(TEST_INT_VALUE));
    // flag & width
    logger.InfoEx("'%-5d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%-15d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%05d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%015d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%-05d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%-015d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%0-5d'", StringToInt(TEST_INT_VALUE));
    logger.InfoEx("'%0-15d'", StringToInt(TEST_INT_VALUE));

    ArrayList expected = GetIntExEpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetIntAmxTplExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_INT_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
#endif

    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_INT_VALUE ... "[0]+");
    expected.PushString("'" ... TEST_INT_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
#endif

    // width
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'       " ... TEST_INT_VALUE ... "'\\s");

    // flag & width
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_INT_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_INT_VALUE ... "       '\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'0000000" ... TEST_INT_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_INT_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_INT_VALUE ... "0000000'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_INT_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_INT_VALUE ... "0000000'\\s");
    return expected;
}

ArrayList GetIntExEpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");

    // width
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'       " ... TEST_INT_VALUE ... "'\\s");

    // flag & width
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "       '\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'-00000001234567'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "0000000'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_INT_VALUE ... "0000000'\\s");
    return expected;
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
    // %[flags][width]specifier
    SetTestContext("Test Hex Upper AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoAmxTpl("'%X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    // width
    logger.InfoAmxTpl("'%3X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    // flag & width
    logger.InfoAmxTpl("'%-3X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%03X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%-03X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%0-3X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoAmxTpl("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));

    ArrayList expected = GetHexUpperAmxTplExpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestHexUpperEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Hex Upper Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoEx("'%X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%-X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%0X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%-0X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%0-X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    // width
    logger.InfoEx("'%5X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%15X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    // flag & width
    logger.InfoEx("'%-5X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%-15X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%05X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%015X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%-05X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%-015X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%0-5X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));
    logger.InfoEx("'%0-15X'", StringToInt(TEST_HEX_UPPER_VALUE, 16));

    ArrayList expected = GetHexUpperExEpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetHexUpperAmxTplExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
#endif

    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "[0]+");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
#endif

    // width
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'          " ... TEST_HEX_UPPER_VALUE ... "'\\s");

    // flag & width
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "          '\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'0000000000" ... TEST_HEX_UPPER_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "0000000000'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "0000000000'\\s");
    return expected;
}

ArrayList GetHexUpperExEpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");

    // width
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'          " ... TEST_HEX_UPPER_VALUE ... "'\\s");

    // flag & width
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "          '\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'0000000000" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "0000000000'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_UPPER_VALUE ... "0000000000'\\s");
    return expected;
}


void TestHexLowerAmxTpl()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Hex Lower AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoAmxTpl("'%x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    // width
    logger.InfoAmxTpl("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    // flag & width
    logger.InfoAmxTpl("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoAmxTpl("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));

    ArrayList expected = GetHexLowerAmxTplExpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestHexLowerEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Hex Lower Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    // flag
    logger.InfoEx("'%x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%-x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%0x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%-0x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%0-x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    // width
    logger.InfoEx("'%5x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%15x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    // flag & width
    logger.InfoEx("'%-5x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%-15x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%05x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%015x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%-05x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%-015x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%0-5x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));
    logger.InfoEx("'%0-15x'", StringToInt(TEST_HEX_LOWER_VALUE, 16));

    ArrayList expected = GetHexLowerExEpected();
    TestSink.AssertLinesRegex("Log lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetHexLowerAmxTplExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
#endif

    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");

#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "[0]+");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
#endif

    // width
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'       " ... TEST_HEX_LOWER_VALUE ... "'\\s");

    // flag & width
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "[ ]+");
#else
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "       '\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'0000000" ... TEST_HEX_LOWER_VALUE ... "'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "0000000'\\s");
#if SOURCEMOD_V_MINOR < 13
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "[0]+");
#else
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
#endif
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "0000000'\\s");
    return expected;
}

ArrayList GetHexLowerExEpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    // flag
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");

    // width
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'       " ... TEST_HEX_LOWER_VALUE ... "'\\s");

    // flag & width
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "       '\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'0000000" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "0000000'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "'\\s");
    expected.PushString("'" ... TEST_HEX_LOWER_VALUE ... "0000000'\\s");
    return expected;
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

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY, GetLanguageByCode("en"));
    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY1, GetLanguageByCode("en"), TEST_TRANSLATES_DATA1);
    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY2, GetLanguageByCode("en"), StringToInt(TEST_TRANSLATES_DATA2));
    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY3, GetLanguageByCode("en"), TEST_TRANSLATES_DATA3_1, TEST_TRANSLATES_DATA3_2);

    ArrayList expected = GetTranslatesExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestTranslatesEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Translates Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY, GetLanguageByCode("en"));
    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY1, GetLanguageByCode("en"), TEST_TRANSLATES_DATA1);
    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY2, GetLanguageByCode("en"), StringToInt(TEST_TRANSLATES_DATA2));
    logger.InfoAmxTpl("'%T'", TEST_TRANSLATES_KEY3, GetLanguageByCode("en"), TEST_TRANSLATES_DATA3_1, TEST_TRANSLATES_DATA3_2);

    ArrayList expected = GetTranslatesExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetTranslatesExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    expected.PushString("'" ... TEST_TRANSLATES_VALUE ... "'");
    expected.PushString("'" ... TEST_TRANSLATES_VALUE1 ... "'");
    expected.PushString("'" ... TEST_TRANSLATES_VALUE2 ... "'");
    expected.PushString("'" ... TEST_TRANSLATES_VALUE3 ... "'");
    return expected;
}


void TestSpecial()
{
    TestSpecialAmxTpl();
    TestSpecialEx();
}

void TestSpecialAmxTpl()
{
    // %T, LANGID
    SetTestContext("Test Special AmxTpl");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("'%N'", 0);
    logger.InfoAmxTpl("'%L'", 0);

    ArrayList expected = GetSpecialExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

void TestSpecialEx()
{
    // flags: [-] / [0]
    // %[flags][width]specifier
    SetTestContext("Test Special Ex");

    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoEx("'%N'", 0);
    logger.InfoEx("'%L'", 0);

    ArrayList expected = GetSpecialExpected();
    TestSink.AssertMsgs("Log msgs", sink.GetMsgs(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}

ArrayList GetSpecialExpected()
{
    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    expected.PushString("'Console'");
    expected.PushString("'Console<0><Console><Console>'");
    return expected;
}
