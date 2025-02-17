#if defined _log_for_sourcepawn_test_utils_included
 #endinput
#endif
#define _log_for_sourcepawn_test_utils_included

#pragma newdecls required
#pragma semicolon 1

#define TEST_MAX_MSG_LENGTH      2048

#include <files>
#include <regex>
#include <testing>


stock char[] PrepareTestPath(const char[] fmt, any ...)
{
    char file[PLATFORM_MAX_PATH];
    VFormat(file, sizeof(file), fmt, 2);

    BuildTestPath(file, sizeof(file), file);
    __CleanTestDir(file);

    return file;
}

stock int BuildTestPath(char[] buffer, int maxlen, const char[] file)
{
    return BuildPath(Path_SM, buffer, maxlen, "logs/test_log4sp/%s", file);
}

stock int CountLines(const char[] path)
{
    File file = OpenFile(path, "r");
    // AssertTrue(path, file != null);

    char buffer[TEST_MAX_MSG_LENGTH];
    int counter = 0;
    while (!file.EndOfFile() && file.ReadLine(buffer, sizeof(buffer)))
    {
        counter++;
        // PrintToServer("[%d] %s", counter, buffer);
    }

    delete file;
    return counter;
}

stock int GetFileSize(const char[] path)
{
    File file = OpenFile(path, "r");
    // AssertTrue(path, file != null);

    int size = file.Size();
    delete file;
    return size;
}

stock bool StrEndWith(const char[] full, const char[] ending, bool caseSensitive = true)
{
    int fullLen = strlen(full);
    int endingLen = strlen(ending);
    return fullLen > endingLen && StrEqual(full[fullLen - endingLen], ending, caseSensitive);
}

stock int CountFiles(const char[] folder, any ...)
{
    char buffer[PLATFORM_MAX_PATH];
    VFormat(buffer, sizeof(buffer), folder, 2);

    int sep = FindCharInString(buffer, '\\', true);
    if (sep == -1)
        sep = FindCharInString(buffer, '/', true);
    if (sep == -1)
        sep = 0;

    buffer[sep] = '\0';

    // AssertTrue(buffer, DirExists(buffer));
    return __CountFiles(buffer);
}

// 整个文件
stock void AssertFileRegexEq(const char[] text, const char[] path, const char[] expectedPattern)
{
    File file = OpenFile(path, "r");
    AssertFileRegexEq2(text, file, expectedPattern);
    delete file;
}

// 从 file 指针位置起始
stock void AssertFileRegexEq2(const char[] text, File file, const char[] expectedPattern)
{
    // AssertTrue(text, file != null);
    char buffer[TEST_MAX_MSG_LENGTH * 10];
    file.ReadString(buffer, sizeof(buffer));

    AssertStrRegexEq(text, buffer, expectedPattern);
}

stock void AssertStrRegexEq(const char[] text, const char[] str, const char[] expectedPattern)
{
    AssertTrue(text, StrRegexEq(str, expectedPattern) > 0);
}

stock int StrRegexEq(const char[] str, const char[] expectedPattern)
{
    char err[128];
    Regex regex = new Regex(expectedPattern, _, err, sizeof(err));
    if (!regex)
    {
        LogMessage("Invalid string regex handle: %s", err);
        LogMessage("Str: '%s'", str);
        LogMessage("Exp: '%s'", expectedPattern);
        return -1;
    }

    RegexError code = REGEX_ERROR_NONE;
    int count = regex.MatchAll(str, code);
    delete regex;

    if (code != REGEX_ERROR_NONE)
    {
        LogMessage("String regex match failure: %d", code);
        LogMessage("Str: '%s'", str);
        LogMessage("Exp: '%s'", expectedPattern);
        return count;
    }

    if (count <= 0)
    {
        LogMessage("String regex match not found");
        LogMessage("Str: '%s'", str);
        LogMessage("Exp: '%s'", expectedPattern);
        return count;
    }

    return count;
}



static void __CleanTestDir(const char[] path)
{
    char buffer[PLATFORM_MAX_PATH];
    strcopy(buffer, sizeof(buffer), path);

    int sep = FindCharInString(buffer, '\\', true);
    if (sep == -1)
        sep = FindCharInString(buffer, '/', true);
    if (sep == -1)
        sep = 0;

    buffer[sep] = '\0';

    __CleanDir(buffer);
}

static void __CleanDir(const char[] path)
{
    DirectoryListing dir = OpenDirectory(path);
    if (dir)
    {
        char filename[PLATFORM_MAX_PATH];
        FileType type;

        while (dir.GetNext(filename, sizeof(filename), type))
        {
            if (StrEqual(filename, ".") || StrEqual(filename, ".."))
                continue;

            char fullpath[PLATFORM_MAX_PATH];
            FormatEx(fullpath, sizeof(fullpath), "%s/%s", path, filename);
            // PrintToServer("__CleanDir: | path %s | file %s |", path, filename);
            // PrintToServer("__CleanDir: | type %d | full %s |", type, fullpath);

            switch (type)
            {
                case FileType_Directory:
                {
                    __CleanDir(fullpath);
                }
                case FileType_File:
                {
                    if (!DeleteFile(fullpath))
                    {
                        char buffer[256];
                        FormatEx(buffer, sizeof(buffer), "Unable to delete '%s'", fullpath);
                        AssertTrue(buffer, false);
                    }
                }
                default:
                {
                    PrintToServer("Ignore unknown file type (%d) \"%s\"", type, fullpath);
                }
            }
        }
        delete dir;

        if (!RemoveDir(path))
        {
            char buffer[256];
            FormatEx(buffer, sizeof(buffer), "Unable to delete %s", path);
            AssertTrue(buffer, false);
        }
    }
}

static int __CountFiles(const char[] path)
{
    DirectoryListing dir = OpenDirectory(path);
    if (dir)
    {
        char filename[PLATFORM_MAX_PATH];
        char fullpath[PLATFORM_MAX_PATH];
        FileType type;
        int counter = 0;

        while (dir.GetNext(filename, sizeof(filename), type))
        {
            if (StrEqual(filename, ".") || StrEqual(filename, ".."))
                continue;

            FormatEx(fullpath, sizeof(fullpath), "%s/%s", path, filename);
            // PrintToServer("__CountFiles: | path %s | file %s |", path, filename);
            // PrintToServer("__CountFiles: | type %d | full %s |", type, fullpath);

            switch (type)
            {
                case FileType_Directory:
                {
                    counter += __CountFiles(fullpath);
                }
                case FileType_File:
                {
                    counter++;
                }
                default:
                {
                    PrintToServer("Ignore unknown file type (%d) \"%s\"", type, fullpath);
                }
            }
        }
        delete dir;
        return counter;
    }
    return 0;
}
