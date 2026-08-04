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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdlib>
#include <fstream>
#include <sstream>

static const char *kPluginName = "Rename Buildings";
static const char *kConfigFileName = "mod-config.json";

static const float kDefaultButtonX = 0.01F;
static const float kDefaultButtonY = 0.75F;

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

// Global UI widgets
MyGUI::Window *gRenameWindow = nullptr;
MyGUI::Button *gShowRenameWindowButton = nullptr;
bool gDragging = false;
int gDragStartX = 0;
int gDragStartY = 0;
int gButtonStartX = 0;
int gButtonStartY = 0;

// Ctrl+T debug hotkey edge-detection state
static bool gCtrlTPressedLast = false;

// Prevents the rename window from opening on the first click after a drag
static bool gWasDragged = false;

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
    if (!gConfig.classRenameable[buildingClass]) { return false; }

    if (!gConfig.allowRenamingNonPlayerBuildings && !building->isThePlayer()) { return false; }

    return true;
}

// UI callback: attempt to rename the currently selected building
void OnRenameButtonPress(MyGUI::WidgetPtr sender)
{
    auto *edit = dynamic_cast<MyGUI::EditBox *>(gRenameWindow->findWidget("RenameBuildingEdit"));
    if (edit == nullptr)
    {
        DebugLog("Edit box not found");
        return;
    }

    std::string newName = edit->getCaption();
    // Trim whitespace from both ends
    size_t start = newName.find_first_not_of(" \t\r\n");
    size_t end = newName.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) { newName.clear(); }
    else
    {
        newName = newName.substr(start, end - start + 1);
    }
    if (newName.empty())
    {
        DebugLog("New name is empty or whitespace only");
        return;
    }

    Building *building = GetRenameTarget(ou->player->selectedObject.getBuilding());
    if (!IsValidBuildingRename(building))
    {
        DebugLog("Building is not valid for renaming");
        return;
    }

    building->setName(newName);
    building->notifyChange();
    DebugLog(("Renamed to " + newName).c_str());
    gRenameWindow->setVisible(false);
}

void OnCloseRenameWindow(MyGUI::Window *sender, const std::string &name) { gRenameWindow->setVisible(false); }

// UI callback: show the rename window, pre-filled with current building name
// If the window is already visible, close it instead
void OnShowRenameWindow(MyGUI::WidgetPtr sender)
{
    // Consume the first click after a drag
    if (gWasDragged)
    {
        gWasDragged = false;
        return;
    }

    if (gRenameWindow->getVisible())
    {
        gRenameWindow->setVisible(false);
        return;
    }

    Building *building = GetRenameTarget(ou->player->selectedObject.getBuilding());
    if (building != nullptr && IsValidBuildingRename(building))
    {
        DebugLog("Showing rename window");
        auto *edit = dynamic_cast<MyGUI::EditBox *>(gRenameWindow->findWidget("RenameBuildingEdit"));
        if (edit != nullptr) { edit->setCaption(building->getName()); }
        gRenameWindow->setVisible(true);
    }
}

// Drag start: record button position and mouse offset at press
void OnRenameButtonMousePress(MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton mouseButtonId)
{
    if (mouseButtonId == MyGUI::MouseButton::Left)
    {
        gDragging = true;
        MyGUI::IntPoint pos = sender->getPosition();
        gButtonStartX = pos.left;
        gButtonStartY = pos.top;
        gDragStartX = left;
        gDragStartY = top;
    }
}

// Drag move: reposition button with mouse delta, clamped to screen
void OnRenameButtonMouseDrag(MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton mouseButtonId)
{
    if (gDragging && mouseButtonId == MyGUI::MouseButton::Left)
    {
        gWasDragged = true;

        int deltaX = left - gDragStartX;
        int deltaY = top - gDragStartY;
        int newLeft = gButtonStartX + deltaX;
        int newTop = gButtonStartY + deltaY;

        // Clamp to parent (screen) edges
        MyGUI::IntSize parentSize = sender->getParentSize();
        MyGUI::IntSize buttonSize = sender->getSize();
        newLeft = std::max(0, newLeft);
        newTop = std::max(0, newTop);
        if (newLeft + buttonSize.width > parentSize.width) { newLeft = parentSize.width - buttonSize.width; }
        if (newTop + buttonSize.height > parentSize.height) { newTop = parentSize.height - buttonSize.height; }

        sender->setPosition(newLeft, newTop);
    }
}

