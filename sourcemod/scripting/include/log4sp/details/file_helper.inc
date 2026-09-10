#if defined _log4sp_details_file_helper_included
    #endinput
#endif
#define _log4sp_details_file_helper_included

#include <log4sp/common>
#include <log4sp/details/os>


methodmap FileHelper < Handle
{
    /**
     * Helper methodmap for file sinks.
     * When failing to open a file, retry several times(5) with a delay interval(10 ms).
     * return false and error description on errors.
     *
     * @note FileHelper handles must be freed via FileHelper.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @param fileOpenPlugin    Handle of the plugin that contains the file open function.
     *                          If null, defaults to the calling plugin.
     * @param fileOpenFunc      Function to call before the sink open the file.
     * @param fileClosePlugin   Handle of the plugin that contains the file close function.
     *                          If null, defaults to the calling plugin.
     * @param fileCloseFunc     Function to call after the sink close the file.
     * @return                  A new FileHelper Handle.
     */
    public FileHelper(Handle fileOpenPlugin=null,  FileOpenFunction fileOpenFunc=INVALID_FUNCTION,
                      Handle fileClosePlugin=null, FileCloseFunction fileCloseFunc=INVALID_FUNCTION) {
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        PrivateForward fileOpenFwd = null;
        if (fileOpenFunc && fileOpenFunc != INVALID_FUNCTION) {
            // void (const char[] filename);
            fileOpenFwd = new PrivateForward(ET_Ignore, Param_String);
            fileOpenFwd.AddFunction(fileOpenPlugin, fileOpenFunc);
        }
        PrivateForward fileCloseFwd = null;
        if (fileCloseFunc && fileCloseFunc != INVALID_FUNCTION) {
            // void (const char[] filename);
            fileCloseFwd = new PrivateForward(ET_Ignore, Param_String);
            fileCloseFwd.AddFunction(fileClosePlugin, fileCloseFunc);
        }

        FileHelper fileHelper = view_as<FileHelper>(new StringMap());
        fileHelper.__SetRefCnt(1);
        fileHelper.__SetOwner(owner);
        fileHelper.__SetFile(null);
        fileHelper.__SetFileOwner(NULL_STRING);
        fileHelper.__SetFilename(NULL_STRING);
        fileHelper.__SetFileOpenFwd(fileOpenFwd);
        fileHelper.__SetFileCloseFwd(fileCloseFwd);
        return fileHelper;
    }

    /**
     * Opens or creates a binary file for appending.
     *
     * @note The file is closed when the fileHelper handle is closed or FileHelper.CloseFile() is called.
     *
     * @param filename          Filename to open.
     * @param truncate          If true, the file will be truncated.
     * @param error             Buffer to store error description.
     * @param maxlen            Maximum length of the error.
     * @return                  True on success, false on error.
     */
    public bool Open(const char[] filename, bool truncate=false, char[] error, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        const int OPEN_TRIES = 5;
        const int OPEN_INTERVAL = 10;

        this.CloseFile(); // ;-)
        this.__SetFilename(filename);
        this.__CallFileOpenFunction();

        char directory[PLATFORM_MAX_PATH];
        SplitPathByFile(directory, sizeof(directory), "", 0, filename);

        for (int tries = 0; tries < OPEN_TRIES; ++tries) {
            // create containing folder if not exists already.
            MakeDirs(directory);

            if (truncate) {
                // Truncate by opening-and-closing a tmp file in "wb" mode, always opening the
                // actual log-we-write-to in "ab" mode, since that interacts more politely with
                // eternal processes that might rotate/truncate the file underneath us.
                File tmp = OpenFile(filename, "wb");
                if (!tmp)
                    continue;
                delete tmp;
            }

            File file = OpenFile(filename, "ab");
            if (file) {
                char fileOwner[PLATFORM_MAX_PATH];
                GetPluginFilename(null, fileOwner, sizeof(fileOwner));

                this.__SetFile(file);
                this.__SetFileOwner(fileOwner);
                return true;
            }
            SafeSleepForMilliseconds(OPEN_INTERVAL);
        }

        FormatEx(error, maxlen, "Failed opening file \"%s\" for writing.", filename);
        return false;
    }

    /**
     * Reopens a binary file for appending.
     *
     * @param truncate          If true, the file will be truncated.
     * @param error             Buffer to store error description.
     * @param maxlen            Maximum length of the error.
     * @return                  True on success, false on error.
     */
    public bool Reopen(bool truncate, char[] error, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        char filename[PLATFORM_MAX_PATH];
        if (!this.__GetFilename(filename, sizeof(filename))) {
            strcopy(error, maxlen, "Failed re opening file - was not opened before.");
            return false;
        }
        return this.Open(filename, truncate, error, maxlen);
    }

    /**
     * Manual flush file contents.
     *
     * @param error             Buffer to store error description.
     * @param maxlen            Maximum length of the error.
     * @return                  True on success, false on error.
     */
    public bool Flush(char[] error, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        File file = this.__GetFile();
        if (!file || !file.Flush()) {
            char filename[PLATFORM_MAX_PATH];
            this.__GetFilename(filename, sizeof(filename));
            FormatEx(error, maxlen, "Failed flush to file \"%s\".", filename);
            return false;
        }
        return true;
    }

    /**
     * Close the file.
     */
    public void CloseFile() {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        File file = this.__GetFile();
        if (file) {
            char fileOwner[PLATFORM_MAX_PATH];
            this.__GetFileOwner(fileOwner, sizeof(fileOwner));
            Handle plugin = FindPluginByFile(fileOwner);
            Function func = GetFunctionByName(plugin, "__FileHelper_OnCloseFile");

            // void (FileHelper fileHelper);
            Call_StartFunction(plugin, func);
            Call_PushCell(this);
            Call_Finish();

            this.__SetFile(null);
            this.__SetFileOwner(NULL_STRING);
            this.__CallFileCloseFunction();
        }
    }

    /**
     * Writes a binary string to a file.
     *
     * @param buffer            String to write.
     * @param error             Buffer to store error description.
     * @param maxlen            Maximum length of the error.
     * @return                  True on success, false on error.
     */
    public bool Write(const char[] buffer, char[] error, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        File file = this.__GetFile();
        if (!file || !file.WriteString(buffer, false)) {
            char filename[PLATFORM_MAX_PATH];
            this.__GetFilename(filename, sizeof(filename));
            FormatEx(error, maxlen, "Failed writing to file \"%s\".", filename);
            return false;
        }
        return true;
    }

    /**
     * Get the file size in bytes.
     *
     * @param error             Buffer to store error description.
     * @param maxlen            Maximum length of the error.
     * @return                  File size in bytes, -1 if failed.
     */
    public int Size(char[] error, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        char filename[PLATFORM_MAX_PATH];
        this.__GetFilename(filename, sizeof(filename));
        int size = FileSize(filename);
        if (size == -1) {
            FormatEx(error, maxlen, "Cannot use Size() on closed file \"%s\".", filename);
        }
#else
        int size = this.__GetFile().Size();
        if (size == -1) {
            char filename[PLATFORM_MAX_PATH];
            this.__GetFilename(filename, sizeof(filename));
            FormatEx(error, maxlen, "Cannot use Size() on closed file \"%s\".", filename);
        }
#endif      // SOURCEMOD_V_*
        return size;
    }

    /**
     * Get the current filename being used by the file.
     *
     * @param buffer            Buffer to store file name.
     * @param maxlen            Maximum length of the buffer.
     * @return                  Number of bytes written.
     */
    public int GetFilename(char[] buffer, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);
        return this.__GetFilename(buffer, maxlen);
    }

    public FileHelper Clone() {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        // Increment the reference count
        int refCnt = this.__GetRefCnt() + 1;
        this.__SetRefCnt(refCnt);
        FileHelper cloned = view_as<FileHelper>(CloneHandle(this));
        return cloned;
    }

    public void Close() {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);

        // Decrement the reference count
        int refCnt = this.__GetRefCnt() - 1;
        this.__SetRefCnt(refCnt);

        // Close if no references
        if (refCnt <= 0) {
            // Close & Flush
            this.CloseFile();

            // Cleanup
            char owner[PLATFORM_MAX_PATH];
            this.__GetOwner(owner, sizeof(owner));
            Handle plugin = FindPluginByFile(owner);
            Function func = GetFunctionByName(plugin, "__FileHelper_OnClose");

            // void (FileHelper fileHelper);
            Call_StartFunction(plugin, func);
            Call_PushCell(this);
            Call_Finish();

            // It will be destroyed soon, so ignore resetting it to null.
            // this.__SetFileOpenFwd(null);
            // this.__SetFileCloseFwd(null);
        }
        delete view_as<StringMap>(this);
    }

    public bool Equals(FileHelper fileHelper) {
        if (!this.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", this);
        if (!fileHelper.IsValid())
            ThrowError("Invalid FileHelper Handle %x.", fileHelper);
        if (this == fileHelper)
            return true;

        /**
         * Compare whether each property is the same.
         * The handle with the same ownership lifecycle as this
         * should be different for each FileHelper.
         */
        if (this.__GetRefCnt() != fileHelper.__GetRefCnt()
         || this.__GetFile() != fileHelper.__GetFile()
         || this.__GetFileOpenFwd() != fileHelper.__GetFileOpenFwd()
         || this.__GetFileCloseFwd() != fileHelper.__GetFileCloseFwd())
            return false;

        char buffer[2][PLATFORM_MAX_PATH];
        int len1 = this.__GetOwner(buffer[0], sizeof(buffer[]));
        int len2 = fileHelper.__GetOwner(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;

        len1 = this.__GetFileOwner(buffer[0], sizeof(buffer[]));
        len2 = fileHelper.__GetFileOwner(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;

        len1 = this.__GetFilename(buffer[0], sizeof(buffer[]));
        len2 = fileHelper.__GetFilename(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;
        return true;
    }

    public bool IsValid() {
#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        return this
         && this.__Contains("m_cRef")       && this.__Contains("m_szOwner")
         && this.__Contains("m_hFile")      && this.__Contains("m_szFileOwner")
         && this.__Contains("m_szFilename") && this.__Contains("m_hFileOpenFwd")
         && this.__Contains("m_hFileCloseFwd");
#else
        return !!this;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
    }

    public static void SplitByExtension(char[] buffer1, int maxlen1, char[] buffer2, int maxlen2, const char[] path) {
        SplitPathByExtension(buffer1, maxlen1, buffer2, maxlen2, path);
    }


    // *** Do not use the function below this line! *** //
    public bool __Contains(const char[] key) {
        return view_as<StringMap>(this).ContainsKey(key);
    }

    public any __GetValue(const char[] key) {
        any value;
        view_as<StringMap>(this).GetValue(key, value);
        return value;
    }
    public void __SetValue(const char[] key, any value) {
        view_as<StringMap>(this).SetValue(key, value);
    }

    public int __GetString(const char[] key, char[] value, int maxlen) {
        int size;
        view_as<StringMap>(this).GetString(key, value, maxlen, size);
        return size;
    }
    public void __SetString(const char[] key, const char[] value) {
        view_as<StringMap>(this).SetString(key, value);
    }

    public any __GetRefCnt()                                { return this.__GetValue("m_cRef"); }
    public void __SetRefCnt(int refCnt)                     { this.__SetValue("m_cRef", refCnt); }

    public int __GetOwner(char[] owner, int maxlen)         { return this.__GetString("m_szOwner", owner, maxlen); }
    public void __SetOwner(const char[] owner)              { this.__SetString("m_szOwner", owner); }

    // Ownership and lifecycle are the same as this, closed in __FileHelper_OnClose.
    public File __GetFile()                                 { return this.__GetValue("m_hFile"); }
    public void __SetFile(File file)                        { this.__SetValue("m_hFile", file); }

    public int __GetFileOwner(char[] fileOwner, int maxlen) { return this.__GetString("m_szFileOwner", fileOwner, maxlen); }
    public void __SetFileOwner(const char[] fileOwner)      { this.__SetString("m_szFileOwner", fileOwner); }

    public int __GetFilename(char[] filename, int maxlen)   { return this.__GetString("m_szFilename", filename, maxlen); }
    public void __SetFilename(const char[] filename)        { this.__SetString("m_szFilename", filename); }

    // Ownership and lifecycle are the same as this, closed in __FileHelper_OnClose.
    public PrivateForward __GetFileOpenFwd()                { return this.__GetValue("m_hFileOpenFwd"); }
    public void __SetFileOpenFwd(PrivateForward fileOpenFwd){ this.__SetValue("m_hFileOpenFwd", fileOpenFwd); }

    // Ownership and lifecycle are the same as this, closed in __FileHelper_OnClose.
    public PrivateForward __GetFileCloseFwd()               { return this.__GetValue("m_hFileCloseFwd"); }
    public void __SetFileCloseFwd(PrivateForward fileCloseFwd){ this.__SetValue("m_hFileCloseFwd", fileCloseFwd); }

    public void __CallFileOpenFunction() {
        PrivateForward fileOpenFwd = this.__GetFileOpenFwd();
        if (fileOpenFwd) {
            char filename[PLATFORM_MAX_PATH];
            this.__GetFilename(filename, sizeof(filename));
            Call_StartForward(fileOpenFwd);
            Call_PushString(filename);
            Call_Finish();
        }
    }

    public void __CallFileCloseFunction() {
        PrivateForward fileCloseFwd = this.__GetFileCloseFwd();
        if (fileCloseFwd) {
            char filename[PLATFORM_MAX_PATH];
            this.__GetFilename(filename, sizeof(filename));
            Call_StartForward(fileCloseFwd);
            Call_PushString(filename);
            Call_Finish();
        }
    }
}


public void __FileHelper_OnCloseFile(FileHelper fileHelper)
{
    // It will be destroyed soon, so ignore resetting it to null.
    // Close & Flush
    // https://github.com/alliedmodders/sourcemod/blob/5acd8c77c99da0b0c381fb8b3449ad9e0521752e/core/logic/smn_filesystem.cpp#L254
    delete fileHelper.__GetFile();
}

public void __FileHelper_OnClose(FileHelper fileHelper)
{
    if (fileHelper.__GetFileOpenFwd())
        delete fileHelper.__GetFileOpenFwd();

    if (fileHelper.__GetFileCloseFwd())
        delete fileHelper.__GetFileCloseFwd();
}
