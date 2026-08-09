// -----------------------------------------------------------------------
// ConfigParsingEngine.h - Table-driven JSON config parser/serializer
// Version: 0.1.0
// Copyright (C) 2026  Nixxen
// Partial code derived from Job-B-Gone by Emkej
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// To use:
//   1. Define a metadata table (one ConfigFieldMeta per config field)
//   2. Define a ParseState array (one per field)
//   3. Call ReadConfigFromFileEngine / SaveConfigToFileEngine
//   4. Adding a field = one row in the table + one member in your config struct
// -----------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

// -----------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------

struct ConfigFieldMeta; // forward

struct ParseState
{
    bool found;    // Key was present in JSON
    bool invalid;  // Value was wrong type / unparseable
    bool modified; // Value was clamped or reset to default
};

// Parser callback: reads a JSON value from body at *pos, writes to dest.
// On failure, sets state->invalid and skips the value.
typedef void (*ParseFunc)(
    const std::string &body, size_t *pos, void *dest, const ConfigFieldMeta &meta, ParseState *state
);

// Writer callback: writes the value to stream in JSON format.
typedef void (*WriteFunc)(std::ostream &stream, const void *value);

struct ConfigFieldMeta
{
    const char *jsonKey; // JSON key string
    size_t memberOffset; // offsetof(ConfigType, member)
    ParseFunc parser;    // Parses JSON value into the config member
    WriteFunc writer;    // Writes the config member as JSON
    // The following three fields default to false / 0.0F when omitted from the initializer.
    // Only specify them when clamping is needed.
    bool clampEnabled; // Whether to clamp numeric values
    float clampMin;    // Clamp minimum (only used when clampEnabled)
    float clampMax;    // Clamp maximum (only used when clampEnabled)
};

// -----------------------------------------------------------------------
// Primitive JSON parser helpers
// -----------------------------------------------------------------------

static void SkipJsonWhitespace(const std::string &text, size_t *pos)
{
    if (pos == nullptr) { return; }

    while (*pos < text.size() && std::isspace(static_cast<unsigned char>(text[*pos])) != 0)
    {
        ++(*pos);
    }
}

static bool IsJsonLiteralTerminator(char currentCharacter)
{
    return std::isspace(static_cast<unsigned char>(currentCharacter)) != 0 || currentCharacter == ',' ||
           currentCharacter == '}' || currentCharacter == ']';
}

// Some text editors insert a UTF-8 byte-order-mark at the very beginning of the file: EF BB BF. This marker is
// invisible to the user but would break JSON parsing because '{' is no longer at offset 0. Skip it if present.
static void SkipUtf8Bom(const std::string &text, size_t *pos)
{
    if (pos == nullptr || *pos != 0 || text.size() < 3) { return; }

    if (static_cast<unsigned char>(text[0]) == 0xEF && static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF)
    {
        *pos = 3;
    }
}

static bool ParseJsonStringToken(const std::string &text, size_t *pos, std::string *valueOut)
{
    if (pos == nullptr || valueOut == nullptr) { return false; }

    SkipJsonWhitespace(text, pos);
    if (*pos >= text.size() || text[*pos] != '"') { return false; }

    ++(*pos);
    valueOut->clear();

    while (*pos < text.size())
    {
        const char currentCharacter = text[*pos];
        if (currentCharacter == '"')
        {
            ++(*pos);
            return true;
        }

        if (currentCharacter == '\\')
        {
            ++(*pos);
            if (*pos >= text.size()) { return false; }

            const char escapedCharacter = text[*pos];
            switch (escapedCharacter)
            {
            case '"':
                valueOut->push_back('"');
                break;
            case '\\':
                valueOut->push_back('\\');
                break;
            case '/':
                valueOut->push_back('/');
                break;
            case 'n':
                valueOut->push_back('\n');
                break;
            case 'r':
                valueOut->push_back('\r');
                break;
            case 't':
                valueOut->push_back('\t');
                break;
            case 'b':
                valueOut->push_back('\b');
                break;
            case 'f':
                valueOut->push_back('\f');
                break;
            default:
                valueOut->push_back(escapedCharacter); // Invalid JSON escape sequence
            }
            ++(*pos);
            continue;
        }

        valueOut->push_back(currentCharacter);
        ++(*pos);
    }

    return false;
}

