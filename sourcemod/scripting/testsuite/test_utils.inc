#if defined _log4sp_test_utils_included
 #endinput
#endif
#define _log4sp_test_utils_included

#include <log4sp/details/os>


stock int BuildTestPath(char[] buffer, int maxlen, const char[] filename)
{
    return BuildPath(Path_SM, buffer, maxlen, "logs/test_log4sp/%s", filename);
}

stock char[] PrepareTestPath(const char[] fmt, any ...)
{
    char filename[PLATFORM_MAX_PATH];
    VFormat(filename, sizeof(filename), fmt, 2);

    BuildTestPath(filename, sizeof(filename), filename);

    char directory[PLATFORM_MAX_PATH];
    SplitPathByFile(directory, sizeof(directory), "", 0, filename);

    char error[256];
    if (!__RemoveDirs(directory, error, sizeof(error)))
        ThrowError("%s", error);

    if (!MakeDirs(directory))
        ThrowError("Failed to make directory \"%s\".", directory);
    return filename;
}

stock void GetSMErrorFilename(char[] buffer, int maxlen)
{
    FormatTime(buffer, maxlen, "addons/sourcemod/logs/errors_%Y%m%d.log");
}

stock int FetchSourceModVersionFromCommand(int &major=0, int &minor=0, int &patch=0, int &build=0)
{
    // https://github.com/alliedmodders/sourcemod/blob/master/core/logic/RootConsoleMenu.cpp#L235
    //  SourceMod Version Information:
    //     SourceMod Version: [0-9]+\.[0-9]+\.[0-9]+\.[0-9]+
    char buffer[1024];  // FIXME: Results may exceed the capacity.
    ServerCommandEx(buffer, sizeof(buffer), "%s", "sm version");

    // Using regular expressions might be better.
    int offset = 55;
    offset += StringToIntEx(buffer[offset], major) + 1;
    offset += StringToIntEx(buffer[offset], minor) + 1;
    offset += StringToIntEx(buffer[offset], patch) + 1;
    StringToIntEx(buffer[offset], build);
    return (major << 24) | (minor << 16) | (patch << 8) | build;
}

stock int FetchHandleCountFromCommand(const char[] handleOwner=NULL_STRING, const char[] dumpFilename=NULL_STRING)
{
    // https://github.com/alliedmodders/sourcemod/blob/e87291362bc8445b187bb2a574ad702e8f1a0696/core/logic/HandleSys.cpp#L1246
    char owner[21];
    if (!handleOwner[0] || IsNullString(handleOwner))
        GetPluginFilename(null, owner, sizeof(owner));
    else
        strcopy(owner, sizeof(owner), handleOwner);
    Format(owner, sizeof(owner), "%-20.20s", owner);
    // PrintToServer("handleOwner: \"%s\" owner: \"%s\".", handleOwner, owner);

    char filename[PLATFORM_MAX_PATH];
    if (!dumpFilename[0] || IsNullString(dumpFilename))
    {
        char date[256];
        FormatTime(date, sizeof(date), "%Y%m%d_%H%M%S");
        BuildPath(Path_SM, filename, sizeof(filename), "logs/dump_%s.log", date);
    }
    else
    {
        strcopy(filename, sizeof(filename), dumpFilename);
    }
    // PrintToServer("Dump File: \"%s\" Exists: %s.", filename, FileExists(filename) ? "true" : "false");

    ServerCommand("sm_dump_handles %s", filename);
    ServerExecute();

    File file = OpenFile(filename, "r");
    if (!file)
        ThrowError("Failed open file \"%s\".", filename);

    int result = 0;
    char line[128];
    while (file.ReadLine(line, sizeof(line))) {
        if (strncmp(line, "0x", 2) != 0)
            continue;

        // line[strlen(line) - 1] = '\0';
        // PrintToServer("owner: %s %s line: %s.",
        //               owner,
        //               strncmp(line[11], owner, sizeof(owner) - 1) != 0 ? "!=" : "==",
        //               line[11]);

        if (strncmp(line[11], owner, sizeof(owner) - 1) != 0)
            continue;

        ++result;
    }
    delete file;
    return result;
}




static bool __RemoveDirs(const char[] directoryPath, char[] error="", int maxlen=0)
{
    if (!directoryPath[0] || !DirExists(directoryPath))
        return true;

    DirectoryListing directory = OpenDirectory(directoryPath);
    if (!directory)
    {
        FormatEx(error, maxlen, "Failed to open directory \"%s\".", directoryPath);
        return false;
    }

    char filename[PLATFORM_MAX_PATH];
    FileType type;
    while (directory.GetNext(filename, sizeof(filename), type))
    {
        if (StrEqual(filename, ".") || StrEqual(filename, ".."))
            continue;

        char path[PLATFORM_MAX_PATH];
        FormatEx(path, sizeof(path), "%s/%s", directoryPath, filename);

        switch (type)
        {
            case FileType_Directory:
            {
                if (!__RemoveDirs(path, error, maxlen))
                    return false;
            }
            case FileType_File:
            {
                if (!DeleteFile(path))
                {
                    FormatEx(error, maxlen, "Failed to delete file \"%s\".", path);
                    return false;
                }
            }
            default:
            {
                if (!DeleteFile(path))
                {
                    FormatEx(error, maxlen, "Failed to delete \"%s\".", path);
                    return false;
                }
            }
        }
    }
    delete directory;

    if (!RemoveDir(directoryPath))
    {
        FormatEx(error, maxlen, "Failed to delete directory \"%s\".", directoryPath);
        return false;
    }
    return true;
}
