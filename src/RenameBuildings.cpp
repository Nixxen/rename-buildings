#include <Debug.h>

#include <kenshi/Building/Building.h>
#include <kenshi/Faction.h>
#include <kenshi/GameWorld.h>
#include <kenshi/Globals.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/gui/TitleScreen.h>

#include <mygui/MyGUI_Button.h>
#include <mygui/MyGUI_Delegate.h>
#include <mygui/MyGUI_EditBox.h>
#include <mygui/MyGUI_Gui.h>
#include <mygui/MyGUI_Window.h>

#include <core/Functions.h>

#include "RenameBuildingsConfig.h"
#include "version.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

static const char *kPluginName = "Rename Buildings";
static const char *kConfigFileName = "mod-config.json";

static const float kDefaultButtonX = 0.01F;
static const float kDefaultButtonY = 0.75F;
static const float kMinimumDragDistance = 0.005F;

static RenameBuildingsConfig gConfig = {
    true,            // enabled
    kDefaultButtonX, // buttonX
    kDefaultButtonY, // buttonY
    false,           // allowRenamingNonPlayerBuildings
    false,           // allowSelectiveRenames
    {
        true, // BCTYPE_FLUFF
        true, // BCTYPE_DOOR
        true, // BCTYPE_USABLE
        true, // BCTYPE_STORAGE
        true, // BCTYPE_PRODUCTION
        true, // BCTYPE_RESEARCH
        true, // BCTYPE_CRAFTING
        true, // BCTYPE_GATEWAY
        true, // BCTYPE_TURRET
        true, // BCTYPE_WALL
        true, // BCTYPE_ITEM_FURNACE
        true, // BCTYPE_LIGHT
        true, // BCTYPE_SHELL_WITH_INTERIOR
        true  // BCTYPE_FARM
    },
    false, // verboseDebugLogging
    false  // developerDebug
};

static std::string gSettingsPath;
static bool gConfigNeedsWriteBack = false;

// Resolve config file path from the DLL location (runs before startPlugin)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        char dllPath[_MAX_PATH] = {0};
        if (GetModuleFileNameA(hModule, dllPath, _MAX_PATH) > 0)
        {
            std::string fullPath(dllPath);
            size_t sep = fullPath.find_last_of("\\/");
            if (sep != std::string::npos) { gSettingsPath = fullPath.substr(0, sep) + "\\" + kConfigFileName; }
        }
    }
    return TRUE;
}

// Resolves a door building to its parent building for renaming.
// Returns the parent if the building is a door, otherwise the building itself.
static Building *GetRenameTarget(Building *building)
{
    if (building == nullptr) { return nullptr; }
    if (building->isDoor() || building->imADoor) { return building->doorParentBuilding(); }
    return building;
}

// Validates whether a building is eligible for renaming.
// Checks: non-null, class type in the renameable list, and (unless overridden) player ownership.
static bool IsValidBuildingRename(Building *building)
{
    if (building == nullptr) { return false; }
    if (!gConfig.enabled) { return false; }

    BuildingClassType buildingClass = building->getBuildingClass();
    if (buildingClass < 0 || buildingClass > BCTYPE_FARM) { return false; }
    if (gConfig.allowSelectiveRenames && !gConfig.classRenameable[buildingClass]) { return false; }

    if (!gConfig.allowRenamingNonPlayerBuildings && !building->isThePlayer()) { return false; }

    return true;
}

// -----------------------------------------------------------------------
// Config parsing and state helpers (inlined)
// -----------------------------------------------------------------------
#include "RenameBuildingsConfigParsing.inl"

static void ApplySelectiveRenames()
{
    if (!gConfig.allowSelectiveRenames)
    {
        // When selective renames are disabled, all class types are renameable
        for (int i = 0; i <= BCTYPE_FARM; ++i)
        {
            gConfig.classRenameable[i] = true;
        }
    }
    // When selective renames are enabled, per-class flags from config are used as-is
}

