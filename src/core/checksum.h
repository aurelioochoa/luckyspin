#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

/*
 * checksum.h - FNV-1a 64-bit integrity check for save files.
 *
 * The checksum is appended to the save as a final "checksum=<hex>" line and
 * covers every byte before it. It is a corruption detector, not a security
 * measure - the file is user-writable, so tampering stays possible. On
 * mismatch the loader warns but still loads (warn-only), and saves written
 * before this feature simply have no checksum line and load silently.
 */
namespace checksum {

const uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
const uint64_t FNV_PRIME        = 1099511628211ULL;

// FNV-1a 64-bit over a byte range.
inline uint64_t fnv1a(const char* data, size_t length) {
    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < length; i++) {
        hash ^= static_cast<unsigned char>(data[i]);
        hash *= FNV_PRIME;
    }
    return hash;
}

inline uint64_t fnv1a(const std::string& text) {
    return fnv1a(text.data(), text.size());
}

// Lowercase 16-digit zero-padded hex, stable across platforms.
inline std::string toHex(uint64_t hash) {
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}

// Parse a hex string back into a hash. Rejects anything malformed.
inline bool fromHex(const std::string& text, uint64_t& out) {
    if (text.empty()) return false;
    uint64_t value = 0;
    for (size_t i = 0; i < text.size(); i++) {
        const char c = text[i];
        value <<= 4;
        if (c >= '0' && c <= '9')      value |= static_cast<uint64_t>(c - '0');
        else if (c >= 'a' && c <= 'f') value |= static_cast<uint64_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') value |= static_cast<uint64_t>(c - 'A' + 10);
        else return false;
    }
    out = value;
    return true;
}

} // namespace checksum

#endif // CHECKSUM_H
