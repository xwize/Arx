#ifndef __ARX_H__
#define __ARX_H__

#define ARX_MAGIC_LENGTH 3
#define ARX_MAX_IDENTIFIER_LENGTH 32
#define ARX_MAGIC_STR "arx"

namespace arx
{
    using u8  =  unsigned __int8;
    using u16 =  unsigned __int16;
    using u32 =  unsigned __int32;

    typedef long    (*tellFunc)     (void* ptr);
    typedef int     (*seekFunc)     (void* ptr, long offset, int origin);
    typedef size_t  (*readFunc)     (void* buf, size_t elementSize, size_t count, void* ptr);
    typedef size_t  (*writeFunc)    (void* buf, size_t elementSize, size_t count, void* ptr);

    struct context_ {
        tellFunc     tell;
        seekFunc     seek;
        readFunc     read;
        writeFunc    write;
        void*        user;
    };

    enum class error_ {
        kNone,
        kIdentiferLength,
        kReadArchiveMagic,
        kMatchArchiveMagic,
        kStreamRead,
        kUnknown
    };

    const char* to_string(error_ error);

#pragma pack(push)
    struct archiveHeader_ {
        char magic[ARX_MAGIC_LENGTH];
    };

    struct memberHeader_ {
        u8      identLen;
        char    ident[ARX_MAX_IDENTIFIER_LENGTH + 1];
        u32     size;
    };

    void            UseContext(const context_& callbacks);
    error_          ReadArchiveHeader(archiveHeader_* matched);
    error_          ReadMemberHeader(memberHeader_* matched);
    error_          ReadMember(const memberHeader_& memHead, void* bufOut, size_t bufSize);
    bool            CheckEnd();
    void            SkipMember(const memberHeader_& memHead);
}

#endif // __ARX_H__