static bool ParseJsonBoolValue(const std::string &text, size_t *pos, bool *valueOut)
{
    if (pos == nullptr || valueOut == nullptr) { return false; }

    SkipJsonWhitespace(text, pos);

    if (*pos + 4 <= text.size() && text.compare(*pos, 4, "true") == 0)
    {
        const size_t end = *pos + 4;
        if (end == text.size() || IsJsonLiteralTerminator(text[end]))
        {
            *valueOut = true;
            *pos = end;
            return true;
        }
    }

    if (*pos + 5 <= text.size() && text.compare(*pos, 5, "false") == 0)
    {
        const size_t end = *pos + 5;
        if (end == text.size() || IsJsonLiteralTerminator(text[end]))
        {
            *valueOut = false;
            *pos = end;
            return true;
        }
    }

    return false;
}

// Scans past an optional '-' and one or more digits. Returns false if no digits found.
// On success, *cursor points past the last digit.
static bool ScanJsonDigits(const std::string &text, size_t *cursor)
{
    if (cursor == nullptr) { return false; }
    if (*cursor < text.size() && text[*cursor] == '-') { ++(*cursor); }
    bool sawDigit = false;
    while (*cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[*cursor])) != 0)
    {
        sawDigit = true;
        ++(*cursor);
    }
    return sawDigit;
}

static bool ParseJsonFloatValue(const std::string &text, size_t *pos, float *valueOut)
{
    if (pos == nullptr || valueOut == nullptr) { return false; }

    SkipJsonWhitespace(text, pos);
    size_t cursor = *pos;

    if (!ScanJsonDigits(text, &cursor)) { return false; }
    if (cursor < text.size() && text[cursor] == '.')
    {
        ++cursor;
        bool sawFractionDigit = false;
        while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
        {
            sawFractionDigit = true;
            ++cursor;
        }
        if (!sawFractionDigit) { return false; }
    }
    if (cursor < text.size() && (text[cursor] == 'e' || text[cursor] == 'E'))
    {
        ++cursor;
        if (cursor < text.size() && (text[cursor] == '+' || text[cursor] == '-')) { ++cursor; }

        bool sawExponentDigit = false;
        while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
        {
            sawExponentDigit = true;
            ++cursor;
        }
        if (!sawExponentDigit) { return false; }
    }
    if (cursor < text.size() && !IsJsonLiteralTerminator(text[cursor])) { return false; }

    const std::string numberText = text.substr(*pos, cursor - *pos);
    *valueOut = static_cast<float>(std::atof(numberText.c_str()));
    *pos = cursor;
    return true;
}

static bool ParseJsonIntValue(const std::string &text, size_t *pos, int *valueOut)
{
    if (pos == nullptr || valueOut == nullptr) { return false; }

    SkipJsonWhitespace(text, pos);
    size_t cursor = *pos;

    if (!ScanJsonDigits(text, &cursor)) { return false; }
    if (cursor < text.size() && !IsJsonLiteralTerminator(text[cursor])) { return false; }

    const std::string numberText = text.substr(*pos, cursor - *pos);
    *valueOut = std::atoi(numberText.c_str());
    *pos = cursor;
    return true;
}

static bool SkipJsonValue(const std::string &text, size_t *pos);

static bool SkipJsonObject(const std::string &text, size_t *pos)
{
    if (pos == nullptr || *pos >= text.size() || text[*pos] != '{') { return false; }

    ++(*pos);
    SkipJsonWhitespace(text, pos);
    if (*pos < text.size() && text[*pos] == '}')
    {
        ++(*pos);
        return true;
    }

    while (*pos < text.size())
    {
        std::string ignoredKey;
        if (!ParseJsonStringToken(text, pos, &ignoredKey)) { return false; }

        SkipJsonWhitespace(text, pos);
        if (*pos >= text.size() || text[*pos] != ':') { return false; }

        ++(*pos);
        if (!SkipJsonValue(text, pos)) { return false; }

        SkipJsonWhitespace(text, pos);
        if (*pos >= text.size()) { return false; }

        if (text[*pos] == ',')
        {
            ++(*pos);
            continue;
        }

        if (text[*pos] == '}')
        {
            ++(*pos);
            return true;
        }

        return false;
    }

    return false;
}

