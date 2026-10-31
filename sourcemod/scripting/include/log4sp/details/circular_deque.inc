#if defined _log4sp_details_circular_deque_included
    #endinput
#endif
#define _log4sp_details_circular_deque_included


methodmap CircularDeque < Handle
{
    public CircularDeque(int maxSize, int blocksize=1) {
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        // blocksize 和 maxSize 在 CreateArray 内被转为 size_t
        // blocksize 为 0 时会抛出错误
        // maxSize 为 0 时正常，但无法存储任何数据
        // Header Only 内部已将二者视为无符号类型，负数是可接受的
        ArrayList buffer = new ArrayList(blocksize, maxSize);

        CircularDeque circularDeque = view_as<CircularDeque>(new StringMap());
        circularDeque.__SetRefCnt(1);
        circularDeque.__SetOwner(owner);
        circularDeque.__SetMaxSize(maxSize);
        circularDeque.__SetLength(0);
        circularDeque.__SetHead(0);
        circularDeque.__SetBuffer(buffer);
        return circularDeque;
    }

    public any GetFront(int block=0, bool asChar=false) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.__IsEmpty())
            ThrowError("CircularDeque is empty.");

        int index = this.__IndexAt(0);
        return this.__GetBuffer().Get(index, block, asChar);
    }

    public int GetFrontArray(any[] buffer, int maxlen=-1, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.__IsEmpty())
            ThrowError("CircularDeque is empty.");

        int index = this.__IndexAt(0);
#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        return this.__GetBuffer().GetArray(index, buffer, maxlen);
#else
        return this.__GetBuffer().GetArray(index, buffer, maxlen, block);
#endif
    }

    public int GetFrontString(char[] buffer, int maxlen, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.__IsEmpty())
            ThrowError("CircularDeque is empty.");

        int index = this.__IndexAt(0);
#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        return this.__GetBuffer().GetString(index, buffer, maxlen);
#else
        return this.__GetBuffer().GetString(index, buffer, maxlen, block);
#endif
    }

    public any GetBack(int block=0, bool asChar=false) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.__IsEmpty())
            ThrowError("CircularDeque is empty.");

        int length = this.__GetLength();
        int index = this.__IndexAt(length - 1);
        return this.__GetBuffer().Get(index, block, asChar);
    }

    public int GetBackArray(any[] buffer, int maxlen=-1, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.__IsEmpty())
            ThrowError("CircularDeque is empty.");

        int length = this.__GetLength();
        int index = this.__IndexAt(length - 1);
#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        return this.__GetBuffer().GetArray(index, buffer, maxlen);
#else
        return this.__GetBuffer().GetArray(index, buffer, maxlen, block);
#endif
    }

    public int GetBackString(char[] buffer, int maxlen, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.__IsEmpty())
            ThrowError("CircularDeque is empty.");

        int length = this.__GetLength();
        int index = this.__IndexAt(length - 1);
#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        return this.__GetBuffer().GetString(index, buffer, maxlen);
#else
        return this.__GetBuffer().GetString(index, buffer, maxlen, block);
