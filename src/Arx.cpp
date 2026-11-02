#include "Arx.h"

#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>

static arx::context_ ar;

void arx::UseContext(const context_& ctx) {
    memcpy(&ar, &ctx, sizeof(context_));
}

arx::error_ arx::ReadArchiveHeader(archiveHeader_* matched) {
    size_t numMagic = ar.read(matched->magic, 1, ARX_MAGIC_LENGTH, ar.user);
    if (numMagic != ARX_MAGIC_LENGTH)
        return error_::kReadArchiveMagic;

    bool magicMatched = strncmp(ARX_MAGIC_STR, matched->magic, ARX_MAGIC_LENGTH) == 0;
    if (!magicMatched)
        return error_::kMatchArchiveMagic;

    return error_::kNone;
}

arx::error_ arx::ReadMemberHeader(memberHeader_* matched) {
    size_t numRead = 0;
    
    numRead = ar.read(&matched->identLen, 1, 1, ar.user);
    if (numRead != 1) {
        return error_::kStreamRead;
    }

    if (matched->identLen >= ARX_MAX_IDENTIFIER_LENGTH) {
        return error_::kIdentifierLength;
    }

    numRead = ar.read(matched->ident, 1, matched->identLen, ar.user);
    if (numRead != matched->identLen) {
        return error_::kStreamRead;
    }

    assert(numRead == matched->identLen);
    assert(matched->identLen < ARX_MAX_IDENTIFIER_LENGTH);
    matched->ident[matched->identLen] = 0;

    numRead = ar.read(&matched->size, sizeof(u32), 1, ar.user);
    if (numRead != 1) {
        return error_::kStreamRead;
    }

    return error_::kNone;
}

arx::error_ arx::ReadMember(const memberHeader_& member, void* bufOut, size_t bufSize) {
    size_t size = member.size;
    bool result = (ar.read(bufOut, 1, bufSize, ar.user) == size);
    return result ? error_::kNone : error_::kStreamRead;
}

bool arx::CheckEnd() {
    u8 byte = 0;
    size_t numRead = ar.read(&byte, 1, 1, ar.user);
    if (numRead != 1 || byte == 0) {
        return true;
    }
    ar.seek(ar.user, -1, SEEK_CUR);
    return false;
}

void arx::SkipMember(const memberHeader_& member) {
    // The seek callback takes a long, which may only hold 31 bits.
    const u32 maxStep = 0x7FFFFFFF;
    u32 remaining = member.size;
    while (remaining > 0) {
        u32 step = remaining < maxStep ? remaining : maxStep;
        ar.seek(ar.user, static_cast<long>(step), SEEK_CUR);
        remaining -= step;
    }
}

const char* arx::to_string(error_ error) {
    switch (error) {
        case error_::kNone:
            return "None";
            break;
        case error_::kIdentifierLength:
            return "IdentifierLength";
            break;
        case error_::kReadArchiveMagic:
            return "ReadArchiveMagic";
            break;
        case error_::kMatchArchiveMagic:
            return "MatchArchiveMagic";
            break;
        case error_::kStreamRead:
            return "StreamRead";
            break;
        case error_::kUnknown:
            return "Unknown";
            break;
    };
    assert(false);
    return "Unknown";
}