static bool SaveConfigState();

// Drag end
void OnRenameButtonMouseRelease(MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton mouseButtonId)
{
    if (mouseButtonId == MyGUI::MouseButton::Left)
    {
        gDragging = false;

        // Persist button position as normalized coordinates (0.0-1.0) to config
        MyGUI::IntPoint pos = sender->getPosition();
        MyGUI::IntSize parentSize = sender->getParentSize();
        if (parentSize.width > 0 && parentSize.height > 0)
        {
            gConfig.buttonX = static_cast<float>(pos.left) / static_cast<float>(parentSize.width);
            gConfig.buttonY = static_cast<float>(pos.top) / static_cast<float>(parentSize.height);
            SaveConfigState();
        }
    }
}

static const char *BuildingClassTypeName(BuildingClassType type)
{
    switch (type)
    {
    case BCTYPE_FLUFF:
        return "BCTYPE_FLUFF";
    case BCTYPE_DOOR:
        return "BCTYPE_DOOR";
    case BCTYPE_USABLE:
        return "BCTYPE_USABLE";
    case BCTYPE_STORAGE:
        return "BCTYPE_STORAGE";
    case BCTYPE_PRODUCTION:
        return "BCTYPE_PRODUCTION";
    case BCTYPE_RESEARCH:
        return "BCTYPE_RESEARCH";
    case BCTYPE_CRAFTING:
        return "BCTYPE_CRAFTING";
    case BCTYPE_GATEWAY:
        return "BCTYPE_GATEWAY";
    case BCTYPE_TURRET:
        return "BCTYPE_TURRET";
    case BCTYPE_WALL:
        return "BCTYPE_WALL";
    case BCTYPE_ITEM_FURNACE:
        return "BCTYPE_ITEM_FURNACE";
    case BCTYPE_LIGHT:
        return "BCTYPE_LIGHT";
    case BCTYPE_SHELL_WITH_INTERIOR:
        return "BCTYPE_SHELL_WITH_INTERIOR";
    case BCTYPE_FARM:
        return "BCTYPE_FARM";
    default:
        return "BCTYPE_UNKNOWN";
    }
}

static const char *BuildingDesignationName(BuildingDesignation designation)
{
    switch (designation)
    {
    case BD_NONE:
        return "BD_NONE";
    case BD_SHOP:
        return "BD_SHOP";
    case BD_BARRACKS:
        return "BD_BARRACKS";
    case BD_BAR:
        return "BD_BAR";
    case BD_HOSPITAL:
        return "BD_HOSPITAL";
    case BD_ARMOURY:
        return "BD_ARMOURY";
    case BD_TREASURE:
        return "BD_TREASURE";
    case BD_PRISON:
        return "BD_PRISON";
    case BD_HQ:
        return "BD_HQ";
    case BD_RESIDENTIAL:
        return "BD_RESIDENTIAL";
    case BD_SLAVE_STORAGE:
        return "BD_SLAVE_STORAGE";
    case BD_RESIDENTIAL_SMALL:
        return "BD_RESIDENTIAL_SMALL";
    default:
        return "BD_UNKNOWN";
    }
}

