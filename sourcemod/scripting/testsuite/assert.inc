#if defined _assert_included
    #endinput
#endif
#define _assert_included

#if defined NDEBUG || (!defined DEBUG && !defined _DEBUG)
    #define SetTestContext(%1)                      __AssertNull()

    #define AssertTrue(%1,%2)                       __AssertNull()
    #define AssertFalse(%1,%2)                      __AssertNull()

    #define AssertLt(%1,%2,%3)                      __AssertNull()
    #define AssertLe(%1,%2,%3)                      __AssertNull()
    #define AssertEq(%1,%2,%3)                      __AssertNull()
    #define AssertNe(%1,%2,%3)                      __AssertNull()
    #define AssertGe(%1,%2,%3)                      __AssertNull()
    #define AssertGt(%1,%2,%3)                      __AssertNull()

    #define AssertArrayEq(%1,%2,%3,%4)              __AssertNull()
    #define AssertArrayNe(%1,%2,%3,%4)              __AssertNull()
    #define AssertArray2DEq(%1,%2,%3,%4,%5)         __AssertNull()
    #define AssertArray2DNe(%1,%2,%3,%4,%5)         __AssertNull()

    #define AssertFloatEq(%1,%2,%3)                 __AssertNull()
    #define AssertFloatNe(%1,%2,%3)                 __AssertNull()
    #define AssertFloatArrayEq(%1,%2,%3,%4)         __AssertNull()
    #define AssertFloatArrayNe(%1,%2,%3,%4)         __AssertNull()
    #define AssertFloatArray2DEq(%1,%2,%3,%4,%5)    __AssertNull()
    #define AssertFloatArray2DNe(%1,%2,%3,%4,%5)    __AssertNull()

    #define AssertStrEq(%1,%2,%3)                   __AssertNull()
    #define AssertStrNe(%1,%2,%3)                   __AssertNull()
    #define AssertStrArrayEq(%1,%2,%3,%4)           __AssertNull()
    #define AssertStrArrayNe(%1,%2,%3,%4)           __AssertNull()
    #define AssertStrContains(%1,%2,%3)             __AssertNull()
    #define AssertStrNotContains(%1,%2,%3)          __AssertNull()
    #define AssertStrStartsWith(%1,%2,%3)           __AssertNull()
    #define AssertStrNotStartsWith(%1,%2,%3)        __AssertNull()
    #define AssertStrEndsWith(%1,%2,%3)             __AssertNull()
    #define AssertStrNotEndsWith(%1,%2,%3)          __AssertNull()

    #define AssertFilesLt(%1,%2,%3)                 __AssertNull()
    #define AssertFilesLe(%1,%2,%3)                 __AssertNull()
    #define AssertFilesEq(%1,%2,%3)                 __AssertNull()
    #define AssertFilesNe(%1,%2,%3)                 __AssertNull()
    #define AssertFilesGe(%1,%2,%3)                 __AssertNull()
    #define AssertFilesGt(%1,%2,%3)                 __AssertNull()
    #define AssertFileLinesLt(%1,%2,%3)             __AssertNull()
    #define AssertFileLinesLe(%1,%2,%3)             __AssertNull()
    #define AssertFileLinesEq(%1,%2,%3)             __AssertNull()
    #define AssertFileLinesNe(%1,%2,%3)             __AssertNull()
    #define AssertFileLinesGe(%1,%2,%3)             __AssertNull()
    #define AssertFileLinesGt(%1,%2,%3)             __AssertNull()

    // #include <regex>
    #if defined _regex_included
    #define AssertStrMatch(%1,%2,%3)                __AssertNull()
    #define AssertStrNotMatch(%1,%2,%3)             __AssertNull()
    #define AssertFileMatch(%1,%2,%3)               __AssertNull()
    #define AssertFileNotMatch(%1,%2,%3)            __AssertNull()
    #endif

    #if defined SM_INT64_SUPPORTED
    #define AssertInt64Lt(%1,%2,%3)                 __AssertNull()
    #define AssertInt64Le(%1,%2,%3)                 __AssertNull()
    #define AssertInt64Eq(%1,%2,%3)                 __AssertNull()
    #define AssertInt64Ne(%1,%2,%3)                 __AssertNull()
    #define AssertInt64Ge(%1,%2,%3)                 __AssertNull()
    #define AssertInt64Gt(%1,%2,%3)                 __AssertNull()
    #endif

    stock void __AssertNull() {}
    #endinput
