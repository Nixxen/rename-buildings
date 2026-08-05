#pragma once

#include <kenshi/Building/Building.h>

#include "tools/shared/ConfigParsingEngine.h"

// Per-field index for the metadata table / diagnostics array.
// Adding a field: add enum entry here, add one row in the metadata table.
enum ConfigFieldIndex
{
    CFI_ENABLED = 0,
    CFI_BUTTON_X,
    CFI_BUTTON_Y,
    CFI_ALLOW_RENAMING_NON_PLAYER_BUILDINGS,
    CFI_ALLOW_SELECTIVE_RENAMES,
    CFI_ALLOW_FLUFF, // classRenameable[BCTYPE_FLUFF]
    CFI_ALLOW_USABLE,
    CFI_ALLOW_STORAGE,
    CFI_ALLOW_PRODUCTION,
    CFI_ALLOW_RESEARCH,
    CFI_ALLOW_CRAFTING,
    CFI_ALLOW_GATEWAY,
    CFI_ALLOW_TURRET,
    CFI_ALLOW_WALL,
    CFI_ALLOW_ITEM_FURNACE,
    CFI_ALLOW_LIGHT,
    CFI_ALLOW_SHELL_WITH_INTERIOR,
    CFI_ALLOW_FARM,
    CFI_VERBOSE_DEBUG_LOGGING,
    CFI_DEVELOPER_DEBUG,

    CFI_COUNT
};

struct RenameBuildingsConfig
{
    bool enabled;
    float buttonX;
    float buttonY;
    bool allowRenamingNonPlayerBuildings;
    bool allowSelectiveRenames;
    bool classRenameable[BCTYPE_FARM + 1]; // indexed by BuildingClassType
    bool verboseDebugLogging;
    bool developerDebug;
};

struct RenameBuildingsConfigParseDiagnostics
{
    ParseState fields[CFI_COUNT];
    bool syntaxError;
    size_t syntaxErrorOffset;
};