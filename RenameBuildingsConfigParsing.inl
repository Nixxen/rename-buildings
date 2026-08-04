// -----------------------------------------------------------------------
// RenameBuildingsConfigParsing.inl. JSON config file parser
// Included inline by RenameBuildings.cpp
// Based on Job-B-Gone mod's config parser, with modifications for RenameBuildings
// https://github.com/Emkej/Job-B-Gone/blob/main/src/JobBGoneConfigParsing.inl
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
{ return std::isspace(static_cast<unsigned char>(currentCharacter)) != 0 || currentCharacter == ',' || currentCharacter == '}' || currentCharacter == ']'; }

static void SkipUtf8Bom(const std::string &text, size_t *pos)
{
    if (pos == nullptr || *pos != 0 || text.size() < 3) { return; }

    const auto b0 = static_cast<unsigned char>(text[0]);
    const auto b1 = static_cast<unsigned char>(text[1]);
    const auto b2 = static_cast<unsigned char>(text[2]);
    if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF) { *pos = 3; }
}

static bool RecordConfigSyntaxError(RenameBuildingsConfigParseDiagnostics *diagnostics, size_t offset)
{
    if (diagnostics != nullptr)
    {
        diagnostics->syntaxError = true;
        diagnostics->syntaxErrorOffset = offset;
    }
    return false;
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
            valueOut->push_back(text[*pos]);
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

static bool ParseJsonFloatValue(const std::string &text, size_t *pos, float *valueOut)
{
    if (pos == nullptr || valueOut == nullptr) { return false; }

    SkipJsonWhitespace(text, pos);
    size_t cursor = *pos;

    bool sawDigit = false;
    if (cursor < text.size() && text[cursor] == '-') { ++cursor; }
    while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
    {
        sawDigit = true;
        ++cursor;
    }
    if (cursor < text.size() && text[cursor] == '.')
    {
        ++cursor;
        while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])) != 0)
        {
            sawDigit = true;
            ++cursor;
        }
    }
    if (!sawDigit) { return false; }
    if (cursor < text.size() && !IsJsonLiteralTerminator(text[cursor])) { return false; }

    const std::string numberText = text.substr(*pos, cursor - *pos);
    *valueOut = static_cast<float>(std::atof(numberText.c_str()));
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

static void ResetConfigParseDiagnostics(RenameBuildingsConfigParseDiagnostics *diagnostics)
{
    if (diagnostics == nullptr) { return; }

    diagnostics->foundEnabled = false;
    diagnostics->invalidEnabled = false;
    diagnostics->foundButtonX = false;
    diagnostics->invalidButtonX = false;
    diagnostics->foundButtonY = false;
    diagnostics->invalidButtonY = false;
    diagnostics->foundAllowRenamingNonPlayerBuildings = false;
    diagnostics->invalidAllowRenamingNonPlayerBuildings = false;
    diagnostics->foundAllowSelectiveRenames = false;
    diagnostics->invalidAllowSelectiveRenames = false;
    diagnostics->foundAllowFluff = false;
    diagnostics->invalidAllowFluff = false;
    diagnostics->foundAllowUsable = false;
    diagnostics->invalidAllowUsable = false;
    diagnostics->foundAllowStorage = false;
    diagnostics->invalidAllowStorage = false;
    diagnostics->foundAllowProduction = false;
    diagnostics->invalidAllowProduction = false;
    diagnostics->foundAllowResearch = false;
    diagnostics->invalidAllowResearch = false;
    diagnostics->foundAllowCrafting = false;
    diagnostics->invalidAllowCrafting = false;
    diagnostics->foundAllowGateway = false;
    diagnostics->invalidAllowGateway = false;
    diagnostics->foundAllowTurret = false;
    diagnostics->invalidAllowTurret = false;
    diagnostics->foundAllowWall = false;
    diagnostics->invalidAllowWall = false;
    diagnostics->foundAllowItemFurnace = false;
    diagnostics->invalidAllowItemFurnace = false;
    diagnostics->foundAllowLight = false;
    diagnostics->invalidAllowLight = false;
    diagnostics->foundAllowShellWithInterior = false;
    diagnostics->invalidAllowShellWithInterior = false;
    diagnostics->foundAllowFarm = false;
    diagnostics->invalidAllowFarm = false;
    diagnostics->foundVerboseDebugLogging = false;
    diagnostics->invalidVerboseDebugLogging = false;
    diagnostics->foundDeveloperDebug = false;
    diagnostics->invalidDeveloperDebug = false;
    diagnostics->syntaxError = false;
    diagnostics->syntaxErrorOffset = 0;
}