static bool SkipJsonArray(const std::string &text, size_t *pos)
{
    if (pos == nullptr || *pos >= text.size() || text[*pos] != '[') { return false; }

    ++(*pos);
    SkipJsonWhitespace(text, pos);
    if (*pos < text.size() && text[*pos] == ']')
    {
        ++(*pos);
        return true;
    }

    while (*pos < text.size())
    {
        if (!SkipJsonValue(text, pos)) { return false; }

        SkipJsonWhitespace(text, pos);
        if (*pos >= text.size()) { return false; }

        if (text[*pos] == ',')
        {
            ++(*pos);
            continue;
        }

        if (text[*pos] == ']')
        {
            ++(*pos);
            return true;
        }

        return false;
    }

    return false;
}

static bool SkipJsonValue(const std::string &text, size_t *pos)
{
    if (pos == nullptr) { return false; }

    SkipJsonWhitespace(text, pos);
    if (*pos >= text.size()) { return false; }

    const char currentCharacter = text[*pos];
    if (currentCharacter == '"')
    {
        std::string ignored;
        return ParseJsonStringToken(text, pos, &ignored);
    }

    if (currentCharacter == '{') { return SkipJsonObject(text, pos); }
    if (currentCharacter == '[') { return SkipJsonArray(text, pos); }

    if (currentCharacter == '-' || std::isdigit(static_cast<unsigned char>(currentCharacter)) != 0)
    {
        size_t cursor = *pos;
        if (text[cursor] == '-') { ++cursor; }

        bool sawDigit = false;
        while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
        {
            sawDigit = true;
            ++cursor;
        }

        if (!sawDigit) { return false; }

        if (cursor < text.size() && text[cursor] == '.')
        {
            ++cursor;
            bool sawFractionDigit = false;
            while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
            {
                sawFractionDigit = true;
                ++cursor;
            }
            if (!sawFractionDigit) { return false; }
        }

        if (cursor < text.size() && (text[cursor] == 'e' || text[cursor] == 'E'))
        {
            ++cursor;
            if (cursor < text.size() && (text[cursor] == '+' || text[cursor] == '-')) { ++cursor; }

            bool sawExponentDigit = false;
            while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
            {
                sawExponentDigit = true;
                ++cursor;
            }
            if (!sawExponentDigit) { return false; }
        }

        *pos = cursor;
        return true;
    }

    if (*pos + 4 <= text.size() && text.compare(*pos, 4, "true") == 0)
    {
        *pos += 4;
        return true;
    }
    if (*pos + 5 <= text.size() && text.compare(*pos, 5, "false") == 0)
    {
        *pos += 5;
        return true;
    }
    if (*pos + 4 <= text.size() && text.compare(*pos, 4, "null") == 0)
    {
        *pos += 4;
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------
// Config field lookup helper
// -----------------------------------------------------------------------

// Returns pointer to value within config struct given offset
static void *GetValuePtr(void *configBase, const ConfigFieldMeta &meta)
{ return static_cast<char *>(configBase) + meta.memberOffset; }

static const void *GetValuePtrConst(const void *configBase, const ConfigFieldMeta &meta)
{ return static_cast<const char *>(configBase) + meta.memberOffset; }

// Linear-scan lookup: returns index or -1 if not found
static int FindFieldByKey(const std::string &key, const ConfigFieldMeta *fields, int fieldCount)
{
    for (int i = 0; i < fieldCount; ++i)
    {
        if (key == fields[i].jsonKey) { return i; }
    }
    return -1;
}

// -----------------------------------------------------------------------
// Parser callbacks (one per type)
// -----------------------------------------------------------------------

static void
ParseBool(const std::string &body, size_t *pos, void *dest, const ConfigFieldMeta & /*meta*/, ParseState *state)
{
    bool parsed = false;
    if (ParseJsonBoolValue(body, pos, &parsed))
    {
        state->found = true;
        *static_cast<bool *>(dest) = parsed;
        return;
    }
    state->invalid = true;
    SkipJsonValue(body, pos);
}

static void ParseInt(const std::string &body, size_t *pos, void *dest, const ConfigFieldMeta &meta, ParseState *state)
{
    int parsed = 0;
    if (ParseJsonIntValue(body, pos, &parsed))
    {
        state->found = true;
        if (meta.clampEnabled)
        {
            const int intMin = static_cast<int>(meta.clampMin);
            const int intMax = static_cast<int>(meta.clampMax);
            if (parsed < intMin)
            {
                parsed = intMin;
                state->modified = true;
            }
            if (parsed > intMax)
            {
                parsed = intMax;
                state->modified = true;
            }
        }
        *static_cast<int *>(dest) = parsed;
        return;
    }
    state->invalid = true;
    SkipJsonValue(body, pos);
}

static void ParseFloat(const std::string &body, size_t *pos, void *dest, const ConfigFieldMeta &meta, ParseState *state)
{
    float parsed = 0.0F;
    if (ParseJsonFloatValue(body, pos, &parsed))
    {
        state->found = true;
        if (meta.clampEnabled)
        {
            if (parsed < meta.clampMin)
            {
                parsed = meta.clampMin;
                state->modified = true;
            }
            if (parsed > meta.clampMax)
            {
                parsed = meta.clampMax;
                state->modified = true;
            }
        }
        *static_cast<float *>(dest) = parsed;
        return;
    }
    state->invalid = true;
    SkipJsonValue(body, pos);
}

static void
ParseString(const std::string &body, size_t *pos, void *dest, const ConfigFieldMeta & /*meta*/, ParseState *state)
{
    if (ParseJsonStringToken(body, pos, static_cast<std::string *>(dest)))
    {
        state->found = true;
        return;
    }
    state->invalid = true;
    SkipJsonValue(body, pos);
}

// -----------------------------------------------------------------------
// Writer callbacks (one per type)
// -----------------------------------------------------------------------

static void WriteBool(std::ostream &stream, const void *value)
{ stream << (*static_cast<const bool *>(value) ? "true" : "false"); }

static void WriteInt(std::ostream &stream, const void *value) { stream << *static_cast<const int *>(value); }

static void WriteFloat(std::ostream &stream, const void *value) { stream << *static_cast<const float *>(value); }

// Writes a string as a JSON string literal, escaping quotes, backslashes, and control characters.
static void WriteEscapedJsonString(std::ostream &stream, const std::string &value)
{
    stream << '"';
    for (size_t i = 0; i < value.size(); ++i)
    {
        const char c = value[i];
        switch (c)
        {
        case '"':
            stream << "\\\"";
            break;
        case '\\':
            stream << "\\\\";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            stream << c;
            break;
        }
    }
    stream << '"';
}

static void WriteString(std::ostream &stream, const void *value)
{ WriteEscapedJsonString(stream, *static_cast<const std::string *>(value)); }

// -----------------------------------------------------------------------
// Engine: parse a complete JSON config object
// -----------------------------------------------------------------------

// Records a syntax error and returns false. Callers use this as their error return.
static bool RecordSyntaxError(bool *syntaxError, size_t *syntaxErrorOffset, size_t offset)
{
    if (syntaxError != nullptr) { *syntaxError = true; }
    if (syntaxErrorOffset != nullptr) { *syntaxErrorOffset = offset; }
    return false;
}

// Returns true on success (no syntax errors).
// Individual field parse failures are recorded in diagFields[].invalid.
static bool ParseConfigJsonEngine(
    const std::string &body,
    const ConfigFieldMeta *fields,
    int fieldCount,
    void *configBase,
    ParseState *diagFields,
    bool *syntaxError,
    size_t *syntaxErrorOffset
)
{
    if (configBase == nullptr || fields == nullptr || diagFields == nullptr || fieldCount <= 0) { return false; }

    size_t pos = 0;
    SkipUtf8Bom(body, &pos);
    SkipJsonWhitespace(body, &pos);
    if (pos >= body.size() || body[pos] != '{') { return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos); }

    ++pos;
    SkipJsonWhitespace(body, &pos);
    if (pos < body.size() && body[pos] == '}')
    {
        ++pos;
        SkipJsonWhitespace(body, &pos);
        if (pos == body.size()) { return true; }
        return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos);
    }

    while (pos < body.size())
    {
        std::string key;
        if (!ParseJsonStringToken(body, &pos, &key)) { return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos); }

        SkipJsonWhitespace(body, &pos);
        if (pos >= body.size() || body[pos] != ':') { return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos); }
        ++pos;

        const int fieldIndex = FindFieldByKey(key, fields, fieldCount);
        if (fieldIndex >= 0)
        {
            const ConfigFieldMeta &meta = fields[fieldIndex];
            meta.parser(body, &pos, GetValuePtr(configBase, meta), meta, &diagFields[fieldIndex]);
        }
        else
        {
            if (!SkipJsonValue(body, &pos)) { return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos); }
        }

        SkipJsonWhitespace(body, &pos);
        if (pos >= body.size()) { return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos); }

        if (body[pos] == ',')
        {
            ++pos;
            SkipJsonWhitespace(body, &pos);
            continue;
        }

        if (body[pos] == '}')
        {
            ++pos;
            break;
        }

        return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos);
    }

    SkipJsonWhitespace(body, &pos);
    if (pos != body.size()) { return RecordSyntaxError(syntaxError, syntaxErrorOffset, pos); }

    return true;
}