#endif


// *** Optional *** //
#if !defined ASSERT_OPTION_MAX_CONTEXT_LENGTH
    #define  ASSERT_OPTION_MAX_CONTEXT_LENGTH   128
#endif

#if !defined ASSERT_OPTION_FLOAT_THRESHOLD
    #define  ASSERT_OPTION_FLOAT_THRESHOLD      0.000001
#endif


static int  __Assert_iCounter = 0;
static char __Assert_sSuiteName[ASSERT_OPTION_MAX_CONTEXT_LENGTH] = {'\0', ...};


// *** Optional code modification to customize processing logic on failure/success *** //
static void ASSERT_OPTION_ON_FAIL(const char[] name, char[] fmt, any ...)
{
    char buffer[512];
    VFormat(buffer, sizeof(buffer), fmt, 3);
    ThrowError("[%2u] [FAIL] [%s] %s%s%s.", __Assert_iCounter, __Assert_sSuiteName, name, (name[0] ? " " : ""), buffer);
}

static void ASSERT_OPTION_ON_OK(const char[] name, char[] fmt, any ...)
{
#if defined ASSERT_QUIET
    #pragma unused name
    #pragma unused fmt
#else
    char buffer[1024];
    VFormat(buffer, sizeof(buffer), fmt, 3);
    PrintToServer("[%2u] [ OK ] [%s] %s%s%s.", __Assert_iCounter, __Assert_sSuiteName, name, (name[0] ? " " : ""), buffer);
#endif
}


// *** Stocks *** //
stock void SetTestContext(const char[] suiteName)
{
    strcopy(__Assert_sSuiteName, sizeof(__Assert_sSuiteName), suiteName);
}

stock void AssertTrue(const char[] name, bool value)
{
    __Assert_iCounter++;
    bool ok = value;
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value == true, got false");
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value == true");
    }
}

stock void AssertFalse(const char[] name, bool value)
{
    __Assert_iCounter++;
    bool ok = !value;
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value == false, got true");
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value == false");
    }
}

stock void AssertLt(const char[] name, any value, any expected)
{
    __Assert_iCounter++;
    bool ok = (value < expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value < %d, got %d", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %d < %d", value, expected);
    }
}

stock void AssertLe(const char[] name, any value, any expected)
{
    __Assert_iCounter++;
    bool ok = (value <= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value <= %d, got %d", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %d <= %d", value, expected);
    }
}

stock void AssertEq(const char[] name, any value, any expected)
{
    __Assert_iCounter++;
    bool ok = (value == expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value == %d, got %d", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value == %d", expected);
    }
}

stock void AssertNe(const char[] name, any value, any expected)
{
    __Assert_iCounter++;
    bool ok = (value != expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value != %d, got equals", expected);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %d != %d", value, expected);
    }
}

stock void AssertGe(const char[] name, any value, any expected)
{
    __Assert_iCounter++;
    bool ok = (value >= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value >= %d, got %d", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %d >= %d", value, expected);
    }
}

stock void AssertGt(const char[] name, any value, any expected)
{
    __Assert_iCounter++;
    bool ok = (value > expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value > %d, got %d", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %d > %d", value, expected);
    }
}

stock void AssertArrayEq(const char[] name, const any[] value, const any[] expected, int length)
{
    __Assert_iCounter++;
    bool ok = __ArrayEq(value, expected, length);
    if (!ok)
    {
        char valueStr[512], expectedStr[512];
        __ArrayToString(valueStr, sizeof(valueStr), value, length);
        __ArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_FAIL(name, "expected array == %s, got %s", expectedStr, valueStr);
    }
    else
    {
        char expectedStr[1024];
        __ArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_OK(name, "array == %s", expectedStr);
    }
}

stock void AssertArrayNe(const char[] name, const any[] value, const any[] expected, int length)
{
    __Assert_iCounter++;
    bool ok = !__ArrayEq(value, expected, length);
    if (!ok)
    {
        char expectedStr[1024];
        __ArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_FAIL(name, "expected array != %s, got equals", expectedStr);
    }
    else
    {
        char valueStr[512], expectedStr[512];
        __ArrayToString(valueStr, sizeof(valueStr), value, length);
        __ArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_OK(name, "array %s != %s", valueStr, expectedStr);
    }
}