static bool ParseConfigBool(const std::string &body, size_t *pos, bool *found, bool *invalid, bool *valueOut)
{
    size_t valuePos = *pos;
    bool parsed = false;
    if (ParseJsonBoolValue(body, &valuePos, &parsed))
    {
        *found = true;
        *valueOut = parsed;
        *pos = valuePos;
        return true;
    }
    *invalid = true;
    SkipJsonValue(body, pos);
    return false;
}

static bool ParseConfigFloat(const std::string &body, size_t *pos, bool *found, bool *invalid, float *valueOut)
{
    size_t valuePos = *pos;
    float parsed = 0.0F;
    if (ParseJsonFloatValue(body, &valuePos, &parsed))
    {
        *found = true;
        *valueOut = parsed;
        *pos = valuePos;
        return true;
    }
    *invalid = true;
    SkipJsonValue(body, pos);
    return false;
}

static bool ParseConfigJson(
    const std::string &body, RenameBuildingsConfig *configOut, RenameBuildingsConfigParseDiagnostics *diagnostics
)
{
    if (configOut == nullptr || diagnostics == nullptr) { return false; }

    size_t pos = 0;
    SkipUtf8Bom(body, &pos);
    SkipJsonWhitespace(body, &pos);
    if (pos >= body.size() || body[pos] != '{') { return RecordConfigSyntaxError(diagnostics, pos); }

    ++pos;
    SkipJsonWhitespace(body, &pos);
    if (pos < body.size() && body[pos] == '}')
    {
        ++pos;
        SkipJsonWhitespace(body, &pos);
        if (pos == body.size()) { return true; }
        return RecordConfigSyntaxError(diagnostics, pos);
    }

    while (pos < body.size())
    {
        std::string key;
        if (!ParseJsonStringToken(body, &pos, &key)) { return RecordConfigSyntaxError(diagnostics, pos); }

        SkipJsonWhitespace(body, &pos);
        if (pos >= body.size() || body[pos] != ':') { return RecordConfigSyntaxError(diagnostics, pos); }
        ++pos;

        if (key == "enabled")
        {
            ParseConfigBool(body, &pos, &diagnostics->foundEnabled, &diagnostics->invalidEnabled, &configOut->enabled);
        }
        else if (key == "buttonX")
        {
            ParseConfigFloat(body, &pos, &diagnostics->foundButtonX, &diagnostics->invalidButtonX, &configOut->buttonX);
        }
        else if (key == "buttonY")
        {
            ParseConfigFloat(body, &pos, &diagnostics->foundButtonY, &diagnostics->invalidButtonY, &configOut->buttonY);
        }
        else if (key == "allowRenamingNonPlayerBuildings")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowRenamingNonPlayerBuildings,
                &diagnostics->invalidAllowRenamingNonPlayerBuildings, &configOut->allowRenamingNonPlayerBuildings
            );
        }
        else if (key == "allowSelectiveRenames")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowSelectiveRenames, &diagnostics->invalidAllowSelectiveRenames,
                &configOut->allowSelectiveRenames
            );
        }
        else if (key == "allowFluff")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowFluff, &diagnostics->invalidAllowFluff,
                &configOut->classRenameable[BCTYPE_FLUFF]
            );
        }
        else if (key == "allowUsable")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowUsable, &diagnostics->invalidAllowUsable,
                &configOut->classRenameable[BCTYPE_USABLE]
            );
        }
        else if (key == "allowStorage")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowStorage, &diagnostics->invalidAllowStorage,
                &configOut->classRenameable[BCTYPE_STORAGE]
            );
        }
        else if (key == "allowProduction")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowProduction, &diagnostics->invalidAllowProduction,
                &configOut->classRenameable[BCTYPE_PRODUCTION]
            );
        }
        else if (key == "allowResearch")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowResearch, &diagnostics->invalidAllowResearch,
                &configOut->classRenameable[BCTYPE_RESEARCH]
            );
        }
        else if (key == "allowCrafting")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowCrafting, &diagnostics->invalidAllowCrafting,
                &configOut->classRenameable[BCTYPE_CRAFTING]
            );
        }
        else if (key == "allowGateway")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowGateway, &diagnostics->invalidAllowGateway,
                &configOut->classRenameable[BCTYPE_GATEWAY]
            );
        }
        else if (key == "allowTurret")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowTurret, &diagnostics->invalidAllowTurret,
                &configOut->classRenameable[BCTYPE_TURRET]
            );
        }
        else if (key == "allowWall")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowWall, &diagnostics->invalidAllowWall,
                &configOut->classRenameable[BCTYPE_WALL]
            );
        }
        else if (key == "allowItemFurnace")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowItemFurnace, &diagnostics->invalidAllowItemFurnace,
                &configOut->classRenameable[BCTYPE_ITEM_FURNACE]
            );
        }
        else if (key == "allowLight")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowLight, &diagnostics->invalidAllowLight,
                &configOut->classRenameable[BCTYPE_LIGHT]
            );
        }
        else if (key == "allowShellWithInterior")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowShellWithInterior, &diagnostics->invalidAllowShellWithInterior,
                &configOut->classRenameable[BCTYPE_SHELL_WITH_INTERIOR]
            );
        }
        else if (key == "allowFarm")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundAllowFarm, &diagnostics->invalidAllowFarm,
                &configOut->classRenameable[BCTYPE_FARM]
            );
        }
        else if (key == "verboseDebugLogging")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundVerboseDebugLogging, &diagnostics->invalidVerboseDebugLogging,
                &configOut->verboseDebugLogging
            );
        }
        else if (key == "developerDebug")
        {
            ParseConfigBool(
                body, &pos, &diagnostics->foundDeveloperDebug, &diagnostics->invalidDeveloperDebug,
                &configOut->developerDebug
            );
        }
        else
        {
            if (!SkipJsonValue(body, &pos)) { return RecordConfigSyntaxError(diagnostics, pos); }
        }

        SkipJsonWhitespace(body, &pos);
        if (pos >= body.size()) { return RecordConfigSyntaxError(diagnostics, pos); }

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

        return RecordConfigSyntaxError(diagnostics, pos);
    }

    SkipJsonWhitespace(body, &pos);
    if (pos != body.size()) { return RecordConfigSyntaxError(diagnostics, pos); }

    return true;
}

