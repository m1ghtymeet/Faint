#pragma once

namespace utf8 {
    inline char32_t next(const char*& ptr) {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(ptr);
        char32_t cp = 0;
        if (*p <= 0x7F) {
            cp = *p++;
        }
        else if ((*p & 0xE0) == 0xC0) {
            cp = (*p++ & 0x1F) << 6;
            cp |= (*p++ & 0x3F);
        }
        else if ((*p & 0xF0) == 0xE0) {
            cp = (*p++ & 0x0F) << 12;
            cp |= (*p++ & 0x3F) << 6;
            cp |= (*p++ & 0x3F);
        }
        else if ((*p & 0xF8) == 0xF0) {
            cp = (*p++ & 0x07) << 18;
            cp |= (*p++ & 0x3F) << 12;
            cp |= (*p++ & 0x3F) << 6;
            cp |= (*p++ & 0x3F);
        }
        else {
            cp = 0xFFFD;
            ++p;
        }
        ptr = reinterpret_cast<const char*>(p);
        return cp;
    }
}