// -----------------------------------------------------------------------
// Engine: reset all per-field diagnostics
// -----------------------------------------------------------------------

static void ResetParseDiagnostics(ParseState *diagFields, int fieldCount)
{
    if (diagFields == nullptr) { return; }
    for (int i = 0; i < fieldCount; ++i)
    {
        diagFields[i].found = false;
        diagFields[i].invalid = false;
        diagFields[i].modified = false;
    }
}

// -----------------------------------------------------------------------
// Engine: check if any fields need write-back
// -----------------------------------------------------------------------

static bool CheckFieldsNeedWriteBack(const ParseState *diagFields, int fieldCount)
{
    if (diagFields == nullptr) { return false; }
    for (int i = 0; i < fieldCount; ++i)
    {
        if (!diagFields[i].found || diagFields[i].invalid || diagFields[i].modified) { return true; }
    }
    return false;
}

// -----------------------------------------------------------------------
// Engine: save config to JSON file
// -----------------------------------------------------------------------

static bool SaveConfigToFileEngine(
    const std::string &configPath, const ConfigFieldMeta *fields, int fieldCount, const void *configBase
)
{
    if (fields == nullptr || configBase == nullptr || fieldCount <= 0) { return false; }

    std::ofstream configOutputStream(configPath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!configOutputStream) { return false; }

    configOutputStream << "{\n";
    for (int i = 0; i < fieldCount; ++i)
    {
        const ConfigFieldMeta &meta = fields[i];
        configOutputStream << "  \"" << meta.jsonKey << "\": ";
        meta.writer(configOutputStream, GetValuePtrConst(configBase, meta));

        if (i < fieldCount - 1) { configOutputStream << ",\n"; }
        else
        {
            configOutputStream << "\n";
        }
    }
    configOutputStream << "}\n";

    return true;
}