#endif
    }

    public void PushFront(any value, int block=0, bool asChar=false) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        int maxSize = this.__GetMaxSize();
        if (maxSize == 0)
            return;

        int head = this.__GetHead();
        int index = this.__PreviousIndex(head);

        this.__GetBuffer().Set(index, value, block, asChar);
        this.__SetHead(index);

        if (!this.__IsFull())
            this.__SetLength(this.__GetLength() + 1);
    }

    public void PushFrontArray(const any[] values, int maxlen=-1, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        int maxSize = this.__GetMaxSize();
        if (maxSize == 0)
            return;

        int head = this.__GetHead();
        int index = this.__PreviousIndex(head);

#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        this.__GetBuffer().SetArray(index, values, maxlen);
#else
        this.__GetBuffer().SetArray(index, values, maxlen, block);
#endif
        this.__SetHead(index);

        if (!this.__IsFull())
            this.__SetLength(this.__GetLength() + 1);
    }

    public void PushFrontString(const char[] value, int maxlen=-1, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        int maxSize = this.__GetMaxSize();
        if (maxSize == 0)
            return;

        int head = this.__GetHead();
        int index = this.__PreviousIndex(head);

#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
        this.__GetBuffer().SetString(index, value);
#else
        this.__GetBuffer().SetString(index, value, maxlen, block);
#endif
        this.__SetHead(index);

        if (!this.__IsFull())
            this.__SetLength(this.__GetLength() + 1);
    }

    public void PushBack(any value, int block=0, bool asChar=false) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        int maxSize = this.__GetMaxSize();
        if (maxSize == 0)
            return;

        if (!this.__IsFull()) {
            int size = this.__GetLength();
            int index = this.__IndexAt(size);

            this.__GetBuffer().Set(index, value, block, asChar);
            this.__SetLength(size + 1);
        } else {
            int index = this.__GetHead();
            this.__GetBuffer().Set(index, value, block, asChar);
            this.__SetHead(this.__NextIndex(index));
        }
    }

    public void PushBackArray(const any[] values, int maxlen=-1, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        int maxSize = this.__GetMaxSize();
        if (maxSize == 0)
            return;

        if (!this.__IsFull()) {
            int length = this.__GetLength();
            int index = this.__IndexAt(length);

#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
            this.__GetBuffer().SetArray(index, values, maxlen);
#else
            this.__GetBuffer().SetArray(index, values, maxlen, block);
#endif
            this.__SetLength(length + 1);
        } else {
            int index = this.__GetHead();
#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
            this.__GetBuffer().SetArray(index, values, maxlen);
#else
            this.__GetBuffer().SetArray(index, values, maxlen, block);
#endif
            this.__SetHead(this.__NextIndex(index));
        }
    }

    public void PushBackString(const char[] value, int maxlen=-1, int block=0) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        int maxSize = this.__GetMaxSize();
        if (maxSize == 0)
            return;

        if (!this.__IsFull()) {
            int length = this.__GetLength();
            int index = this.__IndexAt(length);

#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
            this.__GetBuffer().SetString(index, value);
#else
            this.__GetBuffer().SetString(index, value, maxlen, block);
#endif
            this.__SetLength(length + 1);
        } else {
            int index = this.__GetHead();
#if SOURCEMOD_V_MAJOR == 1 && SOURCEMOD_V_MINOR <= 11
            this.__GetBuffer().SetString(index, value);
#else
            this.__GetBuffer().SetString(index, value, maxlen, block);
#endif
            this.__SetHead(this.__NextIndex(index));
        }
    }

    public void PopFront() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.IsEmpty())
            return;

        int head = this.__GetHead();
        int length = this.__GetLength();

        this.__SetHead(this.__NextIndex(head));
        this.__SetLength(length - 1);
    }

    public void PopBack() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        if (this.IsEmpty())
            return;

        int length = this.__GetLength();
        this.__SetLength(length - 1);
    }

    public int GetMaxSize() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        return this.__GetMaxSize();
    }

    public bool IsEmpty() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        return this.__IsEmpty();
    }

    public bool IsFull() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        return this.__IsFull();
    }

    public CircularDeque Clone() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        // Increment the reference count
        int refCnt = this.__GetRefCnt() + 1;
        this.__SetRefCnt(refCnt);
        CircularDeque cloned = view_as<CircularDeque>(CloneHandle(this));
        return cloned;
    }

    public void Close() {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);

        // Decrement the reference count
        int refCnt = this.__GetRefCnt() - 1;
        this.__SetRefCnt(refCnt);

        // Close if no references
        if (refCnt <= 0) {
            // Cleanup
            char owner[PLATFORM_MAX_PATH];
            this.__GetOwner(owner, sizeof(owner));
            Handle plugin = FindPluginByFile(owner);
            Function func = GetFunctionByName(plugin, "__CircularDeque_OnClose");

            // void (CircularDeque circularDeque);
            Call_StartFunction(plugin, func);
            Call_PushCell(this);
            Call_Finish();

            // It will be destroyed soon, so ignore resetting it to null.
            // this.__SetBuffer(null);
        }
        delete view_as<StringMap>(this);
    }

    public bool Equals(CircularDeque circularDeque) {
        if (!this.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", this);
        if (!circularDeque.IsValid())
            ThrowError("Invalid CircularDeque Handle %x.", circularDeque);
        if (this == circularDeque)
            return true;

        /**
         * Compare whether each property is the same.
         * The handle with the same ownership lifecycle as this
         * should be different for each CircularDeque.
         */
        if (this.__GetRefCnt() != circularDeque.__GetRefCnt()
         || this.__GetMaxSize() != circularDeque.__GetMaxSize()
         || this.__GetLength() != circularDeque.__GetLength()
         || this.__GetHead() != circularDeque.__GetHead()
         || this.__GetBuffer() != circularDeque.__GetBuffer())
            return false;

        char buffer[2][PLATFORM_MAX_PATH];
        int len1 = this.__GetOwner(buffer[0], sizeof(buffer[]));
        int len2 = circularDeque.__GetOwner(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;
        return true;
    }

    public bool IsValid() {
#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        return view_as<bool>(this)
         && this.__Contains("m_cRef")       && this.__Contains("m_szOwner")
         && this.__Contains("m_uMaxSize")   && this.__Contains("m_uLength")
         && this.__Contains("m_uHead")      && this.__Contains("m_hBuffer");
#else
        return view_as<bool>(this);
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
    }

    property int Length {
        public get() {
            if (!this.IsValid())
                ThrowError("Invalid CircularDeque Handle %x.", this);

            return this.__GetLength();
        }
    }

    property int BlockSize {
        public get() {
            if (!this.IsValid())
                ThrowError("Invalid CircularDeque Handle %x.", this);

            return this.__GetBuffer().BlockSize;
        }
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

    public int __GetArray(const char[] key, any[] value, int maxlen) {
        int size;
        view_as<StringMap>(this).GetArray(key, value, maxlen, size);
        return size;
    }
    public void __SetArray(const char[] key, const any[] value, int maxlen) {
        view_as<StringMap>(this).SetArray(key, value, maxlen);
    }

    public any __GetRefCnt()                            { return this.__GetValue("m_cRef"); }
    public void __SetRefCnt(int value)                  { this.__SetValue("m_cRef", value); }

    public int __GetOwner(char[] value, int maxlen)     { return this.__GetString("m_szOwner", value, maxlen); }
    public void __SetOwner(const char[] value)          { this.__SetString("m_szOwner", value); }

    public int  __GetMaxSize()                          { return this.__GetValue("m_uMaxSize"); }
    public void __SetMaxSize(int value)                 { this.__SetValue("m_uMaxSize", value); }

    public int  __GetLength()                           { return this.__GetValue("m_uLength"); }
    public void __SetLength(int value)                  { this.__SetValue("m_uLength", value); }

    public int  __GetHead()                             { return this.__GetValue("m_uHead"); }
    public void __SetHead(int value)                    { this.__SetValue("m_uHead", value); }

    // Ownership and lifecycle are the same as this, closed in __CircularDeque_OnClose.
    public ArrayList __GetBuffer()                      { return this.__GetValue("m_hBuffer"); }
    public void __SetBuffer(ArrayList value)            { this.__SetValue("m_hBuffer", value); }

    public int __NextIndex(int index) {
        return index == this.__GetMaxSize() - 1 ? 0 : index + 1;
    }

    public int __PreviousIndex(int index) {
        return index == 0 ? this.__GetMaxSize() - 1 : index - 1;
    }

    public int __IndexAt(int offset) {
        int index = this.__GetHead() + offset;
        int maxSize = this.__GetMaxSize();

        // Compare index and maxSize as unsigned 32-bit values
        // (inline of UInt32Compare: -1 if a < b, 0 if a == b, 1 if a > b).
        int compare;
        if (index == maxSize)
            compare = 0;
        else if ((index & 0x80000000) != (maxSize & 0x80000000))
            compare = index < 0 ? 1 : -1;
        else
            compare = index < maxSize ? -1 : 1;

        if (compare >= 0)
            return index - maxSize;
        return index;
    }

    public bool __IsEmpty() {
        return this.__GetLength() == 0;
    }

    public bool __IsFull() {
        int maxSize = this.__GetMaxSize();
        return maxSize != 0 && maxSize == this.__GetLength();
    }
}


public void __CircularDeque_OnClose(CircularDeque circularDeque)
{
    delete circularDeque.__GetBuffer();
}