stock void AssertArray2DEq(const char[] name, const any[][] value, const any[][] expected, int rows, int cols)
{
    __Assert_iCounter++;
    bool ok = __Array2DEq(value, expected, rows, cols);
    if (!ok)
    {
        char valueStr[512], expectedStr[512];
        __Array2DToString(valueStr, sizeof(valueStr), value, rows, cols);
        __Array2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_FAIL(name, "expected array 2D == %s, got %s", expectedStr, valueStr);
    }
    else
    {
        char expectedStr[1024];
        __Array2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_OK(name, "array 2D == %s", expectedStr);
    }
}

stock void AssertArray2DNe(const char[] name, const any[][] value, const any[][] expected, int rows, int cols)
{
    __Assert_iCounter++;
    bool ok = !__Array2DEq(value, expected, rows, cols);
    if (!ok)
    {
        char expectedStr[1024];
        __Array2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_FAIL(name, "expected array 2D != %s, got equals", expectedStr);
    }
    else
    {
        char valueStr[512], expectedStr[512];
        __Array2DToString(valueStr, sizeof(valueStr), value, rows, cols);
        __Array2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_OK(name, "array 2D %s != %s", valueStr, expectedStr);
    }
}

stock void AssertFloatEq(const char[] name, float value, float expected)
{
    __Assert_iCounter++;
    bool ok = __FloatNearlyEq(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value == %f, got %f", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value == %f", expected);
    }
}

stock void AssertFloatNe(const char[] name, float value, float expected)
{
    __Assert_iCounter++;
    bool ok = !__FloatNearlyEq(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value != %f, got equals", expected);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %f != %f", value, expected);
    }
}

stock void AssertFloatArrayEq(const char[] name, const float[] value, const float[] expected, int length)
{
    __Assert_iCounter++;
    bool ok = __FloatArrayEq(value, expected, length);
    if (!ok)
    {
        char valueStr[512], expectedStr[512];
        __FloatArrayToString(valueStr, sizeof(valueStr), value, length);
        __FloatArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_FAIL(name, "expected array == %s, got %s", expectedStr, valueStr);
    }
    else
    {
        char expectedStr[1024];
        __FloatArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_OK(name, "array == %s", expectedStr);
    }
}

stock void AssertFloatArrayNe(const char[] name, const float[] value, const float[] expected, int length)
{
    __Assert_iCounter++;
    bool ok = !__FloatArrayEq(value, expected, length);
    if (!ok)
    {
        char expectedStr[1024];
        __FloatArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_FAIL(name, "expected array != %s, got equals", expectedStr);
    }
    else
    {
        char valueStr[512], expectedStr[512];
        __FloatArrayToString(valueStr, sizeof(valueStr), value, length);
        __FloatArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_OK(name, "array %s != %s", valueStr, expectedStr);
    }
}

stock void AssertFloatArray2DEq(const char[] name, const any[][] value, const any[][] expected, int rows, int cols)
{
    __Assert_iCounter++;
    bool ok = __FloatArray2DEq(value, expected, rows, cols);
    if (!ok)
    {
        char valueStr[512], expectedStr[512];
        __FloatArray2DToString(valueStr, sizeof(valueStr), value, rows, cols);
        __FloatArray2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_FAIL(name, "expected array 2D == %s, got %s", expectedStr, valueStr);
    }
    else
    {
        char expectedStr[1024];
        __FloatArray2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_OK(name, "array 2D == %s", expectedStr);
    }
}

stock void AssertFloatArray2DNe(const char[] name, const any[][] value, const any[][] expected, int rows, int cols)
{
    __Assert_iCounter++;
    bool ok = !__FloatArray2DEq(value, expected, rows, cols);
    if (!ok)
    {
        char expectedStr[1024];
        __FloatArray2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_FAIL(name, "expected array 2D != %s, got equals", expectedStr);
    }
    else
    {
        char valueStr[512], expectedStr[512];
        __FloatArray2DToString(valueStr, sizeof(valueStr), value, rows, cols);
        __FloatArray2DToString(expectedStr, sizeof(expectedStr), expected, rows, cols);
        ASSERT_OPTION_ON_OK(name, "array 2D %s != %s", valueStr, expectedStr);
    }
}

