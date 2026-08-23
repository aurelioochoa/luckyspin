/*
 * json.h - Minimal JSON writing and flat-object parsing (no dependencies)
 *
 * The web GUI talks to the game through two endpoints, so this header only
 * implements exactly what those need:
 *
 *   - Writing: jsonEscape() for strings plus trivial numeric formatting.
 *   - Parsing: request bodies are flat objects such as
 *         {"action":"plinko","bet":10,"risk":2}
 *     FlatParse reads string/number/bool values into parallel maps. Nested
 *     arrays or objects are rejected rather than mis-parsed - the API has
 *     no need for them.
 */

#ifndef WEB_JSON_H
#define WEB_JSON_H

#include <string>
#include <map>
#include <sstream>
#include <cstdio>

namespace json {

// ------------------------------------------------------------------------
// WRITING
// ------------------------------------------------------------------------

inline std::string escape(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 8);
    for (size_t i = 0; i < in.size(); i++) {
        const unsigned char c = (unsigned char)in[i];
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += (char)c;   // UTF-8 bytes pass through untouched
                }
        }
    }
    return out;
}

// Fixed-precision number (money keeps cents readable, never scientific).
inline std::string num(double value, int precision = 2) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", precision, value);
    // Trim trailing zeros but keep at least one decimal digit off integers?
    // Keep fixed precision: stable column widths beat compactness here.
    return buf;
}

// Integer-valued numbers (counts, ids, timestamps).
inline std::string integer(long long value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%lld", value);
    return buf;
}

inline std::string boolean(bool value) { return value ? "true" : "false"; }

// ------------------------------------------------------------------------
// PARSING (flat objects only)
// ------------------------------------------------------------------------

struct FlatObject {
    std::map<std::string, std::string> strings;
    std::map<std::string, double> numbers;
    std::map<std::string, bool> booleans;

    std::string getString(const std::string& key,
                          const std::string& fallback = "") const {
        auto it = strings.find(key);
        return it == strings.end() ? fallback : it->second;
    }
    double getNumber(const std::string& key, double fallback = 0.0) const {
        auto it = numbers.find(key);
        return it == numbers.end() ? fallback : it->second;
    }
    bool getBool(const std::string& key, bool fallback = false) const {
        auto it = booleans.find(key);
        return it == booleans.end() ? fallback : it->second;
    }
};

/*
 * FlatParseResult.ok is false on any structural violation (trailing garbage,
 * unterminated string, nested containers). A rejected body must never
 * half-mutate the game, so callers treat !ok as "ignore request". Duplicate
 * keys keep the last value.
 */
struct FlatParseResult {
    bool ok;
    FlatObject object;
};

inline void skipWhitespace(const std::string& s, size_t& i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) ++i;
}

inline bool parseString(const std::string& s, size_t& i, std::string& out) {
    if (i >= s.size() || s[i] != '"') return false;
    ++i;
    out.clear();
    while (i < s.size()) {
        const char c = s[i++];
        if (c == '"') return true;
        if (c == '\\') {
            if (i >= s.size()) return false;
            const char e = s[i++];
            switch (e) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                case 'n':  out += '\n'; break;
                case 't':  out += '\t'; break;
                case 'r':  out += '\r'; break;
                case 'b':  out += '\b'; break;
                case 'f':  out += '\f'; break;
                case 'u': {
                    if (i + 4 > s.size()) return false;
                    // Decode to Latin-1-ish best effort; the API never sends
                    // non-ASCII keys/values, so exact Unicode is unnecessary.
                    unsigned int code = 0;
                    for (int k = 0; k < 4; k++) {
                        const char h = s[i++];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= (unsigned)(h - '0');
                        else if (h >= 'a' && h <= 'f') code |= (unsigned)(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= (unsigned)(h - 'A' + 10);
                        else return false;
                    }
                    if (code < 0x80) {
                        out += (char)code;
                    } else if (code < 0x800) {
                        out += (char)(0xC0 | (code >> 6));
                        out += (char)(0x80 | (code & 0x3F));
                    } else {
                        out += (char)(0xE0 | (code >> 12));
                        out += (char)(0x80 | ((code >> 6) & 0x3F));
                        out += (char)(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default: return false;
            }
        } else {
            out += c;
        }
    }
    return false;   // ran off the end without a closing quote
}

inline bool parseNumber(const std::string& s, size_t& i, double& out) {
    size_t start = i;
    if (i < s.size() && (s[i] == '-' || s[i] == '+')) ++i;
    bool digits = false;
    while (i < s.size()) {
        const char c = s[i];
        if (c >= '0' && c <= '9') { digits = true; ++i; }
        else if (c == '.') { ++i; }
        else if (c == 'e' || c == 'E') { ++i; if (i < s.size() && (s[i] == '-' || s[i] == '+')) ++i; }
        else break;
    }
    if (!digits) return false;
    try {
        out = std::stod(s.substr(start, i - start));
    } catch (...) {
        return false;   // e.g. 1e999 overflowing to out_of_range
    }
    return true;
}

inline FlatParseResult parseFlatObject(const std::string& body) {
    FlatParseResult result{};
    result.ok = false;
    size_t i = 0;
    skipWhitespace(body, i);
    if (i >= body.size() || body[i] != '{') return result;
    ++i;

    FlatObject obj;
    skipWhitespace(body, i);
    if (i < body.size() && body[i] == '}') {
        result.ok = true;
        result.object = obj;
        return result;
    }

    while (true) {
        skipWhitespace(body, i);
        std::string key;
        if (!parseString(body, i, key)) return result;
        skipWhitespace(body, i);
        if (i >= body.size() || body[i] != ':') return result;
        ++i;
        skipWhitespace(body, i);

        if (i < body.size() && body[i] == '"') {
            std::string value;
            if (!parseString(body, i, value)) return result;
            obj.strings[key] = value;
        } else if (i < body.size() && (body[i] == 't' || body[i] == 'f')) {
            if (body.compare(i, 4, "true") == 0) {
                obj.booleans[key] = true;
                i += 4;
            } else if (body.compare(i, 5, "false") == 0) {
                obj.booleans[key] = false;
                i += 5;
            } else {
                return result;
            }
        } else if (i < body.size() && body[i] == 'n'
                   && body.compare(i, 4, "null") == 0) {
            i += 4;   // treated as absent
        } else {
            double value = 0.0;
            if (!parseNumber(body, i, value)) return result;
            obj.numbers[key] = value;
        }

        skipWhitespace(body, i);
        if (i < body.size() && body[i] == ',') { ++i; continue; }
        if (i < body.size() && body[i] == '}') {
            ++i;
            skipWhitespace(body, i);
            result.ok = (i == body.size());   // no trailing garbage
            result.object = obj;
            return result;
        }
        return result;
    }
}

} // namespace json

#endif // WEB_JSON_H
