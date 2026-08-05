// -----------------------------------------------------------------------
// RenameBuildingsConfigParsing.inl. JSON config file parser
// Included inline by RenameBuildings.cpp
// Uses ConfigParsingEngine.h for table-driven parsing
// -----------------------------------------------------------------------

#include "tools/shared/ConfigParsingEngine.h"

#include <fstream>
#include <sstream>
#include <string>

static const float kAlmostOutOfBounds = 0.98F; // Clamped between 0.0 and (1.0 - roughly button width / height)

// -----------------------------------------------------------------------
// Metadata table. The ONLY place fields are defined
// Adding a field: add one row here + one entry in ConfigFieldIndex enum
// -----------------------------------------------------------------------

static const ConfigFieldMeta kRenameBuildingsFields[] = {
    // clang-format off
    // JSON key                                offsetof(RenameBuildingsConfig, member)               parser       writer       [clamp  min   max]
    {"enabled",                                offsetof(RenameBuildingsConfig, enabled),              ParseBool,   WriteBool},
    {"buttonX",                                offsetof(RenameBuildingsConfig, buttonX),              ParseFloat,  WriteFloat,  true, 0.0F, kAlmostOutOfBounds},
    {"buttonY",                                offsetof(RenameBuildingsConfig, buttonY),              ParseFloat,  WriteFloat,  true, 0.0F, kAlmostOutOfBounds},
    {"allowRenamingNonPlayerBuildings",        offsetof(RenameBuildingsConfig, allowRenamingNonPlayerBuildings), ParseBool, WriteBool},
    {"allowSelectiveRenames",                  offsetof(RenameBuildingsConfig, allowSelectiveRenames),         ParseBool, WriteBool},
    {"allowFluff",                             offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_FLUFF]),               ParseBool, WriteBool},
    {"allowUsable",                            offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_USABLE]),              ParseBool, WriteBool},
    {"allowStorage",                           offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_STORAGE]),             ParseBool, WriteBool},
    {"allowProduction",                        offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_PRODUCTION]),          ParseBool, WriteBool},
    {"allowResearch",                          offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_RESEARCH]),            ParseBool, WriteBool},
    {"allowCrafting",                          offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_CRAFTING]),            ParseBool, WriteBool},
    {"allowGateway",                           offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_GATEWAY]),             ParseBool, WriteBool},
    {"allowTurret",                            offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_TURRET]),              ParseBool, WriteBool},
    {"allowWall",                              offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_WALL]),                ParseBool, WriteBool},
    {"allowItemFurnace",                       offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_ITEM_FURNACE]),        ParseBool, WriteBool},
    {"allowLight",                             offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_LIGHT]),               ParseBool, WriteBool},
    {"allowShellWithInterior",                 offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_SHELL_WITH_INTERIOR]), ParseBool, WriteBool},
    {"allowFarm",                              offsetof(RenameBuildingsConfig, classRenameable[BCTYPE_FARM]),                ParseBool, WriteBool},
    {"verboseDebugLogging",                    offsetof(RenameBuildingsConfig, verboseDebugLogging),        ParseBool,   WriteBool},
    {"developerDebug",                         offsetof(RenameBuildingsConfig, developerDebug),             ParseBool,   WriteBool},
    // clang-format on
};

static const int kRenameBuildingsFieldCount = sizeof(kRenameBuildingsFields) / sizeof(kRenameBuildingsFields[0]);

// Compile-time guard: table must match enum count
// (VS2010 doesn't support static_assert, but the linker will catch mismatches because
//  the diagnostics array is sized to CFI_COUNT)
// If you add a field to the enum but forget the table row, the table will be one
// entry short. If you add a row but forget the enum, the enum won't reference it.

// -----------------------------------------------------------------------
// Thin wrappers. Forward to the shared engine
// -----------------------------------------------------------------------

static void ResetConfigParseDiagnostics(RenameBuildingsConfigParseDiagnostics *diagnostics)
{
    if (diagnostics == nullptr) { return; }
    ResetParseDiagnostics(diagnostics->fields, kRenameBuildingsFieldCount);
    diagnostics->syntaxError = false;
    diagnostics->syntaxErrorOffset = 0;
}

static bool ParseConfigJson(
    const std::string &body, RenameBuildingsConfig *configOut, RenameBuildingsConfigParseDiagnostics *diagnostics
)
{
    if (configOut == nullptr || diagnostics == nullptr) { return false; }

    return ParseConfigJsonEngine(
        body, kRenameBuildingsFields, kRenameBuildingsFieldCount, configOut, diagnostics->fields,
        &diagnostics->syntaxError, &diagnostics->syntaxErrorOffset
    );
}

static bool ReadConfigFromFile(
    const std::string &configPath, RenameBuildingsConfig *configOut, bool *foundFileOut, bool *needsWriteBackOut
)
{
    if (configOut == nullptr) { return false; }

    RenameBuildingsConfigParseDiagnostics diagnostics;
    ResetConfigParseDiagnostics(&diagnostics);

    bool foundFile = false;
    bool needsWriteBack = false;

    if (!ReadConfigFromFileEngine(
            configPath, kRenameBuildingsFields, kRenameBuildingsFieldCount, configOut, diagnostics.fields, &foundFile,
            &needsWriteBack
        ))
    {
        std::stringstream error;
        error << "ERROR: mod-config.json parse error near byte offset " << diagnostics.syntaxErrorOffset;
        ErrorLog(error.str().c_str());
        return false;
    }

    if (foundFileOut != nullptr) { *foundFileOut = foundFile; }
    if (needsWriteBackOut != nullptr) { *needsWriteBackOut = needsWriteBack; }

    return true;
}

static bool SaveConfigToFile(const std::string &configPath, const RenameBuildingsConfig &config)
{ return SaveConfigToFileEngine(configPath, kRenameBuildingsFields, kRenameBuildingsFieldCount, &config); }