stock void AssertStrEq(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = StrEqual(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string == \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string == \"%s\"", expected);
    }
}

stock void AssertStrNe(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = !StrEqual(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string != \"%s\", got equals", expected);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" != \"%s\"", value, expected);
    }
}

stock void AssertStrArrayEq(const char[] name, const char[][] value, const char[][] expected, int length)
{
    __Assert_iCounter++;
    bool ok = true;
    int index = -1;
    for (int i = 0; i < length; ++i)
    {
        if (!StrEqual(value[i], expected[i]))
        {
            ok = false;
            index = i;
            break;
        }
    }

    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected array[%u] == \"%s\", got \"%s\"", index, expected[index], value[index]);
    }
    else
    {
        char expectedStr[1024];
        __StrArrayToString(expectedStr, sizeof(expectedStr), value, length);
        ASSERT_OPTION_ON_OK(name, "array == %s", expectedStr);
    }
}

stock void AssertStrArrayNe(const char[] name, const char[][] value, const char[][] expected, int length)
{
    __Assert_iCounter++;
    bool ok = true;
    int index = -1;
    for (int i = 0; i < length; ++i)
    {
        if (StrEqual(value[i], expected[i]))
        {
            ok = false;
            index = i;
            break;
        }
    }

    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected array[%u] != \"%s\", got equals", index, expected[index], value[index]);
    }
    else
    {
        char valueStr[512], expectedStr[512];
        __StrArrayToString(valueStr, sizeof(valueStr), value, length);
        __StrArrayToString(expectedStr, sizeof(expectedStr), expected, length);
        ASSERT_OPTION_ON_OK(name, "array %s != %s", valueStr, expectedStr);
    }
}

stock void AssertStrContains(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = (StrContains(value, expected) != -1);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string contains \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" contains \"%s\"", value, expected);
    }
}

stock void AssertStrNotContains(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = (StrContains(value, expected) == -1);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string not contains \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" not contains \"%s\"", value, expected);
    }
}

stock void AssertStrStartsWith(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = __StrStartsWith(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string starts with \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" starts with \"%s\"", value, expected);
    }
}

stock void AssertStrNotStartsWith(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = !__StrStartsWith(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string not starts with \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" not starts with \"%s\"", value, expected);
    }
}

stock void AssertStrEndsWith(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = __StrEndsWith(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string ends with \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" ends with \"%s\"", value, expected);
    }
}

stock void AssertStrNotEndsWith(const char[] name, const char[] value, const char[] expected)
{
    __Assert_iCounter++;
    bool ok = !__StrEndsWith(value, expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string not ends with \"%s\", got \"%s\"", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string \"%s\" not ends with \"%s\"", value, expected);
    }
}

stock void AssertFilesLt(const char[] name, const char[] directory, int expected)
{
    __Assert_iCounter++;
    char buffer[PLATFORM_MAX_PATH];
    __SplitPathByFile(buffer, sizeof(buffer), "", 0, directory);
    int value = __CountFiles(buffer);
    bool ok = (value < expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected directory \"%s\" files < %u, got %u", buffer, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "directory \"%s\" files %u < %u", buffer, value, expected);
    }
}

stock void AssertFilesLe(const char[] name, const char[] directory, int expected)
{
    __Assert_iCounter++;
    char buffer[PLATFORM_MAX_PATH];
    __SplitPathByFile(buffer, sizeof(buffer), "", 0, directory);
    int value = __CountFiles(buffer);
    bool ok = (value <= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected directory \"%s\" files <= %u, got %u", buffer, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "directory \"%s\" files %u <= %u", buffer, value, expected);
    }
}

stock void AssertFilesEq(const char[] name, const char[] directory, int expected)
{
    __Assert_iCounter++;
    char buffer[PLATFORM_MAX_PATH];
    __SplitPathByFile(buffer, sizeof(buffer), "", 0, directory);
    int value = __CountFiles(buffer);
    bool ok = (value == expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected directory \"%s\" files == %u, got %u", buffer, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "directory \"%s\" files == %u", buffer, expected);
    }
}