static bool ReadConfigFromFile(
    const std::string &configPath, RenameBuildingsConfig *configOut, bool *foundFileOut, bool *needsWriteBackOut
)
{
    if (configOut == nullptr) { return false; }

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
    RenameBuildingsConfigParseDiagnostics diagnostics;
    ResetConfigParseDiagnostics(&diagnostics);
    if (!ParseConfigJson(body, configOut, &diagnostics))
    {
        std::stringstream error;
        error << "ERROR: mod-config.json parse error near byte offset " << diagnostics.syntaxErrorOffset;
        ErrorLog(error.str().c_str());
        return false;
    }

    bool needsWriteBack = false;

    // Check each field
    if (!diagnostics.foundEnabled || diagnostics.invalidEnabled) { needsWriteBack = true; }
    if (!diagnostics.foundButtonX || diagnostics.invalidButtonX) { needsWriteBack = true; }
    if (!diagnostics.foundButtonY || diagnostics.invalidButtonY) { needsWriteBack = true; }
    if (!diagnostics.foundAllowRenamingNonPlayerBuildings || diagnostics.invalidAllowRenamingNonPlayerBuildings)
    {
        needsWriteBack = true;
    }
    if (!diagnostics.foundAllowSelectiveRenames || diagnostics.invalidAllowSelectiveRenames) { needsWriteBack = true; }
    if (!diagnostics.foundAllowFluff || diagnostics.invalidAllowFluff) { needsWriteBack = true; }
    if (!diagnostics.foundAllowUsable || diagnostics.invalidAllowUsable) { needsWriteBack = true; }
    if (!diagnostics.foundAllowStorage || diagnostics.invalidAllowStorage) { needsWriteBack = true; }
    if (!diagnostics.foundAllowProduction || diagnostics.invalidAllowProduction) { needsWriteBack = true; }
    if (!diagnostics.foundAllowResearch || diagnostics.invalidAllowResearch) { needsWriteBack = true; }
    if (!diagnostics.foundAllowCrafting || diagnostics.invalidAllowCrafting) { needsWriteBack = true; }
    if (!diagnostics.foundAllowGateway || diagnostics.invalidAllowGateway) { needsWriteBack = true; }
    if (!diagnostics.foundAllowTurret || diagnostics.invalidAllowTurret) { needsWriteBack = true; }
    if (!diagnostics.foundAllowWall || diagnostics.invalidAllowWall) { needsWriteBack = true; }
    if (!diagnostics.foundAllowItemFurnace || diagnostics.invalidAllowItemFurnace) { needsWriteBack = true; }
    if (!diagnostics.foundAllowLight || diagnostics.invalidAllowLight) { needsWriteBack = true; }
    if (!diagnostics.foundAllowShellWithInterior || diagnostics.invalidAllowShellWithInterior)
    {
        needsWriteBack = true;
    }
    if (!diagnostics.foundAllowFarm || diagnostics.invalidAllowFarm) { needsWriteBack = true; }
    if (!diagnostics.foundVerboseDebugLogging || diagnostics.invalidVerboseDebugLogging) { needsWriteBack = true; }
    if (!diagnostics.foundDeveloperDebug || diagnostics.invalidDeveloperDebug) { needsWriteBack = true; }

    if (needsWriteBackOut != nullptr) { *needsWriteBackOut = needsWriteBack; }
    return true;
}

