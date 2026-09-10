#pragma semicolon 1
#pragma newdecls required

#if !defined DEBUG
    #define  DEBUG
#endif

#if !defined _DEBUG
    #define  _DEBUG
#endif

#if defined NDEBUG
    #undef  NDEBUG
#endif

#include <sourcemod>

#include <log4sp/details/long>

#include "../assert"


Long g_NegL,
     g_NegM,
     g_NegS,
     g_Zero,
     g_PosS,
     g_PosM,
     g_PosL;


public void OnPluginStart()
{
    static const int NEGL[2] = {0,                                  0b10000000000000000000000000000000},
                     NEGM[2] = {0b10001110000010001001010001001111, 0b11111111111111111111111011100000},
                     NEGS[2] = {0b11111111111111111111101100101110, 0b11111111111111111111111111111111},
                     ZERO[2] = {0,                                  0},
                     POSS[2] = {0b010011010010,                     0},
                     POSM[2] = {0b01110001111101110110101110110001, 0b000100011111},
                     POSL[2] = {0b11111111111111111111111111111111, 0b01111111111111111111111111111111};

    g_NegL = Int64ToLong(NEGL);
    g_NegM = Int64ToLong(NEGM);
    g_NegS = Int64ToLong(NEGS);
    g_Zero = Int64ToLong(ZERO);
    g_PosS = Int64ToLong(POSS);
    g_PosM = Int64ToLong(POSM);
    g_PosL = Int64ToLong(POSL);

    Test();
    RegServerCmd("sm_log4sp_test_long", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("-------------- Started testing Long --------------");

    TestStringToLong();

    TestInt64ToLong();

    TestAdd();

    TestSub();

    TestMul();

    TestDiv();

    TestMod();

    TestShl();

    TestShr();

    TestSar();

    TestAnd();

    TestOr();

    TestXor();

    TestNot();

    TestAbs();

    TestNeg();

    TestTestBit();

    TestSetBit();

    TestClearBit();

    TestFlipBit();

    TestCompare();

    // Sign
    // IsNegative
    // IsZero
    // ToInt64
    // ToI64
    // ToString
    // Low
    // High

    // 确保以上操作不会修改传参（总是返回新值）
    TestInt64ToLong();

    PrintToServer("----------------- Test Long ended ----------------");
}

void TestStringToLong()
{
    SetTestContext("StringToLong");

    AssertEq("100000000000 Low ",   StringToLong("100000000000").Low(),    0b01001000011101101110100000000000);
    AssertEq("100000000000 High",   StringToLong("100000000000").High(),   0b00010111);

    AssertEq("200000000000 Low ",   StringToLong("200000000000").Low(),    0b10010000111011011101000000000000);
    AssertEq("200000000000 High",   StringToLong("200000000000").High(),   0b00101110);

    AssertEq("274223593472 Low ",   StringToLong("274223593472").Low(),    0b11011000111111111111100000000000);
    AssertEq("274223593472 High",   StringToLong("274223593472").High(),   0b00111111);

    AssertEq("1234567654321 Low ",  StringToLong("1234567654321").Low(),   0b01110001111101110110101110110001);
    AssertEq("1234567654321 High",  StringToLong("1234567654321").High(),  0b000100011111);

    AssertEq("-1234567654321 Low ", StringToLong("-1234567654321").Low(),  0b10001110000010001001010001001111);
    AssertEq("-1234567654321 High", StringToLong("-1234567654321").High(), 0b11111111111111111111111011100000);
}

void TestInt64ToLong()
{
    SetTestContext("Int64ToLong");

    AssertStrEq("NegL", g_NegL.ToStringFixed(), "-9223372036854775808");
    AssertStrEq("NegM", g_NegM.ToStringFixed(), "-1234567654321");
    AssertStrEq("NegS", g_NegS.ToStringFixed(), "-1234");
    AssertStrEq("Zero", g_Zero.ToStringFixed(), "0");
    AssertStrEq("PosS", g_PosS.ToStringFixed(), "1234");
    AssertStrEq("PosM", g_PosM.ToStringFixed(), "1234567654321");
    AssertStrEq("PosL", g_PosL.ToStringFixed(), "9223372036854775807");
}

void TestAdd()
{
    SetTestContext("Long.Add");

    AssertStrEq("(NegL + NegL)", g_NegL.Add(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegL + NegM)", g_NegL.Add(g_NegM).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(NegL + NegS)", g_NegL.Add(g_NegS).ToStringFixed(), "9223372036854774574");
    AssertStrEq("(NegL + Zero)", g_NegL.Add(g_Zero).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL + PosS)", g_NegL.Add(g_PosS).ToStringFixed(), "-9223372036854774574");
    AssertStrEq("(NegL + PosM)", g_NegL.Add(g_PosM).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(NegL + PosL)", g_NegL.Add(g_PosL).ToStringFixed(), "-1");

    AssertStrEq("(NegM + NegL)", g_NegM.Add(g_NegL).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(NegM + NegM)", g_NegM.Add(g_NegM).ToStringFixed(), "-2469135308642");
    AssertStrEq("(NegM + NegS)", g_NegM.Add(g_NegS).ToStringFixed(), "-1234567655555");
    AssertStrEq("(NegM + Zero)", g_NegM.Add(g_Zero).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM + PosS)", g_NegM.Add(g_PosS).ToStringFixed(), "-1234567653087");
    AssertStrEq("(NegM + PosM)", g_NegM.Add(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(NegM + PosL)", g_NegM.Add(g_PosL).ToStringFixed(), "9223370802287121486");

    AssertStrEq("(NegS + NegL)", g_NegS.Add(g_NegL).ToStringFixed(), "9223372036854774574");
    AssertStrEq("(NegS + NegM)", g_NegS.Add(g_NegM).ToStringFixed(), "-1234567655555");
    AssertStrEq("(NegS + NegS)", g_NegS.Add(g_NegS).ToStringFixed(), "-2468");
    AssertStrEq("(NegS + Zero)", g_NegS.Add(g_Zero).ToStringFixed(), "-1234");
    AssertStrEq("(NegS + PosS)", g_NegS.Add(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(NegS + PosM)", g_NegS.Add(g_PosM).ToStringFixed(), "1234567653087");
    AssertStrEq("(NegS + PosL)", g_NegS.Add(g_PosL).ToStringFixed(), "9223372036854774573");

    AssertStrEq("(Zero + NegL)", g_Zero.Add(g_NegL).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(Zero + NegM)", g_Zero.Add(g_NegM).ToStringFixed(), "-1234567654321");
    AssertStrEq("(Zero + NegS)", g_Zero.Add(g_NegS).ToStringFixed(), "-1234");
    AssertStrEq("(Zero + Zero)", g_Zero.Add(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(Zero + PosS)", g_Zero.Add(g_PosS).ToStringFixed(), "1234");
    AssertStrEq("(Zero + PosM)", g_Zero.Add(g_PosM).ToStringFixed(), "1234567654321");
    AssertStrEq("(Zero + PosL)", g_Zero.Add(g_PosL).ToStringFixed(), "9223372036854775807");

    AssertStrEq("(PosS + NegL)", g_PosS.Add(g_NegL).ToStringFixed(), "-9223372036854774574");
    AssertStrEq("(PosS + NegM)", g_PosS.Add(g_NegM).ToStringFixed(), "-1234567653087");
    AssertStrEq("(PosS + NegS)", g_PosS.Add(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(PosS + Zero)", g_PosS.Add(g_Zero).ToStringFixed(), "1234");
    AssertStrEq("(PosS + PosS)", g_PosS.Add(g_PosS).ToStringFixed(), "2468");
    AssertStrEq("(PosS + PosM)", g_PosS.Add(g_PosM).ToStringFixed(), "1234567655555");
    AssertStrEq("(PosS + PosL)", g_PosS.Add(g_PosL).ToStringFixed(), "-9223372036854774575");

    AssertStrEq("(PosM + NegL)", g_PosM.Add(g_NegL).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(PosM + NegM)", g_PosM.Add(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(PosM + NegS)", g_PosM.Add(g_NegS).ToStringFixed(), "1234567653087");
    AssertStrEq("(PosM + Zero)", g_PosM.Add(g_Zero).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM + PosS)", g_PosM.Add(g_PosS).ToStringFixed(), "1234567655555");
    AssertStrEq("(PosM + PosM)", g_PosM.Add(g_PosM).ToStringFixed(), "2469135308642");
    AssertStrEq("(PosM + PosL)", g_PosM.Add(g_PosL).ToStringFixed(), "-9223370802287121488");

    AssertStrEq("(PosL + NegL)", g_PosL.Add(g_NegL).ToStringFixed(), "-1");
    AssertStrEq("(PosL + NegM)", g_PosL.Add(g_NegM).ToStringFixed(), "9223370802287121486");
    AssertStrEq("(PosL + NegS)", g_PosL.Add(g_NegS).ToStringFixed(), "9223372036854774573");
    AssertStrEq("(PosL + Zero)", g_PosL.Add(g_Zero).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL + PosS)", g_PosL.Add(g_PosS).ToStringFixed(), "-9223372036854774575");
    AssertStrEq("(PosL + PosM)", g_PosL.Add(g_PosM).ToStringFixed(), "-9223370802287121488");
    AssertStrEq("(PosL + PosL)", g_PosL.Add(g_PosL).ToStringFixed(), "-2");

    // Other
    AssertStrEq(
        "(1785980055 + 86400)",
        IntToLong(1785980055).Add(IntToLong(86400)).ToStringFixed(),
        "1786066455"
    );

    AssertStrEq(
        "(1234567654321 + (-1234567))",
        StringToLong("1234567654321").Add(IntToLong(-1234567)).ToStringFixed(),
        "1234566419754");

    AssertStrEq(
        "(1234567654321 + 0)",
        StringToLong("1234567654321").Add(IntToLong(0)).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 + 12)",
        StringToLong("1234567654321").Add(IntToLong(12)).ToStringFixed(),
        "1234567654333");

    AssertStrEq(
        "(1234567654321 + 45)",
        StringToLong("1234567654321").Add(IntToLong(45)).ToStringFixed(),
        "1234567654366");

    AssertStrEq(
        "(1234567654321 + 1234567)",
        StringToLong("1234567654321").Add(IntToLong(1234567)).ToStringFixed(),
        "1234568888888");

    AssertStrEq(
        "(1234567654321 + 200000000000)",
        StringToLong("1234567654321").Add(StringToLong("200000000000")).ToStringFixed(),
        "1434567654321");

    AssertStrEq(
        "(1234567654321 + 1234567654321)",
        StringToLong("1234567654321").Add(StringToLong("1234567654321")).ToStringFixed(),
        "2469135308642");

    AssertStrEq(
        "(1234567654321 + 12345676543210)",
        StringToLong("1234567654321").Add(StringToLong("12345676543210")).ToStringFixed(),
        "13580244197531");

    AssertStrEq(
        "(100000000000 + 300000000000)",
        StringToLong("100000000000").Add(StringToLong("300000000000")).ToStringFixed(),
        "400000000000");

}

void TestSub()
{
    SetTestContext("Long.Sub");

    AssertStrEq("(NegL - NegL)", g_NegL.Sub(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegL - NegM)", g_NegL.Sub(g_NegM).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(NegL - NegS)", g_NegL.Sub(g_NegS).ToStringFixed(), "-9223372036854774574");
    AssertStrEq("(NegL - Zero)", g_NegL.Sub(g_Zero).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL - PosS)", g_NegL.Sub(g_PosS).ToStringFixed(), "9223372036854774574");
    AssertStrEq("(NegL - PosM)", g_NegL.Sub(g_PosM).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(NegL - PosL)", g_NegL.Sub(g_PosL).ToStringFixed(), "1");

    AssertStrEq("(NegM - NegL)", g_NegM.Sub(g_NegL).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(NegM - NegM)", g_NegM.Sub(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(NegM - NegS)", g_NegM.Sub(g_NegS).ToStringFixed(), "-1234567653087");
    AssertStrEq("(NegM - Zero)", g_NegM.Sub(g_Zero).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM - PosS)", g_NegM.Sub(g_PosS).ToStringFixed(), "-1234567655555");
    AssertStrEq("(NegM - PosM)", g_NegM.Sub(g_PosM).ToStringFixed(), "-2469135308642");
    AssertStrEq("(NegM - PosL)", g_NegM.Sub(g_PosL).ToStringFixed(), "9223370802287121488");

    AssertStrEq("(NegS - NegL)", g_NegS.Sub(g_NegL).ToStringFixed(), "9223372036854774574");
    AssertStrEq("(NegS - NegM)", g_NegS.Sub(g_NegM).ToStringFixed(), "1234567653087");
    AssertStrEq("(NegS - NegS)", g_NegS.Sub(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(NegS - Zero)", g_NegS.Sub(g_Zero).ToStringFixed(), "-1234");
    AssertStrEq("(NegS - PosS)", g_NegS.Sub(g_PosS).ToStringFixed(), "-2468");
    AssertStrEq("(NegS - PosM)", g_NegS.Sub(g_PosM).ToStringFixed(), "-1234567655555");
    AssertStrEq("(NegS - PosL)", g_NegS.Sub(g_PosL).ToStringFixed(), "9223372036854774575");

    AssertStrEq("(Zero - NegL)", g_Zero.Sub(g_NegL).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(Zero - NegM)", g_Zero.Sub(g_NegM).ToStringFixed(), "1234567654321");
    AssertStrEq("(Zero - NegS)", g_Zero.Sub(g_NegS).ToStringFixed(), "1234");
    AssertStrEq("(Zero - Zero)", g_Zero.Sub(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(Zero - PosS)", g_Zero.Sub(g_PosS).ToStringFixed(), "-1234");
    AssertStrEq("(Zero - PosM)", g_Zero.Sub(g_PosM).ToStringFixed(), "-1234567654321");
    AssertStrEq("(Zero - PosL)", g_Zero.Sub(g_PosL).ToStringFixed(), "-9223372036854775807");

    AssertStrEq("(PosS - NegL)", g_PosS.Sub(g_NegL).ToStringFixed(), "-9223372036854774574");
    AssertStrEq("(PosS - NegM)", g_PosS.Sub(g_NegM).ToStringFixed(), "1234567655555");
    AssertStrEq("(PosS - NegS)", g_PosS.Sub(g_NegS).ToStringFixed(), "2468");
    AssertStrEq("(PosS - Zero)", g_PosS.Sub(g_Zero).ToStringFixed(), "1234");
    AssertStrEq("(PosS - PosS)", g_PosS.Sub(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(PosS - PosM)", g_PosS.Sub(g_PosM).ToStringFixed(), "-1234567653087");
    AssertStrEq("(PosS - PosL)", g_PosS.Sub(g_PosL).ToStringFixed(), "-9223372036854774573");

    AssertStrEq("(PosM - NegL)", g_PosM.Sub(g_NegL).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(PosM - NegM)", g_PosM.Sub(g_NegM).ToStringFixed(), "2469135308642");
    AssertStrEq("(PosM - NegS)", g_PosM.Sub(g_NegS).ToStringFixed(), "1234567655555");
    AssertStrEq("(PosM - Zero)", g_PosM.Sub(g_Zero).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM - PosS)", g_PosM.Sub(g_PosS).ToStringFixed(), "1234567653087");
    AssertStrEq("(PosM - PosM)", g_PosM.Sub(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(PosM - PosL)", g_PosM.Sub(g_PosL).ToStringFixed(), "-9223370802287121486");

    AssertStrEq("(PosL - NegL)", g_PosL.Sub(g_NegL).ToStringFixed(), "-1");
    AssertStrEq("(PosL - NegM)", g_PosL.Sub(g_NegM).ToStringFixed(), "-9223370802287121488");
    AssertStrEq("(PosL - NegS)", g_PosL.Sub(g_NegS).ToStringFixed(), "-9223372036854774575");
    AssertStrEq("(PosL - Zero)", g_PosL.Sub(g_Zero).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL - PosS)", g_PosL.Sub(g_PosS).ToStringFixed(), "9223372036854774573");
    AssertStrEq("(PosL - PosM)", g_PosL.Sub(g_PosM).ToStringFixed(), "9223370802287121486");
    AssertStrEq("(PosL - PosL)", g_PosL.Sub(g_PosL).ToStringFixed(), "0");

    // Other
    AssertStrEq(
        "(1234567654321 - (-1234567))",
        StringToLong("1234567654321").Sub(IntToLong(-1234567)).ToStringFixed(),
        "1234568888888");

    AssertStrEq(
        "(1234567654321 - 0)",
        StringToLong("1234567654321").Sub(IntToLong(0)).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 - 12)",
        StringToLong("1234567654321").Sub(IntToLong(12)).ToStringFixed(),
        "1234567654309");

    AssertStrEq(
        "(1234567654321 - 45)",
        StringToLong("1234567654321").Sub(IntToLong(45)).ToStringFixed(),
        "1234567654276");

    AssertStrEq(
        "(1234567654321 - 1234567)",
        StringToLong("1234567654321").Sub(IntToLong(1234567)).ToStringFixed(),
        "1234566419754");

    AssertStrEq(
        "(1234567654321 - 200000000000)",
        StringToLong("1234567654321").Sub(StringToLong("200000000000")).ToStringFixed(),
        "1034567654321");

    AssertStrEq(
        "(1234567654321 - 1234567654321)",
        StringToLong("1234567654321").Sub(StringToLong("1234567654321")).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 - 12345676543210)",
        StringToLong("1234567654321").Sub(StringToLong("12345676543210")).ToStringFixed(),
        "-11111108888889");

    AssertStrEq(
        "(100000000000 - 300000000000)",
        StringToLong("100000000000").Sub(StringToLong("300000000000")).ToStringFixed(),
        "-200000000000");

    AssertStrEq(
        "-3 - (-450000000000)",
        StringToLong("-3").Sub(StringToLong("-450000000000")).ToStringFixed(),
        "449999999997");

}

void TestMul()
{
    SetTestContext("Long.Mul");

    AssertStrEq("(NegL * NegL)", g_NegL.Mul(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegL * NegM)", g_NegL.Mul(g_NegM).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL * NegS)", g_NegL.Mul(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(NegL * Zero)", g_NegL.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(NegL * PosS)", g_NegL.Mul(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(NegL * PosM)", g_NegL.Mul(g_PosM).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL * PosL)", g_NegL.Mul(g_PosL).ToStringFixed(), "-9223372036854775808");

    AssertStrEq("(NegM * NegL)", g_NegM.Mul(g_NegL).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegM * NegM)", g_NegM.Mul(g_NegM).ToStringFixed(), "-4935994595552300959");
    AssertStrEq("(NegM * NegS)", g_NegM.Mul(g_NegS).ToStringFixed(), "1523456485432114");
    AssertStrEq("(NegM * Zero)", g_NegM.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(NegM * PosS)", g_NegM.Mul(g_PosS).ToStringFixed(), "-1523456485432114");
    AssertStrEq("(NegM * PosM)", g_NegM.Mul(g_PosM).ToStringFixed(), "4935994595552300959");
    AssertStrEq("(NegM * PosL)", g_NegM.Mul(g_PosL).ToStringFixed(), "-9223370802287121487");

    AssertStrEq("(NegS * NegL)", g_NegS.Mul(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegS * NegM)", g_NegS.Mul(g_NegM).ToStringFixed(), "1523456485432114");
    AssertStrEq("(NegS * NegS)", g_NegS.Mul(g_NegS).ToStringFixed(), "1522756");
    AssertStrEq("(NegS * Zero)", g_NegS.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(NegS * PosS)", g_NegS.Mul(g_PosS).ToStringFixed(), "-1522756");
    AssertStrEq("(NegS * PosM)", g_NegS.Mul(g_PosM).ToStringFixed(), "-1523456485432114");
    AssertStrEq("(NegS * PosL)", g_NegS.Mul(g_PosL).ToStringFixed(), "1234");

    AssertStrEq("(Zero * NegL)", g_Zero.Mul(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(Zero * NegM)", g_Zero.Mul(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(Zero * NegS)", g_Zero.Mul(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(Zero * Zero)", g_Zero.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(Zero * PosS)", g_Zero.Mul(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(Zero * PosM)", g_Zero.Mul(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(Zero * PosL)", g_Zero.Mul(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(PosS * NegL)", g_PosS.Mul(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(PosS * NegM)", g_PosS.Mul(g_NegM).ToStringFixed(), "-1523456485432114");
    AssertStrEq("(PosS * NegS)", g_PosS.Mul(g_NegS).ToStringFixed(), "-1522756");
    AssertStrEq("(PosS * Zero)", g_PosS.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(PosS * PosS)", g_PosS.Mul(g_PosS).ToStringFixed(), "1522756");
    AssertStrEq("(PosS * PosM)", g_PosS.Mul(g_PosM).ToStringFixed(), "1523456485432114");
    AssertStrEq("(PosS * PosL)", g_PosS.Mul(g_PosL).ToStringFixed(), "-1234");

    AssertStrEq("(PosM * NegL)", g_PosM.Mul(g_NegL).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(PosM * NegM)", g_PosM.Mul(g_NegM).ToStringFixed(), "4935994595552300959");
    AssertStrEq("(PosM * NegS)", g_PosM.Mul(g_NegS).ToStringFixed(), "-1523456485432114");
    AssertStrEq("(PosM * Zero)", g_PosM.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(PosM * PosS)", g_PosM.Mul(g_PosS).ToStringFixed(), "1523456485432114");
    AssertStrEq("(PosM * PosM)", g_PosM.Mul(g_PosM).ToStringFixed(), "-4935994595552300959");
    AssertStrEq("(PosM * PosL)", g_PosM.Mul(g_PosL).ToStringFixed(), "9223370802287121487");

    AssertStrEq("(PosL * NegL)", g_PosL.Mul(g_NegL).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(PosL * NegM)", g_PosL.Mul(g_NegM).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(PosL * NegS)", g_PosL.Mul(g_NegS).ToStringFixed(), "1234");
    AssertStrEq("(PosL * Zero)", g_PosL.Mul(g_Zero).ToStringFixed(), "0");
    AssertStrEq("(PosL * PosS)", g_PosL.Mul(g_PosS).ToStringFixed(), "-1234");
    AssertStrEq("(PosL * PosM)", g_PosL.Mul(g_PosM).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(PosL * PosL)", g_PosL.Mul(g_PosL).ToStringFixed(), "1");

    // Other
    AssertStrEq(
        "100000000000 * 300000000000",
        StringToLong("100000000000").Mul(StringToLong("300000000000")).ToStringFixed(),
        "5594136148269072384");
}

void TestDiv()
{
    SetTestContext("Long.Div");

    AssertStrEq("(NegL / NegL)", g_NegL.Div(g_NegL).ToStringFixed(), "1");
    AssertStrEq("(NegL / NegM)", g_NegL.Div(g_NegM).ToStringFixed(), "7470932");
    AssertStrEq("(NegL / NegS)", g_NegL.Div(g_NegS).ToStringFixed(), "7474369559849899");
    AssertStrEq("(NegL / PosS)", g_NegL.Div(g_PosS).ToStringFixed(), "-7474369559849899");
    AssertStrEq("(NegL / PosM)", g_NegL.Div(g_PosM).ToStringFixed(), "-7470932");
    AssertStrEq("(NegL / PosL)", g_NegL.Div(g_PosL).ToStringFixed(), "-1");

    AssertStrEq("(NegM / NegL)", g_NegM.Div(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegM / NegM)", g_NegM.Div(g_NegM).ToStringFixed(), "1");
    AssertStrEq("(NegM / NegS)", g_NegM.Div(g_NegS).ToStringFixed(), "1000460011");
    AssertStrEq("(NegM / PosS)", g_NegM.Div(g_PosS).ToStringFixed(), "-1000460011");
    AssertStrEq("(NegM / PosM)", g_NegM.Div(g_PosM).ToStringFixed(), "-1");
    AssertStrEq("(NegM / PosL)", g_NegM.Div(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(NegS / NegL)", g_NegS.Div(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegS / NegM)", g_NegS.Div(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(NegS / NegS)", g_NegS.Div(g_NegS).ToStringFixed(), "1");
    AssertStrEq("(NegS / PosS)", g_NegS.Div(g_PosS).ToStringFixed(), "-1");
    AssertStrEq("(NegS / PosM)", g_NegS.Div(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(NegS / PosL)", g_NegS.Div(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(Zero / NegL)", g_Zero.Div(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(Zero / NegM)", g_Zero.Div(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(Zero / NegS)", g_Zero.Div(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(Zero / PosS)", g_Zero.Div(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(Zero / PosM)", g_Zero.Div(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(Zero / PosL)", g_Zero.Div(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(PosS / NegL)", g_PosS.Div(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(PosS / NegM)", g_PosS.Div(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(PosS / NegS)", g_PosS.Div(g_NegS).ToStringFixed(), "-1");
    AssertStrEq("(PosS / PosS)", g_PosS.Div(g_PosS).ToStringFixed(), "1");
    AssertStrEq("(PosS / PosM)", g_PosS.Div(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(PosS / PosL)", g_PosS.Div(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(PosM / NegL)", g_PosM.Div(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(PosM / NegM)", g_PosM.Div(g_NegM).ToStringFixed(), "-1");
    AssertStrEq("(PosM / NegS)", g_PosM.Div(g_NegS).ToStringFixed(), "-1000460011");
    AssertStrEq("(PosM / PosS)", g_PosM.Div(g_PosS).ToStringFixed(), "1000460011");
    AssertStrEq("(PosM / PosM)", g_PosM.Div(g_PosM).ToStringFixed(), "1");
    AssertStrEq("(PosM / PosL)", g_PosM.Div(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(PosL / NegL)", g_PosL.Div(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(PosL / NegM)", g_PosL.Div(g_NegM).ToStringFixed(), "-7470932");
    AssertStrEq("(PosL / NegS)", g_PosL.Div(g_NegS).ToStringFixed(), "-7474369559849899");
    AssertStrEq("(PosL / PosS)", g_PosL.Div(g_PosS).ToStringFixed(), "7474369559849899");
    AssertStrEq("(PosL / PosM)", g_PosL.Div(g_PosM).ToStringFixed(), "7470932");
    AssertStrEq("(PosL / PosL)", g_PosL.Div(g_PosL).ToStringFixed(), "1");

    // Other
    AssertStrEq(
        "100 / 7",
        IntToLong(100).Div(IntToLong(7)).ToStringFixed(),
        "14");

    AssertStrEq(
        "300000000000 / 200000000000",
        StringToLong("300000000000").Div(StringToLong("200000000000")).ToStringFixed(),
        "1");

    AssertStrEq(
        "300000000000 / 100000000000",
        StringToLong("300000000000").Div(StringToLong("100000000000")).ToStringFixed(),
        "3");
}

void TestMod()
{
    SetTestContext("Long.Mod");

    AssertStrEq("(NegL % NegL)", g_NegL.Mod(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(NegL % NegM)", g_NegL.Mod(g_NegM).ToStringFixed(), "-1042023078636");
    AssertStrEq("(NegL % NegS)", g_NegL.Mod(g_NegS).ToStringFixed(), "-442");
    AssertStrEq("(NegL % PosS)", g_NegL.Mod(g_PosS).ToStringFixed(), "-442");
    AssertStrEq("(NegL % PosM)", g_NegL.Mod(g_PosM).ToStringFixed(), "-1042023078636");
    AssertStrEq("(NegL % PosL)", g_NegL.Mod(g_PosL).ToStringFixed(), "-1");

    AssertStrEq("(NegM % NegL)", g_NegM.Mod(g_NegL).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM % NegM)", g_NegM.Mod(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(NegM % NegS)", g_NegM.Mod(g_NegS).ToStringFixed(), "-747");
    AssertStrEq("(NegM % PosS)", g_NegM.Mod(g_PosS).ToStringFixed(), "-747");
    AssertStrEq("(NegM % PosM)", g_NegM.Mod(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(NegM % PosL)", g_NegM.Mod(g_PosL).ToStringFixed(), "-1234567654321");

    AssertStrEq("(NegS % NegL)", g_NegS.Mod(g_NegL).ToStringFixed(), "-1234");
    AssertStrEq("(NegS % NegM)", g_NegS.Mod(g_NegM).ToStringFixed(), "-1234");
    AssertStrEq("(NegS % NegS)", g_NegS.Mod(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(NegS % PosS)", g_NegS.Mod(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(NegS % PosM)", g_NegS.Mod(g_PosM).ToStringFixed(), "-1234");
    AssertStrEq("(NegS % PosL)", g_NegS.Mod(g_PosL).ToStringFixed(), "-1234");

    AssertStrEq("(Zero % NegL)", g_Zero.Mod(g_NegL).ToStringFixed(), "0");
    AssertStrEq("(Zero % NegM)", g_Zero.Mod(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(Zero % NegS)", g_Zero.Mod(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(Zero % PosS)", g_Zero.Mod(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(Zero % PosM)", g_Zero.Mod(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(Zero % PosL)", g_Zero.Mod(g_PosL).ToStringFixed(), "0");

    AssertStrEq("(PosS % NegL)", g_PosS.Mod(g_NegL).ToStringFixed(), "1234");
    AssertStrEq("(PosS % NegM)", g_PosS.Mod(g_NegM).ToStringFixed(), "1234");
    AssertStrEq("(PosS % NegS)", g_PosS.Mod(g_NegS).ToStringFixed(), "0");
    AssertStrEq("(PosS % PosS)", g_PosS.Mod(g_PosS).ToStringFixed(), "0");
    AssertStrEq("(PosS % PosM)", g_PosS.Mod(g_PosM).ToStringFixed(), "1234");
    AssertStrEq("(PosS % PosL)", g_PosS.Mod(g_PosL).ToStringFixed(), "1234");

    AssertStrEq("(PosM % NegL)", g_PosM.Mod(g_NegL).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM % NegM)", g_PosM.Mod(g_NegM).ToStringFixed(), "0");
    AssertStrEq("(PosM % NegS)", g_PosM.Mod(g_NegS).ToStringFixed(), "747");
    AssertStrEq("(PosM % PosS)", g_PosM.Mod(g_PosS).ToStringFixed(), "747");
    AssertStrEq("(PosM % PosM)", g_PosM.Mod(g_PosM).ToStringFixed(), "0");
    AssertStrEq("(PosM % PosL)", g_PosM.Mod(g_PosL).ToStringFixed(), "1234567654321");

    AssertStrEq("(PosL % NegL)", g_PosL.Mod(g_NegL).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL % NegM)", g_PosL.Mod(g_NegM).ToStringFixed(), "1042023078635");
    AssertStrEq("(PosL % NegS)", g_PosL.Mod(g_NegS).ToStringFixed(), "441");
    AssertStrEq("(PosL % PosS)", g_PosL.Mod(g_PosS).ToStringFixed(), "441");
    AssertStrEq("(PosL % PosM)", g_PosL.Mod(g_PosM).ToStringFixed(), "1042023078635");
    AssertStrEq("(PosL % PosL)", g_PosL.Mod(g_PosL).ToStringFixed(), "0");

    // Other
    AssertStrEq(
        "100 % 7",
        IntToLong(100).Mod(IntToLong(7)).ToStringFixed(),
        "2");

    AssertStrEq(
        "300000000000 % 200000000000",
        StringToLong("300000000000").Mod(StringToLong("200000000000")).ToStringFixed(),
        "100000000000");

    AssertStrEq(
        "300000000000 % 100000000000",
        StringToLong("300000000000").Mod(StringToLong("100000000000")).ToStringFixed(),
        "0");
}

void TestShl()
{
    SetTestContext("Long.Shl");

    AssertStrEq(
        "(1234567654321 << (-1234567))",
        StringToLong("1234567654321").Shl(-1234567).ToStringFixed(),
        "7061644215716937728");

    AssertStrEq(
        "(1234567654321 << 0)",
        StringToLong("1234567654321").Shl(0).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 << 12)",
        StringToLong("1234567654321").Shl(12).ToStringFixed(),
        "5056789112098816");

    AssertStrEq(
        "(1234567654321 << 45)",
        StringToLong("1234567654321").Shl(45).ToStringFixed(),
        "-1335845055096684544");

    AssertStrEq(
        "(1234567654321 << 1234567)",
        StringToLong("1234567654321").Shl(1234567).ToStringFixed(),
        "158024659753088");
}

void TestShr()
{
    SetTestContext("Long.Shr");

    AssertStrEq(
        "(1234567654321 >>> (-1234567))",
        StringToLong("1234567654321").Shr(-1234567).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 >>> 0)",
        StringToLong("1234567654321").Shr(0).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 >>> 12)",
        StringToLong("1234567654321").Shr(12).ToStringFixed(),
        "301408118");

    AssertStrEq(
        "(1234567654321 >>> 45)",
        StringToLong("1234567654321").Shr(45).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 >>> 1234567)",
        StringToLong("1234567654321").Shr(1234567).ToStringFixed(),
        "9645059799");

    AssertStrEq(
        "(-1 >>> 10)",
        StringToLong("-1").Shr(10).ToStringFixed(),
        "18014398509481983");

    AssertStrEq(
        "(-450000000000 >>> 2)",
        StringToLong("-450000000000").Shr(2).ToStringFixed(),
        "4611685905927387904");
}

void TestSar()
{
    SetTestContext("Long.Sar");

    AssertStrEq(
        "(1234567654321 >> (-1234567))",
        StringToLong("1234567654321").Sar(-1234567).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 >> 0)",
        StringToLong("1234567654321").Sar(0).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 >> 12)",
        StringToLong("1234567654321").Sar(12).ToStringFixed(),
        "301408118");

    AssertStrEq(
        "(1234567654321 >> 45)",
        StringToLong("1234567654321").Sar(45).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 >> 1234567)",
        StringToLong("1234567654321").Sar(1234567).ToStringFixed(),
        "9645059799");

    AssertStrEq(
        "(-450000000000 >> 2)",
        StringToLong("-450000000000").Sar(2).ToStringFixed(),
        "-112500000000");

    AssertStrEq(
        "(0x5e5e5e5e5e5e5e5e >> 10)",
        StringToLong("0x5e5e5e5e5e5e5e5e", 16).Sar(10).ToStringFixed(),
        "6640601803495319");
}

void TestAnd()
{
    SetTestContext("Long.And");

    AssertStrEq(
        "(1234567654321 & (-1234567))",
        StringToLong("1234567654321").And(IntToLong(-1234567)).ToStringFixed(),
        "1234566457649");

    AssertStrEq(
        "(1234567654321 & 0)",
        StringToLong("1234567654321").And(IntToLong(0)).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 & 12)",
        StringToLong("1234567654321").And(IntToLong(12)).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 & 45)",
        StringToLong("1234567654321").And(IntToLong(45)).ToStringFixed(),
        "33");

    AssertStrEq(
        "(1234567654321 & 1234567)",
        StringToLong("1234567654321").And(IntToLong(1234567)).ToStringFixed(),
        "1196673");

    AssertStrEq(
        "(1234567654321 & 200000000000)",
        StringToLong("1234567654321").And(StringToLong("200000000000")).ToStringFixed(),
        "60413001728");

    AssertStrEq(
        "(1234567654321 & 1234567654321)",
        StringToLong("1234567654321").And(StringToLong("1234567654321")).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 & 12345676543210)",
        StringToLong("1234567654321").And(StringToLong("12345676543210")).ToStringFixed(),
        "1213087228064");

    AssertStrEq(
        "(100000000000 & 300000000000)",
        StringToLong("100000000000").And(StringToLong("300000000000")).ToStringFixed(),
        "22689392640");
}

void TestOr()
{
    SetTestContext("Long.Or");

    AssertStrEq(
        "(1234567654321 | (-1234567))",
        StringToLong("1234567654321").Or(IntToLong(-1234567)).ToStringFixed(),
        "-37895");

    AssertStrEq(
        "(1234567654321 | 0)",
        StringToLong("1234567654321").Or(IntToLong(0)).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 | 12)",
        StringToLong("1234567654321").Or(IntToLong(12)).ToStringFixed(),
        "1234567654333");

    AssertStrEq(
        "(1234567654321 | 45)",
        StringToLong("1234567654321").Or(IntToLong(45)).ToStringFixed(),
        "1234567654333");

    AssertStrEq(
        "(1234567654321 | 1234567)",
        StringToLong("1234567654321").Or(IntToLong(1234567)).ToStringFixed(),
        "1234567692215");

    AssertStrEq(
        "(1234567654321 | 200000000000)",
        StringToLong("1234567654321").Or(StringToLong("200000000000")).ToStringFixed(),
        "1374154652593");

    AssertStrEq(
        "(1234567654321 | 1234567654321)",
        StringToLong("1234567654321").Or(StringToLong("1234567654321")).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 | 12345676543210)",
        StringToLong("1234567654321").Or(StringToLong("12345676543210")).ToStringFixed(),
        "12367156969467");

    AssertStrEq(
        "(100000000000 | 200000000000)",
        StringToLong("100000000000").Or(StringToLong("200000000000")).ToStringFixed(),
        "274223593472");
}

void TestXor()
{
    SetTestContext("Long.Xor");

    AssertStrEq(
        "(1234567654321 ^ (-1234567))",
        StringToLong("1234567654321").Xor(IntToLong(-1234567)).ToStringFixed(),
        "-1234566495544");

    AssertStrEq(
        "(1234567654321 ^ 0)",
        StringToLong("1234567654321").Xor(IntToLong(0)).ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(1234567654321 ^ 12)",
        StringToLong("1234567654321").Xor(IntToLong(12)).ToStringFixed(),
        "1234567654333");

    AssertStrEq(
        "(1234567654321 ^ 45)",
        StringToLong("1234567654321").Xor(IntToLong(45)).ToStringFixed(),
        "1234567654300");

    AssertStrEq(
        "(1234567654321 ^ 1234567)",
        StringToLong("1234567654321").Xor(IntToLong(1234567)).ToStringFixed(),
        "1234566495542");

    AssertStrEq(
        "(1234567654321 ^ 200000000000)",
        StringToLong("1234567654321").Xor(StringToLong("200000000000")).ToStringFixed(),
        "1313741650865");

    AssertStrEq(
        "(1234567654321 ^ 1234567654321)",
        StringToLong("1234567654321").Xor(StringToLong("1234567654321")).ToStringFixed(),
        "0");

    AssertStrEq(
        "(1234567654321 ^ 12345676543210)",
        StringToLong("1234567654321").Xor(StringToLong("12345676543210")).ToStringFixed(),
        "11154069741403");

    AssertStrEq(
        "(100000000000 ^ 300000000000)",
        StringToLong("100000000000").Xor(StringToLong("300000000000")).ToStringFixed(),
        "354621214720");
}

void TestNot()
{
    SetTestContext("Long.Not");

    AssertStrEq(
        "(~(-1234567))",
        IntToLong(-1234567).Not().ToStringFixed(),
        "1234566");

    AssertStrEq(
        "(~0)",
        IntToLong(0).Not().ToStringFixed(),
        "-1");

    AssertStrEq(
        "(~12)",
        IntToLong(12).Not().ToStringFixed(),
        "-13");

    AssertStrEq(
        "(~45)",
        IntToLong(45).Not().ToStringFixed(),
        "-46");

    AssertStrEq(
        "(~1234567)",
        IntToLong(1234567).Not().ToStringFixed(),
        "-1234568");

    AssertStrEq(
        "(~200000000000)",
        StringToLong("200000000000").Not().ToStringFixed(),
        "-200000000001");

    AssertStrEq(
        "(~1234567654321)",
        StringToLong("1234567654321").Not().ToStringFixed(),
        "-1234567654322");
}

void TestAbs()
{
    SetTestContext("Long.Abs");

    AssertStrEq(
        "|(-1234567654321)|",
        StringToLong("-1234567654321").Abs().ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "|(-1234567)|",
        IntToLong(-1234567).Abs().ToStringFixed(),
        "1234567");

    AssertStrEq(
        "|0|",
        IntToLong(0).Abs().ToStringFixed(),
        "0");

    AssertStrEq(
        "|12|",
        IntToLong(12).Abs().ToStringFixed(),
        "12");

    AssertStrEq(
        "|45|",
        IntToLong(45).Abs().ToStringFixed(),
        "45");

    AssertStrEq(
        "|1234567|",
        IntToLong(1234567).Abs().ToStringFixed(),
        "1234567");

    AssertStrEq(
        "|200000000000|",
        StringToLong("200000000000").Abs().ToStringFixed(),
        "200000000000");

    AssertStrEq(
        "|1234567654321|",
        StringToLong("1234567654321").Abs().ToStringFixed(),
        "1234567654321");
}

void TestNeg()
{
    SetTestContext("Long.Neg");

    AssertStrEq(
        "(-(-1234567654321))",
        StringToLong("-1234567654321").Neg().ToStringFixed(),
        "1234567654321");

    AssertStrEq(
        "(-(-1234567))",
        IntToLong(-1234567).Neg().ToStringFixed(),
        "1234567");

    AssertStrEq(
        "(-(-1))",
        IntToLong(-1).Neg().ToStringFixed(),
        "1");

    AssertStrEq(
        "(-0)",
        IntToLong(0).Neg().ToStringFixed(),
        "0");

    AssertStrEq(
        "(-12)",
        IntToLong(12).Neg().ToStringFixed(),
        "-12");

    AssertStrEq(
        "(-45)",
        IntToLong(45).Neg().ToStringFixed(),
        "-45");

    AssertStrEq(
        "(-1234567)",
        IntToLong(1234567).Neg().ToStringFixed(),
        "-1234567");

    AssertStrEq(
        "(-200000000000)",
        StringToLong("200000000000").Neg().ToStringFixed(),
        "-200000000000");

    AssertStrEq(
        "(-1234567654321)",
        StringToLong("1234567654321").Neg().ToStringFixed(),
        "-1234567654321");
}

void TestTestBit()
{
    SetTestContext("Long.TestBit");

    AssertFalse("(NegL & (1 <<  0))", g_NegL.TestBit( 0));
    AssertFalse("(NegL & (1 <<  1))", g_NegL.TestBit( 1));
    AssertFalse("(NegL & (1 <<  7))", g_NegL.TestBit( 7));
    AssertFalse("(NegL & (1 << 14))", g_NegL.TestBit(14));
    AssertFalse("(NegL & (1 << 35))", g_NegL.TestBit(35));
    AssertFalse("(NegL & (1 << 56))", g_NegL.TestBit(56));
    AssertTrue( "(NegL & (1 << 63))", g_NegL.TestBit(63));
    AssertFalse("(NegL & (1 << 64))", g_NegL.TestBit(64));
    AssertFalse("(NegL & (1 << 99))", g_NegL.TestBit(99));

    AssertTrue( "(NegM & (1 <<  0))", g_NegM.TestBit( 0));
    AssertTrue( "(NegM & (1 <<  1))", g_NegM.TestBit( 1));
    AssertFalse("(NegM & (1 <<  7))", g_NegM.TestBit( 7));
    AssertFalse("(NegM & (1 << 14))", g_NegM.TestBit(14));
    AssertFalse("(NegM & (1 << 35))", g_NegM.TestBit(35));
    AssertTrue( "(NegM & (1 << 56))", g_NegM.TestBit(56));
    AssertTrue( "(NegM & (1 << 63))", g_NegM.TestBit(63));
    AssertTrue( "(NegM & (1 << 64))", g_NegM.TestBit(64));
    AssertFalse("(NegM & (1 << 99))", g_NegM.TestBit(99));

    AssertFalse("(NegS & (1 <<  0))", g_NegS.TestBit( 0));
    AssertTrue( "(NegS & (1 <<  1))", g_NegS.TestBit( 1));
    AssertFalse("(NegS & (1 <<  7))", g_NegS.TestBit( 7));
    AssertTrue( "(NegS & (1 << 14))", g_NegS.TestBit(14));
    AssertTrue( "(NegS & (1 << 35))", g_NegS.TestBit(35));
    AssertTrue( "(NegS & (1 << 56))", g_NegS.TestBit(56));
    AssertTrue( "(NegS & (1 << 63))", g_NegS.TestBit(63));
    AssertFalse("(NegS & (1 << 64))", g_NegS.TestBit(64));
    AssertTrue( "(NegS & (1 << 99))", g_NegS.TestBit(99));

    AssertFalse("(Zero & (1 <<  0))", g_Zero.TestBit( 0));
    AssertFalse("(Zero & (1 <<  1))", g_Zero.TestBit( 1));
    AssertFalse("(Zero & (1 <<  7))", g_Zero.TestBit( 7));
    AssertFalse("(Zero & (1 << 14))", g_Zero.TestBit(14));
    AssertFalse("(Zero & (1 << 35))", g_Zero.TestBit(35));
    AssertFalse("(Zero & (1 << 56))", g_Zero.TestBit(56));
    AssertFalse("(Zero & (1 << 63))", g_Zero.TestBit(63));
    AssertFalse("(Zero & (1 << 64))", g_Zero.TestBit(64));
    AssertFalse("(Zero & (1 << 99))", g_Zero.TestBit(99));

    AssertFalse("(PosS & (1 <<  0))", g_PosS.TestBit( 0));
    AssertTrue( "(PosS & (1 <<  1))", g_PosS.TestBit( 1));
    AssertTrue( "(PosS & (1 <<  7))", g_PosS.TestBit( 7));
    AssertFalse("(PosS & (1 << 14))", g_PosS.TestBit(14));
    AssertFalse("(PosS & (1 << 35))", g_PosS.TestBit(35));
    AssertFalse("(PosS & (1 << 56))", g_PosS.TestBit(56));
    AssertFalse("(PosS & (1 << 63))", g_PosS.TestBit(63));
    AssertFalse("(PosS & (1 << 64))", g_PosS.TestBit(64));
    AssertFalse("(PosS & (1 << 99))", g_PosS.TestBit(99));

    AssertTrue( "(PosM & (1 <<  0))", g_PosM.TestBit( 0));
    AssertFalse("(PosM & (1 <<  1))", g_PosM.TestBit( 1));
    AssertTrue( "(PosM & (1 <<  7))", g_PosM.TestBit( 7));
    AssertTrue( "(PosM & (1 << 14))", g_PosM.TestBit(14));
    AssertTrue( "(PosM & (1 << 35))", g_PosM.TestBit(35));
    AssertFalse("(PosM & (1 << 56))", g_PosM.TestBit(56));
    AssertFalse("(PosM & (1 << 63))", g_PosM.TestBit(63));
    AssertTrue( "(PosM & (1 << 64))", g_PosM.TestBit(64));
    AssertTrue( "(PosM & (1 << 99))", g_PosM.TestBit(99));

    AssertTrue( "(PosL & (1 <<  0))", g_PosL.TestBit( 0));
    AssertTrue( "(PosL & (1 <<  1))", g_PosL.TestBit( 1));
    AssertTrue( "(PosL & (1 <<  7))", g_PosL.TestBit( 7));
    AssertTrue( "(PosL & (1 << 14))", g_PosL.TestBit(14));
    AssertTrue( "(PosL & (1 << 35))", g_PosL.TestBit(35));
    AssertTrue( "(PosL & (1 << 56))", g_PosL.TestBit(56));
    AssertFalse("(PosL & (1 << 63))", g_PosL.TestBit(63));
    AssertTrue( "(PosL & (1 << 64))", g_PosL.TestBit(64));
    AssertTrue( "(PosL & (1 << 99))", g_PosL.TestBit(99));
}

void TestSetBit()
{
    SetTestContext("Long.SetBit");

    AssertStrEq("(NegL | (1 <<  0))", g_NegL.SetBit( 0).ToStringFixed(), "-9223372036854775807");
    AssertStrEq("(NegL | (1 <<  1))", g_NegL.SetBit( 1).ToStringFixed(), "-9223372036854775806");
    AssertStrEq("(NegL | (1 <<  7))", g_NegL.SetBit( 7).ToStringFixed(), "-9223372036854775680");
    AssertStrEq("(NegL | (1 << 14))", g_NegL.SetBit(14).ToStringFixed(), "-9223372036854759424");
    AssertStrEq("(NegL | (1 << 35))", g_NegL.SetBit(35).ToStringFixed(), "-9223372002495037440");
    AssertStrEq("(NegL | (1 << 56))", g_NegL.SetBit(56).ToStringFixed(), "-9151314442816847872");
    AssertStrEq("(NegL | (1 << 63))", g_NegL.SetBit(63).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL | (1 << 64))", g_NegL.SetBit(64).ToStringFixed(), "-9223372036854775807");
    AssertStrEq("(NegL | (1 << 99))", g_NegL.SetBit(99).ToStringFixed(), "-9223372002495037440");

    AssertStrEq("(NegM | (1 <<  0))", g_NegM.SetBit( 0).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM | (1 <<  1))", g_NegM.SetBit( 1).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM | (1 <<  7))", g_NegM.SetBit( 7).ToStringFixed(), "-1234567654193");
    AssertStrEq("(NegM | (1 << 14))", g_NegM.SetBit(14).ToStringFixed(), "-1234567637937");
    AssertStrEq("(NegM | (1 << 35))", g_NegM.SetBit(35).ToStringFixed(), "-1200207915953");
    AssertStrEq("(NegM | (1 << 56))", g_NegM.SetBit(56).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM | (1 << 63))", g_NegM.SetBit(63).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM | (1 << 64))", g_NegM.SetBit(64).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM | (1 << 99))", g_NegM.SetBit(99).ToStringFixed(), "-1200207915953");

    AssertStrEq("(NegS | (1 <<  0))", g_NegS.SetBit( 0).ToStringFixed(), "-1233");
    AssertStrEq("(NegS | (1 <<  1))", g_NegS.SetBit( 1).ToStringFixed(), "-1234");
    AssertStrEq("(NegS | (1 <<  7))", g_NegS.SetBit( 7).ToStringFixed(), "-1106");
    AssertStrEq("(NegS | (1 << 14))", g_NegS.SetBit(14).ToStringFixed(), "-1234");
    AssertStrEq("(NegS | (1 << 35))", g_NegS.SetBit(35).ToStringFixed(), "-1234");
    AssertStrEq("(NegS | (1 << 56))", g_NegS.SetBit(56).ToStringFixed(), "-1234");
    AssertStrEq("(NegS | (1 << 63))", g_NegS.SetBit(63).ToStringFixed(), "-1234");
    AssertStrEq("(NegS | (1 << 64))", g_NegS.SetBit(64).ToStringFixed(), "-1233");
    AssertStrEq("(NegS | (1 << 99))", g_NegS.SetBit(99).ToStringFixed(), "-1234");

    AssertStrEq("(Zero | (1 <<  0))", g_Zero.SetBit( 0).ToStringFixed(), "1");
    AssertStrEq("(Zero | (1 <<  1))", g_Zero.SetBit( 1).ToStringFixed(), "2");
    AssertStrEq("(Zero | (1 <<  7))", g_Zero.SetBit( 7).ToStringFixed(), "128");
    AssertStrEq("(Zero | (1 << 14))", g_Zero.SetBit(14).ToStringFixed(), "16384");
    AssertStrEq("(Zero | (1 << 35))", g_Zero.SetBit(35).ToStringFixed(), "34359738368");
    AssertStrEq("(Zero | (1 << 56))", g_Zero.SetBit(56).ToStringFixed(), "72057594037927936");
    AssertStrEq("(Zero | (1 << 63))", g_Zero.SetBit(63).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(Zero | (1 << 64))", g_Zero.SetBit(64).ToStringFixed(), "1");
    AssertStrEq("(Zero | (1 << 99))", g_Zero.SetBit(99).ToStringFixed(), "34359738368");

    AssertStrEq("(PosS | (1 <<  0))", g_PosS.SetBit( 0).ToStringFixed(), "1235");
    AssertStrEq("(PosS | (1 <<  1))", g_PosS.SetBit( 1).ToStringFixed(), "1234");
    AssertStrEq("(PosS | (1 <<  7))", g_PosS.SetBit( 7).ToStringFixed(), "1234");
    AssertStrEq("(PosS | (1 << 14))", g_PosS.SetBit(14).ToStringFixed(), "17618");
    AssertStrEq("(PosS | (1 << 35))", g_PosS.SetBit(35).ToStringFixed(), "34359739602");
    AssertStrEq("(PosS | (1 << 56))", g_PosS.SetBit(56).ToStringFixed(), "72057594037929170");
    AssertStrEq("(PosS | (1 << 63))", g_PosS.SetBit(63).ToStringFixed(), "-9223372036854774574");
    AssertStrEq("(PosS | (1 << 64))", g_PosS.SetBit(64).ToStringFixed(), "1235");
    AssertStrEq("(PosS | (1 << 99))", g_PosS.SetBit(99).ToStringFixed(), "34359739602");

    AssertStrEq("(PosM | (1 <<  0))", g_PosM.SetBit( 0).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM | (1 <<  1))", g_PosM.SetBit( 1).ToStringFixed(), "1234567654323");
    AssertStrEq("(PosM | (1 <<  7))", g_PosM.SetBit( 7).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM | (1 << 14))", g_PosM.SetBit(14).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM | (1 << 35))", g_PosM.SetBit(35).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM | (1 << 56))", g_PosM.SetBit(56).ToStringFixed(), "72058828605582257");
    AssertStrEq("(PosM | (1 << 63))", g_PosM.SetBit(63).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(PosM | (1 << 64))", g_PosM.SetBit(64).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM | (1 << 99))", g_PosM.SetBit(99).ToStringFixed(), "1234567654321");

    AssertStrEq("(PosL | (1 <<  0))", g_PosL.SetBit( 0).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 <<  1))", g_PosL.SetBit( 1).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 <<  7))", g_PosL.SetBit( 7).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 << 14))", g_PosL.SetBit(14).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 << 35))", g_PosL.SetBit(35).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 << 56))", g_PosL.SetBit(56).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 << 63))", g_PosL.SetBit(63).ToStringFixed(), "-1");
    AssertStrEq("(PosL | (1 << 64))", g_PosL.SetBit(64).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL | (1 << 99))", g_PosL.SetBit(99).ToStringFixed(), "9223372036854775807");
}

void TestClearBit()
{
    SetTestContext("Long.ClearBit");

    AssertStrEq("(NegL & ~(1 <<  0))", g_NegL.ClearBit( 0).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 <<  1))", g_NegL.ClearBit( 1).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 <<  7))", g_NegL.ClearBit( 7).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 << 14))", g_NegL.ClearBit(14).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 << 35))", g_NegL.ClearBit(35).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 << 56))", g_NegL.ClearBit(56).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 << 63))", g_NegL.ClearBit(63).ToStringFixed(), "0");
    AssertStrEq("(NegL & ~(1 << 64))", g_NegL.ClearBit(64).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(NegL & ~(1 << 99))", g_NegL.ClearBit(99).ToStringFixed(), "-9223372036854775808");

    AssertStrEq("(NegM & ~(1 <<  0))", g_NegM.ClearBit( 0).ToStringFixed(), "-1234567654322");
    AssertStrEq("(NegM & ~(1 <<  1))", g_NegM.ClearBit( 1).ToStringFixed(), "-1234567654323");
    AssertStrEq("(NegM & ~(1 <<  7))", g_NegM.ClearBit( 7).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM & ~(1 << 14))", g_NegM.ClearBit(14).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM & ~(1 << 35))", g_NegM.ClearBit(35).ToStringFixed(), "-1234567654321");
    AssertStrEq("(NegM & ~(1 << 56))", g_NegM.ClearBit(56).ToStringFixed(), "-72058828605582257");
    AssertStrEq("(NegM & ~(1 << 63))", g_NegM.ClearBit(63).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(NegM & ~(1 << 64))", g_NegM.ClearBit(64).ToStringFixed(), "-1234567654322");
    AssertStrEq("(NegM & ~(1 << 99))", g_NegM.ClearBit(99).ToStringFixed(), "-1234567654321");

    AssertStrEq("(NegS & ~(1 <<  0))", g_NegS.ClearBit( 0).ToStringFixed(), "-1234");
    AssertStrEq("(NegS & ~(1 <<  1))", g_NegS.ClearBit( 1).ToStringFixed(), "-1236");
    AssertStrEq("(NegS & ~(1 <<  7))", g_NegS.ClearBit( 7).ToStringFixed(), "-1234");
    AssertStrEq("(NegS & ~(1 << 14))", g_NegS.ClearBit(14).ToStringFixed(), "-17618");
    AssertStrEq("(NegS & ~(1 << 35))", g_NegS.ClearBit(35).ToStringFixed(), "-34359739602");
    AssertStrEq("(NegS & ~(1 << 56))", g_NegS.ClearBit(56).ToStringFixed(), "-72057594037929170");
    AssertStrEq("(NegS & ~(1 << 63))", g_NegS.ClearBit(63).ToStringFixed(), "9223372036854774574");
    AssertStrEq("(NegS & ~(1 << 64))", g_NegS.ClearBit(64).ToStringFixed(), "-1234");
    AssertStrEq("(NegS & ~(1 << 99))", g_NegS.ClearBit(99).ToStringFixed(), "-34359739602");

    AssertStrEq("(Zero & ~(1 <<  0))", g_Zero.ClearBit( 0).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 <<  1))", g_Zero.ClearBit( 1).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 <<  7))", g_Zero.ClearBit( 7).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 << 14))", g_Zero.ClearBit(14).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 << 35))", g_Zero.ClearBit(35).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 << 56))", g_Zero.ClearBit(56).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 << 63))", g_Zero.ClearBit(63).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 << 64))", g_Zero.ClearBit(64).ToStringFixed(), "0");
    AssertStrEq("(Zero & ~(1 << 99))", g_Zero.ClearBit(99).ToStringFixed(), "0");

    AssertStrEq("(PosS & ~(1 <<  0))", g_PosS.ClearBit( 0).ToStringFixed(), "1234");
    AssertStrEq("(PosS & ~(1 <<  1))", g_PosS.ClearBit( 1).ToStringFixed(), "1232");
    AssertStrEq("(PosS & ~(1 <<  7))", g_PosS.ClearBit( 7).ToStringFixed(), "1106");
    AssertStrEq("(PosS & ~(1 << 14))", g_PosS.ClearBit(14).ToStringFixed(), "1234");
    AssertStrEq("(PosS & ~(1 << 35))", g_PosS.ClearBit(35).ToStringFixed(), "1234");
    AssertStrEq("(PosS & ~(1 << 56))", g_PosS.ClearBit(56).ToStringFixed(), "1234");
    AssertStrEq("(PosS & ~(1 << 63))", g_PosS.ClearBit(63).ToStringFixed(), "1234");
    AssertStrEq("(PosS & ~(1 << 64))", g_PosS.ClearBit(64).ToStringFixed(), "1234");
    AssertStrEq("(PosS & ~(1 << 99))", g_PosS.ClearBit(99).ToStringFixed(), "1234");

    AssertStrEq("(PosM & ~(1 <<  0))", g_PosM.ClearBit( 0).ToStringFixed(), "1234567654320");
    AssertStrEq("(PosM & ~(1 <<  1))", g_PosM.ClearBit( 1).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM & ~(1 <<  7))", g_PosM.ClearBit( 7).ToStringFixed(), "1234567654193");
    AssertStrEq("(PosM & ~(1 << 14))", g_PosM.ClearBit(14).ToStringFixed(), "1234567637937");
    AssertStrEq("(PosM & ~(1 << 35))", g_PosM.ClearBit(35).ToStringFixed(), "1200207915953");
    AssertStrEq("(PosM & ~(1 << 56))", g_PosM.ClearBit(56).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM & ~(1 << 63))", g_PosM.ClearBit(63).ToStringFixed(), "1234567654321");
    AssertStrEq("(PosM & ~(1 << 64))", g_PosM.ClearBit(64).ToStringFixed(), "1234567654320");
    AssertStrEq("(PosM & ~(1 << 99))", g_PosM.ClearBit(99).ToStringFixed(), "1200207915953");

    AssertStrEq("(PosL & ~(1 <<  0))", g_PosL.ClearBit( 0).ToStringFixed(), "9223372036854775806");
    AssertStrEq("(PosL & ~(1 <<  1))", g_PosL.ClearBit( 1).ToStringFixed(), "9223372036854775805");
    AssertStrEq("(PosL & ~(1 <<  7))", g_PosL.ClearBit( 7).ToStringFixed(), "9223372036854775679");
    AssertStrEq("(PosL & ~(1 << 14))", g_PosL.ClearBit(14).ToStringFixed(), "9223372036854759423");
    AssertStrEq("(PosL & ~(1 << 35))", g_PosL.ClearBit(35).ToStringFixed(), "9223372002495037439");
    AssertStrEq("(PosL & ~(1 << 56))", g_PosL.ClearBit(56).ToStringFixed(), "9151314442816847871");
    AssertStrEq("(PosL & ~(1 << 63))", g_PosL.ClearBit(63).ToStringFixed(), "9223372036854775807");
    AssertStrEq("(PosL & ~(1 << 64))", g_PosL.ClearBit(64).ToStringFixed(), "9223372036854775806");
    AssertStrEq("(PosL & ~(1 << 99))", g_PosL.ClearBit(99).ToStringFixed(), "9223372002495037439");
}

void TestFlipBit()
{
    SetTestContext("Long.FlipBit");

    AssertStrEq("(NegL ^ (1 <<  0))", g_NegL.FlipBit( 0).ToStringFixed(), "-9223372036854775807");
    AssertStrEq("(NegL ^ (1 <<  1))", g_NegL.FlipBit( 1).ToStringFixed(), "-9223372036854775806");
    AssertStrEq("(NegL ^ (1 <<  7))", g_NegL.FlipBit( 7).ToStringFixed(), "-9223372036854775680");
    AssertStrEq("(NegL ^ (1 << 14))", g_NegL.FlipBit(14).ToStringFixed(), "-9223372036854759424");
    AssertStrEq("(NegL ^ (1 << 35))", g_NegL.FlipBit(35).ToStringFixed(), "-9223372002495037440");
    AssertStrEq("(NegL ^ (1 << 56))", g_NegL.FlipBit(56).ToStringFixed(), "-9151314442816847872");
    AssertStrEq("(NegL ^ (1 << 63))", g_NegL.FlipBit(63).ToStringFixed(), "0");
    AssertStrEq("(NegL ^ (1 << 64))", g_NegL.FlipBit(64).ToStringFixed(), "-9223372036854775807");
    AssertStrEq("(NegL ^ (1 << 99))", g_NegL.FlipBit(99).ToStringFixed(), "-9223372002495037440");

    AssertStrEq("(NegM ^ (1 <<  0))", g_NegM.FlipBit( 0).ToStringFixed(), "-1234567654322");
    AssertStrEq("(NegM ^ (1 <<  1))", g_NegM.FlipBit( 1).ToStringFixed(), "-1234567654323");
    AssertStrEq("(NegM ^ (1 <<  7))", g_NegM.FlipBit( 7).ToStringFixed(), "-1234567654193");
    AssertStrEq("(NegM ^ (1 << 14))", g_NegM.FlipBit(14).ToStringFixed(), "-1234567637937");
    AssertStrEq("(NegM ^ (1 << 35))", g_NegM.FlipBit(35).ToStringFixed(), "-1200207915953");
    AssertStrEq("(NegM ^ (1 << 56))", g_NegM.FlipBit(56).ToStringFixed(), "-72058828605582257");
    AssertStrEq("(NegM ^ (1 << 63))", g_NegM.FlipBit(63).ToStringFixed(), "9223370802287121487");
    AssertStrEq("(NegM ^ (1 << 64))", g_NegM.FlipBit(64).ToStringFixed(), "-1234567654322");
    AssertStrEq("(NegM ^ (1 << 99))", g_NegM.FlipBit(99).ToStringFixed(), "-1200207915953");

    AssertStrEq("(NegS ^ (1 <<  0))", g_NegS.FlipBit( 0).ToStringFixed(), "-1233");
    AssertStrEq("(NegS ^ (1 <<  1))", g_NegS.FlipBit( 1).ToStringFixed(), "-1236");
    AssertStrEq("(NegS ^ (1 <<  7))", g_NegS.FlipBit( 7).ToStringFixed(), "-1106");
    AssertStrEq("(NegS ^ (1 << 14))", g_NegS.FlipBit(14).ToStringFixed(), "-17618");
    AssertStrEq("(NegS ^ (1 << 35))", g_NegS.FlipBit(35).ToStringFixed(), "-34359739602");
    AssertStrEq("(NegS ^ (1 << 56))", g_NegS.FlipBit(56).ToStringFixed(), "-72057594037929170");
    AssertStrEq("(NegS ^ (1 << 63))", g_NegS.FlipBit(63).ToStringFixed(), "9223372036854774574");
    AssertStrEq("(NegS ^ (1 << 64))", g_NegS.FlipBit(64).ToStringFixed(), "-1233");
    AssertStrEq("(NegS ^ (1 << 99))", g_NegS.FlipBit(99).ToStringFixed(), "-34359739602");

    AssertStrEq("(Zero ^ (1 <<  0))", g_Zero.FlipBit( 0).ToStringFixed(), "1");
    AssertStrEq("(Zero ^ (1 <<  1))", g_Zero.FlipBit( 1).ToStringFixed(), "2");
    AssertStrEq("(Zero ^ (1 <<  7))", g_Zero.FlipBit( 7).ToStringFixed(), "128");
    AssertStrEq("(Zero ^ (1 << 14))", g_Zero.FlipBit(14).ToStringFixed(), "16384");
    AssertStrEq("(Zero ^ (1 << 35))", g_Zero.FlipBit(35).ToStringFixed(), "34359738368");
    AssertStrEq("(Zero ^ (1 << 56))", g_Zero.FlipBit(56).ToStringFixed(), "72057594037927936");
    AssertStrEq("(Zero ^ (1 << 63))", g_Zero.FlipBit(63).ToStringFixed(), "-9223372036854775808");
    AssertStrEq("(Zero ^ (1 << 64))", g_Zero.FlipBit(64).ToStringFixed(), "1");
    AssertStrEq("(Zero ^ (1 << 99))", g_Zero.FlipBit(99).ToStringFixed(), "34359738368");

    AssertStrEq("(PosS ^ (1 <<  0))", g_PosS.FlipBit( 0).ToStringFixed(), "1235");
    AssertStrEq("(PosS ^ (1 <<  1))", g_PosS.FlipBit( 1).ToStringFixed(), "1232");
    AssertStrEq("(PosS ^ (1 <<  7))", g_PosS.FlipBit( 7).ToStringFixed(), "1106");
    AssertStrEq("(PosS ^ (1 << 14))", g_PosS.FlipBit(14).ToStringFixed(), "17618");
    AssertStrEq("(PosS ^ (1 << 35))", g_PosS.FlipBit(35).ToStringFixed(), "34359739602");
    AssertStrEq("(PosS ^ (1 << 56))", g_PosS.FlipBit(56).ToStringFixed(), "72057594037929170");
    AssertStrEq("(PosS ^ (1 << 63))", g_PosS.FlipBit(63).ToStringFixed(), "-9223372036854774574");
    AssertStrEq("(PosS ^ (1 << 64))", g_PosS.FlipBit(64).ToStringFixed(), "1235");
    AssertStrEq("(PosS ^ (1 << 99))", g_PosS.FlipBit(99).ToStringFixed(), "34359739602");

    AssertStrEq("(PosM ^ (1 <<  0))", g_PosM.FlipBit( 0).ToStringFixed(), "1234567654320");
    AssertStrEq("(PosM ^ (1 <<  1))", g_PosM.FlipBit( 1).ToStringFixed(), "1234567654323");
    AssertStrEq("(PosM ^ (1 <<  7))", g_PosM.FlipBit( 7).ToStringFixed(), "1234567654193");
    AssertStrEq("(PosM ^ (1 << 14))", g_PosM.FlipBit(14).ToStringFixed(), "1234567637937");
    AssertStrEq("(PosM ^ (1 << 35))", g_PosM.FlipBit(35).ToStringFixed(), "1200207915953");
    AssertStrEq("(PosM ^ (1 << 56))", g_PosM.FlipBit(56).ToStringFixed(), "72058828605582257");
    AssertStrEq("(PosM ^ (1 << 63))", g_PosM.FlipBit(63).ToStringFixed(), "-9223370802287121487");
    AssertStrEq("(PosM ^ (1 << 64))", g_PosM.FlipBit(64).ToStringFixed(), "1234567654320");
    AssertStrEq("(PosM ^ (1 << 99))", g_PosM.FlipBit(99).ToStringFixed(), "1200207915953");

    AssertStrEq("(PosL ^ (1 <<  0))", g_PosL.FlipBit( 0).ToStringFixed(), "9223372036854775806");
    AssertStrEq("(PosL ^ (1 <<  1))", g_PosL.FlipBit( 1).ToStringFixed(), "9223372036854775805");
    AssertStrEq("(PosL ^ (1 <<  7))", g_PosL.FlipBit( 7).ToStringFixed(), "9223372036854775679");
    AssertStrEq("(PosL ^ (1 << 14))", g_PosL.FlipBit(14).ToStringFixed(), "9223372036854759423");
    AssertStrEq("(PosL ^ (1 << 35))", g_PosL.FlipBit(35).ToStringFixed(), "9223372002495037439");
    AssertStrEq("(PosL ^ (1 << 56))", g_PosL.FlipBit(56).ToStringFixed(), "9151314442816847871");
    AssertStrEq("(PosL ^ (1 << 63))", g_PosL.FlipBit(63).ToStringFixed(), "-1");
    AssertStrEq("(PosL ^ (1 << 64))", g_PosL.FlipBit(64).ToStringFixed(), "9223372036854775806");
    AssertStrEq("(PosL ^ (1 << 99))", g_PosL.FlipBit(99).ToStringFixed(), "9223372002495037439");
}

void TestCompare()
{
    AssertEq(
        "(1234567654321 == (-1234567))",
        StringToLong("1234567654321").Compare(IntToLong(-1234567)),
        1);

    AssertEq(
        "(1234567654321 == 0)",
        StringToLong("1234567654321").Compare(IntToLong(0)),
        1);

    AssertEq(
        "(1234567654321 == 12)",
        StringToLong("1234567654321").Compare(IntToLong(12)),
        1);

    AssertEq(
        "(1234567654321 == 45)",
        StringToLong("1234567654321").Compare(IntToLong(45)),
        1);

    AssertEq(
        "(1234567654321 == 1234567)",
        StringToLong("1234567654321").Compare(IntToLong(1234567)),
        1);

    AssertEq(
        "(1234567654321 == 200000000000)",
        StringToLong("1234567654321").Compare(StringToLong("200000000000")),
        1);

    AssertEq(
        "(4296201863 == 4296201862)",
        StringToLong("4296201863").Compare(StringToLong("4296201862")),
        1);

    AssertEq(
        "(4296201863 == 4296201863)",
        StringToLong("4296201863").Compare(StringToLong("4296201863")),
        0);

    AssertEq(
        "(1234567654321 == 1234567654321)",
        StringToLong("1234567654321").Compare(StringToLong("1234567654321")),
        0);

    AssertEq(
        "(1234567654321 == 12345676543210)",
        StringToLong("1234567654321").Compare(StringToLong("12345676543210")),
        -1);

    AssertEq(
        "(100000000000 == 300000000000)",
        StringToLong("100000000000").Compare(StringToLong("300000000000")),
        -1);

    AssertEq(
        "100000000000 == 10",
        StringToLong("100000000000").Compare(IntToLong(10)),
        1);

    AssertEq(
        "100000000000 == 11",
        StringToLong("100000000000").Compare(IntToLong(11)),
        1);

    AssertEq(
        "200000000000 == 200000000000",
        StringToLong("200000000000").Compare(StringToLong("200000000000")),
        0);

    AssertEq(
        "-200000000000 == -200000000000",
        StringToLong("-200000000000").Compare(StringToLong("-200000000000")),
        0);

    AssertEq(
        "-450000000000 != -45",
        StringToLong("-450000000000").Compare(IntToLong(-45)),
        -1);

    AssertEq(
        "-450000000000 != -5",
        StringToLong("-450000000000").Compare(IntToLong(-5)),
        -1);
}