stock void AssertFilesNe(const char[] name, const char[] directory, int expected)
{
    __Assert_iCounter++;
    char buffer[PLATFORM_MAX_PATH];
    __SplitPathByFile(buffer, sizeof(buffer), "", 0, directory);
    int value = __CountFiles(buffer);
    bool ok = (value != expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected directory \"%s\" files != %u, got equals", buffer, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "directory \"%s\" files %u != %u", buffer, value, expected);
    }
}

stock void AssertFilesGe(const char[] name, const char[] directory, int expected)
{
    __Assert_iCounter++;
    char buffer[PLATFORM_MAX_PATH];
    __SplitPathByFile(buffer, sizeof(buffer), "", 0, directory);
    int value = __CountFiles(buffer);
    bool ok = (value >= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected directory \"%s\" files >= %u, got %u", buffer, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "directory \"%s\" files %u >= %u", buffer, value, expected);
    }
}

stock void AssertFilesGt(const char[] name, const char[] directory, int expected)
{
    __Assert_iCounter++;
    char buffer[PLATFORM_MAX_PATH];
    __SplitPathByFile(buffer, sizeof(buffer), "", 0, directory);
    int value = __CountFiles(buffer);
    bool ok = (value > expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected directory \"%s\" files > %u, got %u", buffer, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "directory \"%s\" files %u > %u", buffer, value, expected);
    }
}

stock void AssertFileLinesLt(const char[] name, const char[] filename, int expected)
{
    __Assert_iCounter++;
    int value = __CountFileLines(filename);
    bool ok = (value < expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" lines < %u, got %u", filename, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" lines %u < %u", filename, value, expected);
    }
}

stock void AssertFileLinesLe(const char[] name, const char[] filename, int expected)
{
    __Assert_iCounter++;
    int value = __CountFileLines(filename);
    bool ok = (value <= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" lines <= %u, got %u", filename, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" lines %u <= %u", filename, value, expected);
    }
}

stock void AssertFileLinesEq(const char[] name, const char[] filename, int expected)
{
    __Assert_iCounter++;
    int value = __CountFileLines(filename);
    bool ok = (value == expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" lines == %u, got %u", filename, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" lines == %u", filename, expected);
    }
}

stock void AssertFileLinesNe(const char[] name, const char[] filename, int expected)
{
    __Assert_iCounter++;
    int value = __CountFileLines(filename);
    bool ok = (value != expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" lines != %u, got equals", filename, expected);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" lines %u != %u", filename, value, expected);
    }
}

stock void AssertFileLinesGe(const char[] name, const char[] filename, int expected)
{
    __Assert_iCounter++;
    int value = __CountFileLines(filename);
    bool ok = (value >= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" lines >= %u, got %u", filename, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" lines %u >= %u", filename, value, expected);
    }
}

stock void AssertFileLinesGt(const char[] name, const char[] filename, int expected)
{
    __Assert_iCounter++;
    int value = __CountFileLines(filename);
    bool ok = (value > expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" lines > %u, got %u", filename, expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" lines %u > %u", filename, value, expected);
    }
}

#if defined _regex_included
    #if !defined AUTOLOAD_EXTENSIONS || !defined REQUIRE_EXTENSIONS
        #warning "Regex natives require extensions."
    #endif
stock void AssertStrMatch(const char[] name, const char[] value, const char[] expectedPattern)
{
    __Assert_iCounter++;
    char error[512];
    int substrings = SimpleRegexMatch(value, expectedPattern, _, error, sizeof(error));
    if (substrings == -1)
        ThrowError("Regular matching error %s.", error);

    bool ok = (substrings > 0);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string match \"%s\", got \"%s\"", expectedPattern, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string match \"%s\"", expectedPattern);
    }
}

stock void AssertStrNotMatch(const char[] name, const char[] value, const char[] expectedPattern)
{
    __Assert_iCounter++;
    char error[512];
    int substrings = SimpleRegexMatch(value, expectedPattern, _, error, sizeof(error));
    if (substrings == -1)
        ThrowError("Regular matching error %s.", error);

    bool ok = (substrings <= 0);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected string not match \"%s\", got \"%s\"", expectedPattern, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "string not match \"%s\"", expectedPattern);
    }
}