static bool SaveConfigToFile(const std::string &configPath, const RenameBuildingsConfig &config)
{
    std::ofstream configOutputStream(configPath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!configOutputStream) { return false; }

    configOutputStream << "{\n";
    configOutputStream << "  \"enabled\": " << (config.enabled ? "true" : "false") << ",\n";
    configOutputStream << "  \"buttonX\": " << config.buttonX << ",\n";
    configOutputStream << "  \"buttonY\": " << config.buttonY << ",\n";
    configOutputStream << "  \"allowRenamingNonPlayerBuildings\": " << (config.allowRenamingNonPlayerBuildings ? "true" : "false")
        << ",\n";
    configOutputStream << "  \"allowSelectiveRenames\": " << (config.allowSelectiveRenames ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowFluff\": " << (config.classRenameable[BCTYPE_FLUFF] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowUsable\": " << (config.classRenameable[BCTYPE_USABLE] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowStorage\": " << (config.classRenameable[BCTYPE_STORAGE] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowProduction\": " << (config.classRenameable[BCTYPE_PRODUCTION] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowResearch\": " << (config.classRenameable[BCTYPE_RESEARCH] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowCrafting\": " << (config.classRenameable[BCTYPE_CRAFTING] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowGateway\": " << (config.classRenameable[BCTYPE_GATEWAY] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowTurret\": " << (config.classRenameable[BCTYPE_TURRET] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowWall\": " << (config.classRenameable[BCTYPE_WALL] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowItemFurnace\": " << (config.classRenameable[BCTYPE_ITEM_FURNACE] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowLight\": " << (config.classRenameable[BCTYPE_LIGHT] ? "true" : "false") << ",\n";
    configOutputStream << "  \"allowShellWithInterior\": " << (config.classRenameable[BCTYPE_SHELL_WITH_INTERIOR] ? "true" : "false")
        << ",\n";
    configOutputStream << "  \"allowFarm\": " << (config.classRenameable[BCTYPE_FARM] ? "true" : "false") << ",\n";
    configOutputStream << "  \"verboseDebugLogging\": " << (config.verboseDebugLogging ? "true" : "false") << ",\n";
    configOutputStream << "  \"developerDebug\": " << (config.developerDebug ? "true" : "false") << "\n";
    configOutputStream << "}\n";

    return true;
}