// Dumps verbose building info to the debug log
static void DumpBuildingInfo(Building *building)
{
    std::ostringstream oss;
    oss << "=== Building Dump ===";
    DebugLog(oss.str());
    oss.str("");

    // Pointer
    oss << "  ptr: " << building;
    DebugLog(oss.str());
    oss.str("");

    // Display name + data string ID
    oss << "  displayName: \"" << building->getName() << "\"";
    DebugLog(oss.str());
    oss.str("");

    if (building->data != nullptr)
    {
        oss << "  data->name: \"" << building->data->name << "\"";
        DebugLog(oss.str());
        oss.str("");

        oss << "  data->stringID: \"" << building->data->stringID << "\"";
        DebugLog(oss.str());
        oss.str("");

        oss << "  data->id: " << building->data->id;
        DebugLog(oss.str());
        oss.str("");
    }
    else
    {
        DebugLog("  data: nullptr");
    }

    // Class type and designation
    oss << "  buildingClass: " << BuildingClassTypeName(building->getBuildingClass());
    DebugLog(oss.str());
    oss.str("");

    oss << "  designation: " << BuildingDesignationName(building->getBuildingDesignation());
    DebugLog(oss.str());
    oss.str("");

    // Boolean flags
    oss << "  isThePlayer: " << (building->isThePlayer() ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    oss << "  isSign: " << (building->isSign() ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    oss << "  isForSale: " << (building->isForSale() ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    oss << "  isDoor: " << (building->isDoor() ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    oss << "  isFoliage: " << (building->isFoliage ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    oss << "  destroyed: " << (building->destroyed ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    oss << "  imADoor: " << (building->imADoor ? "true" : "false");
    DebugLog(oss.str());
    oss.str("");

    // Door parent building (if this is a door)
    if (building->isDoor() || building->imADoor)
    {
        Building *parent = building->doorParentBuilding();
        if (parent != nullptr)
        {
            oss << "   doorParent: ptr=" << parent << " name=\"" << parent->getName()
                << "\" class=" << BuildingClassTypeName(parent->getBuildingClass()) << " data=\""
                << (parent->data != nullptr ? parent->data->stringID : "nullptr") << "\"";
            DebugLog(oss.str());
            oss.str("");
        }
        else
        {
            DebugLog("   doorParent: nullptr");
        }
    }

    // Position
    oss << "  pos: (" << building->pos.x << ", " << building->pos.y << ", " << building->pos.z << ")";
    DebugLog(oss.str());
    oss.str("");

    // Handle info
    oss << "  handle.index: " << building->handle.index;
    DebugLog(oss.str());
    oss.str("");

    oss << "  handle.serial: " << building->handle.serial;
    DebugLog(oss.str());
    oss.str("");

    oss << "  handle.type: " << static_cast<int>(building->handle.type);
    DebugLog(oss.str());
    oss.str("");

    // Construction state
    Building::ConstructionState *buildState = building->getBuildState();
    if (buildState != nullptr)
    {
        oss << "  buildState: " << buildState;
        DebugLog(oss.str());
        oss.str("");

        oss << "  buildState->isComplete: " << (buildState->isComplete ? "true" : "false");
        DebugLog(oss.str());
        oss.str("");

        oss << "  buildState->constructionProgress: " << buildState->constructionProgress;
        DebugLog(oss.str());
        oss.str("");
    }
    else
    {
        DebugLog("  buildState: nullptr");
    }

    DebugLog("=== End Building Dump ===");
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
        ErrorLog("RenameBuildings ERROR: failed to read mod-config.json; using defaults and rewriting file");
        gConfigNeedsWriteBack = true;
        return;
    }

    gConfigNeedsWriteBack = (!foundConfigFile) || needsWriteBack;
    if (!foundConfigFile) { DebugLog("RenameBuildings INFO: mod-config.json not found; using defaults"); }

    ApplySelectiveRenames();

    std::stringstream info;
    info << "RenameBuildings INFO: loaded config enabled=" << (gConfig.enabled ? "true" : "false") << " settingsPath=\""
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
        ErrorLog("RenameBuildings ERROR: settings path is empty; cannot save mod-config.json");
        return false;
    }

    if (!SaveConfigToFile(gSettingsPath, gConfig))
    {
        std::stringstream error;
        error << "RenameBuildings ERROR: failed to save mod-config.json path=\"" << gSettingsPath << "\"";
        ErrorLog(error.str().c_str());
        return false;
    }

    DebugLog("RenameBuildings INFO: saved mod-config.json");
    return true;
}

// Checks debug hotkeys and dispatches actions on rising edge
static void CheckDebugHotkeys()
{
    if (!gConfig.developerDebug) { return; }

    static const int kKeyStatePressed = 0x8000;

    bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & kKeyStatePressed) != 0;
    bool tKeyDown = (GetAsyncKeyState('T') & kKeyStatePressed) != 0;
    bool ctrlT = ctrlDown && tKeyDown;

    if (ctrlT && !gCtrlTPressedLast)
    {
        Building *building = ou->player->selectedObject.getBuilding();
        if (building != nullptr)
        {
            DebugLog("Ctrl+T pressed: dumping building info for selected building");
            DumpBuildingInfo(GetRenameTarget(building));
        }
        else
        {
            DebugLog("No building selected");
        }
    }
    gCtrlTPressedLast = ctrlT;
}

// Main loop hook: check if a player-owned building is selected
void (*GameWorld_mainLoop_orig)(GameWorld *thisptr, float time);
void GameWorld_mainLoop_hook(GameWorld *thisptr, float time)
{
    if (gShowRenameWindowButton != nullptr)
    {
        Building *building = ou->player->selectedObject.getBuilding();

        // Verbose debug: dump building info on new selection (any ownership, any class)
        if (gConfig.verboseDebugLogging)
        {
            static Building *sPreviousBuilding = nullptr;
            if (building != nullptr && building != sPreviousBuilding) { DumpBuildingInfo(building); }
            sPreviousBuilding = building;
        }

        if (IsValidBuildingRename(building)) { gShowRenameWindowButton->setVisible(true); }
        else
        {
            gShowRenameWindowButton->setVisible(false);
            gRenameWindow->setVisible(false);
        }
    }

    CheckDebugHotkeys();

    GameWorld_mainLoop_orig(thisptr, time);
}

// Title screen constructor hook
TitleScreen *(*TitleScreen_orig)(TitleScreen *) = nullptr;
TitleScreen *TitleScreen_hook(TitleScreen *thisptr)
{
    // Call original constructor
    TitleScreen *titleScreen = TitleScreen_orig(thisptr);

    MyGUI::Gui *gui = MyGUI::Gui::getInstancePtr();

    static const float kRenameWindowX = 0.25F;
    static const float kRenameWindowY = 0.35F;
    static const float kRenameWindowWidth = 0.30F;
    static const float kRenameWindowHeight = 0.072F;
    static const float kEditBoxX = 0.05F;
    static const float kEditBoxY = 0.10F;
    static const float kEditBoxWidth = 0.68F;
    static const float kEditBoxHeight = 0.70F;
    static const float kConfirmButtonX = 0.75F;
    static const float kConfirmButtonY = 0.10F;
    static const float kConfirmButtonWidth = 0.19F;
    static const float kConfirmButtonHeight = 0.70F;
    static const float kShowButtonX = gConfig.buttonX;
    static const float kShowButtonY = gConfig.buttonY;
    static const float kShowButtonWidth = 0.06F;
    static const float kShowButtonHeight = 0.02F;

    // Create rename window (hidden initially)
    gRenameWindow = gui->createWidgetReal<MyGUI::Window>(
        "Kenshi_WindowCX", kRenameWindowX, kRenameWindowY, kRenameWindowWidth, kRenameWindowHeight,
        MyGUI::Align::Center, "Window", "RenameBuildingWindow"
    );
    gRenameWindow->setCaption("Rename Building");
    gRenameWindow->setVisible(false);
    gRenameWindow->eventWindowButtonPressed += MyGUI::newDelegate(OnCloseRenameWindow);

    MyGUI::EditBox *edit = gRenameWindow->getClientWidget()->createWidgetReal<MyGUI::EditBox>(
        "Kenshi_EditBox", kEditBoxX, kEditBoxY, kEditBoxWidth, kEditBoxHeight, MyGUI::Align::Default,
        "RenameBuildingEdit"
    );
    edit->setCaption("");

    MyGUI::Button *confirmRenameButton = gRenameWindow->getClientWidget()->createWidgetReal<MyGUI::Button>(
        "Kenshi_Button1", kConfirmButtonX, kConfirmButtonY, kConfirmButtonWidth, kConfirmButtonHeight,
        MyGUI::Align::Center, "ConfirmRenameBuildingButton"
    );
    confirmRenameButton->setCaption("Rename");
    confirmRenameButton->eventMouseButtonClick += MyGUI::newDelegate(OnRenameButtonPress);

    // Create the "Rename" selection button (hidden initially)
    // Shows the main rename window when clicked, and can be dragged around the screen
    gShowRenameWindowButton = gui->createWidgetReal<MyGUI::Button>(
        "Kenshi_Button1", kShowButtonX, kShowButtonY, kShowButtonWidth, kShowButtonHeight, MyGUI::Align::Default,
        "Window", "ShowRenameBuildingWindowButton"
    );
    gShowRenameWindowButton->setCaption("Rename");
    gShowRenameWindowButton->setVisible(false);
    gShowRenameWindowButton->eventMouseButtonClick += MyGUI::newDelegate(OnShowRenameWindow);
    gShowRenameWindowButton->eventMouseButtonPressed += MyGUI::newDelegate(OnRenameButtonMousePress);
    gShowRenameWindowButton->eventMouseDrag += MyGUI::newDelegate(OnRenameButtonMouseDrag);
    gShowRenameWindowButton->eventMouseButtonReleased += MyGUI::newDelegate(OnRenameButtonMouseRelease);

    return titleScreen;
}

__declspec(dllexport) void startPlugin()
{
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
}