stock void AssertFileMatch(const char[] name, const char[] filename, const char[] expectedPattern)
{
    __Assert_iCounter++;
    int size = FileSize(filename) + 1;
    char[] value = new char[size];

    File file = OpenFile(filename, "r");
    file.ReadString(value, size);
    delete file;

    char error[256];
    int substrings = SimpleRegexMatch(value, expectedPattern, _, error, sizeof(error));
    if (substrings == -1)
        ThrowError("Regular matching error %s.", error);

    bool ok = (substrings > 0);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" match \"%s\", got not match", filename, expectedPattern, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" match \"%s\"", filename, expectedPattern);
    }
}

stock void AssertFileNotMatch(const char[] name, const char[] filename, const char[] expectedPattern)
{
    __Assert_iCounter++;
    int size = FileSize(filename) + 1;
    char[] value = new char[size];

    File file = OpenFile(filename, "r");
    file.ReadString(value, size);
    delete file;

    char error[256];
    int substrings = SimpleRegexMatch(value, expectedPattern, _, error, sizeof(error));
    if (substrings == -1)
        ThrowError("Regular matching error %s.", error);

    bool ok = (substrings <= 0);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected file \"%s\" not match \"%s\", got match", filename, expectedPattern, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "file \"%s\" not match \"%s\"", filename, expectedPattern);
    }
}
#endif

#if defined SM_INT64_SUPPORTED
stock void AssertInt64Lt(const char[] name, int64 value, int64 expected)
{
    __Assert_iCounter++;
    bool ok = (value < expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value < %ld, got %ld", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %ld < %ld", value, expected);
    }
}

stock void AssertInt64Le(const char[] name, int64 value, int64 expected)
{
    __Assert_iCounter++;
    bool ok = (value <= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value <= %ld, got %ld", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %ld <= %ld", value, expected);
    }
}

stock void AssertInt64Eq(const char[] name, int64 value, int64 expected)
{
    __Assert_iCounter++;
    bool ok = (value == expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value == %ld, got %ld", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value == %ld", expected);
    }
}

stock void AssertInt64Ne(const char[] name, int64 value, int64 expected)
{
    __Assert_iCounter++;
    bool ok = (value != expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value != %ld, got equals", expected);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %ld != %ld", value, expected);
    }
}

stock void AssertInt64Ge(const char[] name, int64 value, int64 expected)
{
    __Assert_iCounter++;
    bool ok = (value >= expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value >= %ld, got %ld", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %ld >= %ld", value, expected);
    }
}

stock void AssertInt64Gt(const char[] name, int64 value, int64 expected)
{
    __Assert_iCounter++;
    bool ok = (value > expected);
    if (!ok)
    {
        ASSERT_OPTION_ON_FAIL(name, "expected value > %ld, got %ld", expected, value);
    }
    else
    {
        ASSERT_OPTION_ON_OK(name, "value %ld > %ld", value, expected);
    }
}
#endif



// *** Do not use the function below this line! *** //
static int __ArrayToString(char[] buffer, int maxlen, const any[] value, int length)
{
    for (int i = 0; i < length; ++i)
    {
        if (i == 0) FormatEx(buffer, maxlen, "%d", value[i]);
        else        Format(buffer, maxlen, "%s, %d", buffer, value[i]);
    }
    return Format(buffer, maxlen, "[%s]", buffer);
}

static int __Array2DToString(char[] buffer, int maxlen, const any[][] value, int rows, int cols)
{
    char[] line = new char[maxlen];
    for (int i = 0; i < rows; ++i)
    {
        __ArrayToString(line, maxlen, value[i], cols);
        if (i == 0) FormatEx(buffer, maxlen, "[%s]", line);
        else        Format(buffer, maxlen, "%s, [%s]", buffer, line);
    }
    return Format(buffer, maxlen, "[%s]", buffer);
}

static int __FloatArrayToString(char[] buffer, int maxlen, const float[] value, int length)
{
    for (int i = 0; i < length; ++i)
    {
        if (i == 0) FormatEx(buffer, maxlen, "%.1f", value[i]);
        else        Format(buffer, maxlen, "%s, %.1f", buffer, value[i]);
    }
    return Format(buffer, maxlen, "[%s]", buffer);
}

