#ifndef ARX_H_
#define ARX_H_

#include <cstddef>
#include <cstdint>

#define ARX_MAGIC_LENGTH 3
#define ARX_MAX_IDENTIFIER_LENGTH 32
#define ARX_MAGIC_STR "arx"

namespace arx
{
typedef int  (*SeekFunc) (void* ptr, long offset, int origin);
typedef std::size_t  (*ReadFunc) (void* buf, std::size_t elementSize, std::size_t count, void* ptr);

struct Context
{
    SeekFunc     seek;
    ReadFunc     read;
    void*        user;
};

enum class Error
{
    kNone,
    kIdentifierLength,
    kReadArchiveMagic,
    kMatchArchiveMagic,
    kNotFound,
    kStreamRead,
    kUnknown
};

struct ArchiveHeader
{
    char magic[ARX_MAGIC_LENGTH];
};

struct MemberHeader
{
    std::uint8_t ident_len;
    char ident[ARX_MAX_IDENTIFIER_LENGTH + 1];
    std::uint32_t size;
};

bool            check_end(const Context& context);
Error           find_member(const Context& context, const char* ident, MemberHeader* found_out);
Error           read_archive_header(const Context& context, ArchiveHeader* matched);
Error           read_member_header(const Context& context, MemberHeader* matched);
Error           read_member(const Context& context, const MemberHeader& mem_head, void* buf_out, std::size_t buf_size);
void            skip_member(const Context& context, const MemberHeader& mem_head);
const char*     to_string(Error error);
} // namespace arx

#endif // ARX_H_

#ifdef ARX_IMPLEMENTATION
#include <cassert>
#include <cstring>
#include <cstdio>

namespace arx
{

bool check_end(const Context& context)
{
    std::uint8_t byte = 0;
    std::size_t num_read = context.read(&byte, 1, 1, context.user);
    if (num_read != 1 || byte == 0) {
        return true;
    }
    context.seek(context.user, -1, SEEK_CUR);
    return false;
}

Error find_member(const Context& context, const char* ident, MemberHeader* found_out)
{
    while (!check_end(context)) {

        MemberHeader mem_head{};
        if (const Error err = read_member_header(context, &mem_head); err != Error::kNone)
            return err;

        if (std::strcmp(ident, mem_head.ident) == 0) {
            if (found_out)
                *found_out = mem_head;
            return Error::kNone;
        }

        skip_member(context, mem_head);
    }

    return Error::kNotFound;
}

Error read_archive_header(const Context& context, ArchiveHeader* matched)
{
    std::size_t num_magic = context.read(matched->magic, 1, ARX_MAGIC_LENGTH, context.user);
    if (num_magic != ARX_MAGIC_LENGTH) {
        return Error::kReadArchiveMagic;
    }

    bool magic_matched = std::strncmp(ARX_MAGIC_STR, matched->magic, ARX_MAGIC_LENGTH) == 0;
    if (!magic_matched) {
        return Error::kMatchArchiveMagic;
    }

    return Error::kNone;
}

Error read_member_header(const Context& context, MemberHeader* matched)
{
    std::size_t num_read = 0;

    num_read = context.read(&matched->ident_len, 1, 1, context.user);
    if (num_read != 1) {
        return Error::kStreamRead;
    }

    if (matched->ident_len >= ARX_MAX_IDENTIFIER_LENGTH) {
        return Error::kIdentifierLength;
    }

    num_read = context.read(matched->ident, 1, matched->ident_len, context.user);
    if (num_read != matched->ident_len) {
        return Error::kStreamRead;
    }

    assert(num_read == matched->ident_len);
    assert(matched->ident_len < ARX_MAX_IDENTIFIER_LENGTH);
    matched->ident[matched->ident_len] = 0;

    num_read = context.read(&matched->size, sizeof(std::uint32_t), 1, context.user);
    if (num_read != 1) {
        return Error::kStreamRead;
    }

    return Error::kNone;
}

Error read_member(const Context& context, const MemberHeader& mem_head, void* buf_out, std::size_t buf_size)
{
    const bool result = context.read(buf_out, 1, buf_size, context.user) == mem_head.size;
    return result ? Error::kNone : Error::kStreamRead;
}

void skip_member(const Context& context, const MemberHeader& mem_head)
{
    std::uint32_t remaining = mem_head.size;
    while (remaining > 0) {
        // The seek callback takes a long, which may only hold 31 bits.
        constexpr std::uint32_t max_step = 0x7FFFFFFF;
        const std::uint32_t step = remaining < max_step ? remaining : max_step;
        context.seek(context.user, static_cast<long>(step), SEEK_CUR);
        remaining -= step;
    }
}

const char* to_string(Error error)
{
    switch (error) {
    case Error::kNone: return "None";
        break;
    case Error::kIdentifierLength: return "IdentifierLength";
        break;
    case Error::kReadArchiveMagic: return "ReadArchiveMagic";
        break;
    case Error::kMatchArchiveMagic: return "MatchArchiveMagic";
        break;
    case Error::kNotFound: return "NotFound";
        break;
    case Error::kStreamRead: return "StreamRead";
        break;
    case Error::kUnknown: return "Unknown";
        break;
    };
    assert(false);
    return "Unknown";
}
} // namespace arx

#endif // ARX_IMPLEMENTATION