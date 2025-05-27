#if defined _log4sp_test_utils_included
 #endinput
#endif
#define _log4sp_test_utils_included

#pragma newdecls required
#pragma semicolon 1

#define TEST_LOG4SP_LEVEL_TOTAL  view_as<int>(LogLevel_Total)
// Pattern
#define P_FOLDER_SEPS           "(/|\\)"
#define P_EOL                   "(\n|\r\n)"
#define P_SM_DATE               "L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}"
#define P_SM_PREFIX             P_SM_DATE ... ": \\[SM\\] "
#define P_DATE                  "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}\\]"
#define P_PREFIX                P_DATE ... " \\[test-.*\\] \\[(trace|debug|info|warn|error|fatal|off)\\] "

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
    if (!file)
        ThrowError("Failed open file: (%s).", file);

    char buffer[2048];
    int counter = 0;
    while (!file.EndOfFile() && file.ReadLine(buffer, sizeof(buffer)))
    {
        counter++;
        // PrintToServer("[%d] %s", counter, buffer);
    }

    delete file;
    return counter;
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

    if (!DirExists(buffer))
        ThrowError("Invalid path: (%s).", buffer);
    return __CountFiles(buffer);
}

stock void AssertFileMatch(const char[] text, const char[] path, const char[] expectedPattern)
{
    AssertTrue(text, StringMatch(GetFileContents(path), expectedPattern) > 0);
}

stock void AssertStrMatch(const char[] text, const char[] str, const char[] expectedPattern)
{
    AssertTrue(text, StringMatch(str, expectedPattern) > 0);
}

stock int StringMatch(const char[] str, const char[] expectedPattern)
{
    char err[128];
    Regex regex = new Regex(expectedPattern, _, err, sizeof(err));
    if (!regex)
    {
        LogError("Str: (%s)", str);
        LogError("Exp: (%s)", expectedPattern);
        ThrowError("Invalid regex handle: (%s).", err);
    }

    RegexError code = REGEX_ERROR_NONE;
    int count = regex.MatchAll(str, code);
    delete regex;

    if (code != REGEX_ERROR_NONE)
    {
        LogError("Str: (%s)", str);
        LogError("Exp: (%s)", expectedPattern);
        ThrowError("String match failure: (%d).", code);
    }

    if (count <= 0)
    {
        Logger.Get(LOG4SP_GLOBAL_LOGGER_NAME).WarnEx("Str: (%s)", str);
        Logger.Get(LOG4SP_GLOBAL_LOGGER_NAME).WarnEx("Exp: (%s)", expectedPattern);
        Logger.Get(LOG4SP_GLOBAL_LOGGER_NAME).WarnEx("StringMatch: No match found!");
        return count;
    }

    return count;
}

stock char[] GetErrorFilename()
{
    char filename[PLATFORM_MAX_PATH];
    FormatTime(filename, sizeof(filename), "addons/sourcemod/logs/errors_%Y%m%d.log");
    return filename;
}

stock void BackupSMErrorFile()
{
    if (!FileExists(GetErrorFilename()))
        return;

    char prefix[PLATFORM_MAX_PATH];
    FormatTime(prefix, sizeof(prefix), "addons/sourcemod/logs/errors_%Y%m%d");

    char newpath[PLATFORM_MAX_PATH];
    FormatEx(newpath, sizeof(newpath), "%s-backup.1.log", prefix);

    // 查找未被占用的文件名
    for (int i = 2; FileExists(newpath); ++i)
        FormatEx(newpath, sizeof(newpath), "%s-backup.%d.log", prefix, i);

    RenameFile(newpath, GetErrorFilename());
}

stock void MarkErrorTestStart(const char[] fmt, any ...)
{
    char buffer[2048];
    VFormat(buffer, sizeof(buffer), fmt, 2);

    BackupSMErrorFile(); // 保留原先的错误信息以便排查

    FindConVar("sv_logecho").SetBool(false); // HACK: 暂时关闭控制台回显日志消息
    LogError("======= Start - %s =======", buffer);
}

stock void MarkErrorTestEnd(const char[] fmt, any ...)
{
    char buffer[2048];
    VFormat(buffer, sizeof(buffer), fmt, 2);

    LogError("======= End - %s =======", buffer);
    FindConVar("sv_logecho").SetBool(true); // HACK: 重新启用控制台回显日志消息
}

stock char[] GetFileContents(const char[] path)
{
    File file = OpenFile(path, "rt");
    if (!file)
        ThrowError("Failed open file '%s'.", file);

    char buffer[2048 * 2];
    file.ReadString(buffer, sizeof(buffer));
    delete file;
    return buffer;
}



// 内部封装
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
                        ThrowError("Unable to delete: (%s).", fullpath);
                }
                default:
                {
                    LogError("Ignore unknown file type: (%d) (%s).", type, fullpath);
                }
            }
        }
        delete dir;

        if (!RemoveDir(path))
            ThrowError("Unable to delete: (%s).", path);
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
                    ThrowError("Unknown file type: (%d) (%s).", type, fullpath);
                }
            }
        }
        delete dir;
        return counter;
    }
    return 0;
}
