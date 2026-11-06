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
#include <log4sp/details/file_helper>

#include "../assert"
#include "../test_utils"


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_file_helper", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("---------- Started testing FileHelper ----------");

    PrepareTestPath("file_helper/");

    TestFilename();

    TestSize();

    TestReopen();

    TestReopenTruncate();

    TestSplitByExtension();

    TestCallbacks();

    TestOpenFail();

    PrintToServer("----------- Test FileHelper ended -------------");
}

void TestFilename()
{
    SetTestContext("FileHelper Filename");

    FileHelper helper = new FileHelper();

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "file_helper/filename.log");
    AssertTrue("[helper.Open]", helper.Open(filename, _, "", 0));

    char buffer[PLATFORM_MAX_PATH];
    int len = helper.GetFilename(buffer, sizeof(buffer));

    AssertStrEq("[helper.GetFilename]", buffer, filename);
    AssertEq("[helper.GetFilename length]", len, strlen(filename));

    helper.Close();
}

void TestSize()
{
    SetTestContext("FileHelper Size");

    FileHelper helper = new FileHelper();

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "file_helper/size.log");
    AssertTrue("[helper.Open]", helper.Open(filename, _, "", 0));

    char buffer[256];
    for (int i = 0; i < 123; ++i)
        buffer[i] = '1';
    buffer[123] = '\0';

    AssertTrue("[helper.Write]", helper.Write(buffer, "", 0));
    AssertTrue("[helper.Flush]", helper.Flush("", 0));
    AssertEq("[helper.Size]", helper.Size("", 0), 123);

    helper.Close();
}

void TestReopen()
{
    SetTestContext("FileHelper Reopen");

    FileHelper helper = new FileHelper();

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "file_helper/reopen.log");
    AssertTrue("[helper.Open]", helper.Open(filename, _, "", 0));

    AssertTrue("[helper.Write]", helper.Write("12", "", 0));
    AssertTrue("[helper.Flush]", helper.Flush("", 0));
    AssertEq("[helper.Size]", helper.Size("", 0), 2);

    // Reopen without truncation preserves the existing content.
    AssertTrue("[helper.Reopen]", helper.Reopen(false, "", 0));
    AssertEq("[helper.Size after Reopen]", helper.Size("", 0), 2);

    helper.Close();
}

void TestReopenTruncate()
{
    SetTestContext("FileHelper Reopen Truncate");

    FileHelper helper = new FileHelper();

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "file_helper/reopen_truncate.log");
    AssertTrue("[helper.Open]", helper.Open(filename, _, "", 0));

    AssertTrue("[helper.Write]", helper.Write("12", "", 0));
    AssertTrue("[helper.Flush]", helper.Flush("", 0));
    AssertEq("[helper.Size]", helper.Size("", 0), 2);

    // Reopen with truncation discards the existing content.
    AssertTrue("[helper.Reopen]", helper.Reopen(true, "", 0));
    AssertEq("[helper.Size after Reopen]", helper.Size("", 0), 0);

    helper.Close();
}

void TestSplitByExtension()
{
    SetTestContext("FileHelper SplitByExtension");

    // Ref spdlog::test_file_helper.cpp file_helper_split_by_extension.
    TestSplitExt("mylog.txt",                  "mylog",                  ".txt");
    TestSplitExt(".mylog.txt",                 ".mylog",                 ".txt");
    TestSplitExt(".mylog",                     ".mylog",                 "");
    TestSplitExt("/aaa/bb.d/mylog",            "/aaa/bb.d/mylog",        "");
    TestSplitExt("/aaa/bb.d/mylog.txt",        "/aaa/bb.d/mylog",        ".txt");
    TestSplitExt("aaa/bbb/ccc/mylog.txt",      "aaa/bbb/ccc/mylog",      ".txt");
    TestSplitExt("aaa/bbb/ccc/mylog.",         "aaa/bbb/ccc/mylog.",     "");
    TestSplitExt("aaa/bbb/ccc/.mylog.txt",     "aaa/bbb/ccc/.mylog",     ".txt");
    TestSplitExt("/aaa/bbb/ccc/mylog.txt",     "/aaa/bbb/ccc/mylog",     ".txt");
    TestSplitExt("/aaa/bbb/ccc/.mylog",        "/aaa/bbb/ccc/.mylog",    "");
    TestSplitExt("../mylog.txt",               "../mylog",               ".txt");
    TestSplitExt(".././mylog.txt",             ".././mylog",             ".txt");
    TestSplitExt(".././mylog.txt/xxx",         ".././mylog.txt/xxx",     "");
    TestSplitExt("/mylog.txt",                 "/mylog",                 ".txt");
    TestSplitExt("//mylog.txt",                "//mylog",                ".txt");
    TestSplitExt("",                           "",                       "");
    TestSplitExt(".",                          ".",                      "");
    TestSplitExt("..txt",                      ".",                      ".txt");
}

void TestSplitExt(const char[] path, const char[] expectedBase, const char[] expectedExt)
{
    char base[PLATFORM_MAX_PATH], ext[PLATFORM_MAX_PATH];
    FileHelper.SplitByExtension(base, sizeof(base), ext, sizeof(ext), path);

    char name[256];
    FormatEx(name, sizeof(name), "[split \"%s\"]", path);
    AssertStrEq(name, base, expectedBase);
    AssertStrEq(name, ext, expectedExt);
}

static int g_openCount;
static int g_closeCount;
static bool g_openFileExists;
static bool g_closeFileExists;
void TestCallbacks()
{
    SetTestContext("FileHelper Callbacks");

    g_openCount = 0;
    g_closeCount = 0;

    FileHelper helper = new FileHelper(null, CB_OnFileOpen, null, CB_OnFileClose);

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "file_helper/callback.log");
    AssertTrue("[helper.Open]", helper.Open(filename, _, "", 0));

    AssertEq("[open callback count]", g_openCount, 1);
    AssertFalse("[open callback before open]", g_openFileExists);

    helper.CloseFile();
    AssertEq("[close callback count]", g_closeCount, 1);
    AssertTrue("[close callback after close]", g_closeFileExists);

    // Re-open: the open callback fires again.
    AssertTrue("[helper.Reopen]", helper.Reopen(false, "", 0));
    AssertEq("[open callback count after Reopen]", g_openCount, 2);

    helper.Close();
    AssertEq("[close callback count after Close]", g_closeCount, 2);
}

void TestOpenFail()
{
    SetTestContext("FileHelper Open Fail");

    FileHelper helper = new FileHelper();

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "file_helper/open_fail.log");
    AssertTrue("[helper.Open]", helper.Open(filename, _, "", 0));

    helper.CloseFile();

    // The parent directory of this path is a file, so opening must fail.
    char invalid[PLATFORM_MAX_PATH];
    FormatEx(invalid, sizeof(invalid), "%s/invalid", filename);
    AssertFalse("[helper.Open invalid]", helper.Open(invalid, _, "", 0));

    helper.Close();
}


public void CB_OnFileOpen(const char[] filename)
{
    g_openCount++;
    g_openFileExists = FileExists(filename);
}

public void CB_OnFileClose(const char[] filename)
{
    g_closeCount++;
    g_closeFileExists = FileExists(filename);
}
