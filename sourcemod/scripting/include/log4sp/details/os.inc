#if defined _log4sp_details_os_included
    #endinput
#endif
#define _log4sp_details_os_included


/**
 * return file path and its extension:
 *
 *  "mylog.txt" => ("mylog", ".txt")
 *  "mylog" => ("mylog", "")
 *  "mylog." => ("mylog.", "")
 *  "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
 *
 *  the starting dot in filenames is ignored (hidden files):
 *
 *  ".mylog" => (".mylog". "")
 *  "my_folder/.mylog" => ("my_folder/.mylog", "")
 *  "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
 *
 * @note Ref spdlog::details::file_helper
 */
stock void SplitPathByExtension(char[] buffer1, int maxlen1, char[] buffer2, int maxlen2, const char[] path)
{
    int extOffset = FindCharInString(path, '.', true);

    // no valid extension found - return whole path and empty string as extension
    if (extOffset <= 0 || extOffset == strlen(path) - 1)
    {
        strcopy(buffer1, maxlen1, path);
        return;
    }

    // treat cases like "/etc/rc.d/somelogfile or "/abc/.hiddenfile"
    int sepOffset = FindCharInString(path, '/', true);
    if (sepOffset == -1)
        sepOffset = FindCharInString(path, '\\', true);
    if (sepOffset != -1 && sepOffset >= extOffset - 1)
    {
        strcopy(buffer1, maxlen1, path);
        return;
    }

    // finally - return a valid base and extension
    if (maxlen1 > extOffset + 1)
        strcopy(buffer1, extOffset + 1, path);
    else
        strcopy(buffer1, maxlen1, path);

    strcopy(buffer2, maxlen2, path[extOffset]);
}

/**
 * Return directory name from given path or empty string.
 *  "abc/file" => "abc"
 *  "abc/" => "abc"
 *  "abc" => ""
 *  "abc///" => "abc//"
 *
 * @note Ref spdlog::os::dir_name
 */
stock void SplitPathByFile(char[] buffer1, int maxlen1, char[] buffer2, int maxlen2, const char[] path)
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

/**
 * create the given directory - and all directories leading to it
 * return true on success or if the directory already exists.
 */
stock bool MakeDirs(const char[] directoryPath, int mode=FPERM_O_READ|FPERM_O_EXEC|FPERM_G_READ|FPERM_G_EXEC|FPERM_U_READ|FPERM_U_WRITE|FPERM_U_EXEC)
{
    if (!directoryPath[0])
        return false;

    if (DirExists(directoryPath))
        return true;

    int searchOffset = 0;
    int endOffset = strlen(directoryPath) - 1;
    char subDir[PLATFORM_MAX_PATH];

    do {
        int sepOffset = FindCharInString(directoryPath[searchOffset], '/');
        if (sepOffset == -1)
            sepOffset = FindCharInString(directoryPath[searchOffset], '\\');

        // treat the entire path as a folder if no folder separator not found
        if (sepOffset == -1)
            sepOffset = endOffset;

        searchOffset += (sepOffset + 1);
        strcopy(subDir, searchOffset, directoryPath);

        if (subDir[0] && !DirExists(subDir) && !CreateDirectory(subDir, mode))
            return false;  // return error if failed creating dir
    } while(searchOffset < endOffset);
    return true;
}

stock void SafeSleepForMilliseconds(int milliseconds)
{
    // https://github.com/actions/runner/blob/c96dcd472907e514274cdb4af281116adf7aad18/src/Misc/layoutroot/safe_sleep.sh#L33
    int endTickCount = GetSysTickCount() + milliseconds;
    // PrintToServer("Start | sys tick = %u | time %u | engine time = %f |", GetSysTickCount(), GetTime(), GetEngineTime());
    while (GetSysTickCount() < endTickCount) {}
    // PrintToServer("End   | sys tick = %u | time %u | engine time = %f |", GetSysTickCount(), GetTime(), GetEngineTime());
}