static void LoadConfigState()
{
    gConfigNeedsWriteBack = false;
    gConfig.enabled = true;
    gConfig.buttonX = kDefaultButtonX;
    gConfig.buttonY = kDefaultButtonY;
    gConfig.allowRenamingNonPlayerBuildings = false;
    gConfig.allowSelectiveRenames = false;
    for (int i = 0; i <= BCTYPE_FARM; ++i)
    {
        gConfig.classRenameable[i] = true;
    }
    gConfig.verboseDebugLogging = false;
    gConfig.developerDebug = false;

    if (gSettingsPath.empty()) { return; }

    bool foundConfigFile = false;
    bool needsWriteBack = false;
    if (!ReadConfigFromFile(gSettingsPath, &gConfig, &foundConfigFile, &needsWriteBack))
    {
        ErrorLog("ERROR: failed to read mod-config.json; using defaults and rewriting file");
        gConfigNeedsWriteBack = true;
        return;
    }

    gConfigNeedsWriteBack = (!foundConfigFile) || needsWriteBack;
    if (!foundConfigFile) { DebugLog("INFO: mod-config.json not found; using defaults"); }

    ApplySelectiveRenames();

    std::stringstream info;
    info << "INFO: loaded config enabled=" << (gConfig.enabled ? "true" : "false") << " settingsPath=\""
         << gSettingsPath << "\""
         << " allowRenamingNonPlayerBuildings=" << (gConfig.allowRenamingNonPlayerBuildings ? "true" : "false")
         << " allowSelectiveRenames=" << (gConfig.allowSelectiveRenames ? "true" : "false")
         << " verboseDebugLogging=" << (gConfig.verboseDebugLogging ? "true" : "false")
         << " developerDebug=" << (gConfig.developerDebug ? "true" : "false");
    DebugLog(info.str().c_str());
}

static bool SaveConfigState()
{
    if (gSettingsPath.empty())
    {
        ErrorLog("ERROR: settings path is empty; cannot save mod-config.json");
        return false;
    }

    if (!SaveConfigToFile(gSettingsPath, gConfig))
    {
        std::stringstream error;
        error << "ERROR: failed to save mod-config.json path=\"" << gSettingsPath << "\"";
        ErrorLog(error.str().c_str());
        return false;
    }

    DebugLog("INFO: saved mod-config.json");
    return true;
}

// -----------------------------------------------------------------------
// Debug helpers (inlined)
// -----------------------------------------------------------------------
#include "RenameBuildingsDebug.inl"

// -----------------------------------------------------------------------
// UI callbacks and drag handling (inlined)
// -----------------------------------------------------------------------
#include "RenameBuildingsUI.inl"

// -----------------------------------------------------------------------
// Emkej's Mod Core (Mod Hub) integration (inlined)
// -----------------------------------------------------------------------
#include "RenameBuildingsModHub.inl"

// -----------------------------------------------------------------------
// KenshiLib hooks (inlined)
// -----------------------------------------------------------------------
#include "RenameBuildingsHooks.inl"

__declspec(dllexport) void startPlugin()
{
    DebugLog("v" RB_VERSION_STRING " loaded");

    LoadConfigState();
    if (gConfigNeedsWriteBack) { SaveConfigState(); }

    if (KenshiLib::SUCCESS !=
        KenshiLib::AddHook(KenshiLib::GetRealAddress(&TitleScreen::_CONSTRUCTOR), TitleScreen_hook, &TitleScreen_orig))
    {
        ErrorLog("Could not add hook!");
    }
    else
    {
        DebugLog("Hook installed");
    }

    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
                                  KenshiLib::GetRealAddress(&GameWorld::_NV_mainLoop_GPUSensitiveStuff),
                                  GameWorld_mainLoop_hook, &GameWorld_mainLoop_orig
                              ))
    {
        ErrorLog("Could not add main loop hook!");
    }
    else
    {
        DebugLog("Main loop hook installed");
    }

    RenameBuildingsModHub_OnStartup();
}