static int __FloatArray2DToString(char[] buffer, int maxlen, const float[][] value, int rows, int cols)
{
    char[] line = new char[maxlen];
    for (int i = 0; i < rows; ++i)
    {
        __FloatArrayToString(line, maxlen, value[i], cols);
        if (i == 0) FormatEx(buffer, maxlen, "[%s]", line);
        else        Format(buffer, maxlen, "%s, [%s]", buffer, line);
    }
    return Format(buffer, maxlen, "[%s]", buffer);
}

static int __StrArrayToString(char[] buffer, int maxlen, const char[][] value, int length)
{
    for (int i = 0; i < length; ++i)
    {
        if (i == 0) FormatEx(buffer, maxlen, "\"%s\"", value[i]);
        else        Format(buffer, maxlen, "%s, \"%s\"", buffer, value[i]);
    }
    return Format(buffer, maxlen, "[%s]", buffer);
}

static bool __ArrayEq(const any[] value, const any[] expected, int length, int &index=-1)
{
    for (int i = 0; i < length; ++i)
    {
        if (value[i] != expected[i])
        {
            index = i;
            return false;
        }
    }
    return true;
}

static bool __Array2DEq(const any[][] value, const any[][] expected, int rows, int cols, int &row=-1, int &col=-1)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (value[i][j] != expected[i][j])
            {
                row = i;
                col = j;
                return false;
            }
        }
    }
    return true;
}

static bool __FloatNearlyEq(const float value, const float expected)
{
    return FloatAbs(value - expected) < ASSERT_OPTION_FLOAT_THRESHOLD;
}

static bool __FloatArrayEq(const any[] value, const any[] expected, int length, int &index=-1)
{
    for (int i = 0; i < length; ++i)
    {
        if (!__FloatNearlyEq(value[i], expected[i]))
        {
            index = i;
            return false;
        }
    }
    return true;
}

static bool __FloatArray2DEq(const any[][] value, const any[][] expected, int rows, int cols, int &row=-1, int &col=-1)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (!__FloatNearlyEq(value[i][j], expected[i][j]))
            {
                row = i;
                col = j;
                return false;
            }
        }
    }
    return true;
}

static bool __StrStartsWith(const char[] value, const char[] expected)
{
    for (int i = 0; expected[i] != '\0'; ++i)
        if (value[i] != expected[i])
            return false;
    return true;
}

static bool __StrEndsWith(const char[] value, const char[] expected)
{
    int valueLength = strlen(value);
    int expectedLength = strlen(expected);
    if (valueLength < expectedLength)
        return false;
    return StrEqual(value[valueLength - expectedLength], expected);
}

static void __SplitPathByFile(char[] buffer1, int maxlen1, char[] buffer2, int maxlen2, const char[] path)
{
    int sepOffset = FindCharInString(path, '/', true);
    if (sepOffset == -1)
        sepOffset = FindCharInString(path, '\\', true);
    if (sepOffset == -1)
        return;

    if (maxlen1 > sepOffset + 2)
        strcopy(buffer1, sepOffset + 2, path);
    else
        strcopy(buffer1, maxlen1, path);
    strcopy(buffer2, maxlen2, path[sepOffset]);
}

static int __CountFiles(const char[] directory)
{
    DirectoryListing dir = OpenDirectory(directory);
    if (dir)
    {
        char filename[PLATFORM_MAX_PATH];
        FileType type;
        int counter = 0;
        while (dir.GetNext(filename, sizeof(filename), type))
        {
            if (StrEqual(filename, ".") || StrEqual(filename, ".."))
                continue;

            switch (type)
            {
                case FileType_Directory:
                {
                    Format(filename, sizeof(filename), "%s/%s", directory, filename);
                    counter += __CountFiles(filename);
                }
                case FileType_File:
                {
                    counter++;
                }
                default:
                {
                    counter++;
                }
            }
        }
        delete dir;
        return counter;
    }
    return 0;
}

static int __CountFileLines(const char[] filename)
{
    File file = OpenFile(filename, "r");
    if (!file)
        ThrowError("Failed open file \"%s\".", filename);

    char buffer[1024];
    int  lines = 0;
    bool incomplete = false;
    while (file.ReadLine(buffer, sizeof(buffer)))
    {
        int len = strlen(buffer);
        if (buffer[len - 1] == '\n')
        {
            lines++;
            incomplete = false;
        }
        else
        {
            if (!incomplete)
                incomplete = true;
        }
    }

    delete file;
    return incomplete ? lines + 1 : lines;
}