// -----------------------------------------------------------------------
// Engine: read config from file (parse + write-back detection)
// -----------------------------------------------------------------------

// On success returns true. *foundFile is set if the file existed.
// *needsWriteBack is set if any field was missing/invalid/modified.
static bool ReadConfigFromFileEngine(
    const std::string &configPath,
    const ConfigFieldMeta *fields,
    int fieldCount,
    void *configBase,
    ParseState *diagFields,
    bool *foundFileOut,
    bool *needsWriteBackOut
)
{
    if (configBase == nullptr || fields == nullptr || diagFields == nullptr || fieldCount <= 0) { return false; }

    if (foundFileOut != nullptr) { *foundFileOut = false; }
    if (needsWriteBackOut != nullptr) { *needsWriteBackOut = false; }

    std::ifstream configInputStream(configPath.c_str(), std::ios::in | std::ios::binary);
    if (!configInputStream)
    {
        if (needsWriteBackOut != nullptr) { *needsWriteBackOut = true; }
        return true;
    }

    if (foundFileOut != nullptr) { *foundFileOut = true; }

    const std::string body((std::istreambuf_iterator<char>(configInputStream)), std::istreambuf_iterator<char>());
    ResetParseDiagnostics(diagFields, fieldCount);

    bool syntaxError = false;
    size_t syntaxErrorOffset = 0;
    if (!ParseConfigJsonEngine(body, fields, fieldCount, configBase, diagFields, &syntaxError, &syntaxErrorOffset))
    {
        // Syntax errors are logged by the caller (they have access to ErrorLog)
        return false;
    }

    if (needsWriteBackOut != nullptr) { *needsWriteBackOut = CheckFieldsNeedWriteBack(diagFields, fieldCount); }
    return true;
}