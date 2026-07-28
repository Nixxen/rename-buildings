#include <Debug.h>

#include <kenshi/Building/Building.h>
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Set to true to enable verbose on-click building debug dumps for any building.
static const bool kVerboseDebugLogging = false;

// Global UI widgets
MyGUI::Window *gRenameWindow = nullptr;
MyGUI::Button *gShowRenameWindowButton = nullptr;
bool gDragging = false;
int gDragStartX = 0;
int gDragStartY = 0;
int gButtonStartX = 0;
int gButtonStartY = 0;

// Resolves a door building to its parent building for renaming.
// Returns the parent if the building is a door, otherwise the building itself.
static Building *GetRenameTarget(Building *building)
{
    if (building == nullptr) { return nullptr; }
    if (building->isDoor() || building->imADoor) { return building->doorParentBuilding(); }
    return building;
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
    if (building != nullptr)
    {
        if (building->isThePlayer())
        {
            building->setName(newName);
            building->notifyChange();
            DebugLog(("Renamed to " + newName).c_str());
            gRenameWindow->setVisible(false);
        }
        else
        {
            DebugLog("Building is not player-owned");
        }
    }
    else
    {
        DebugLog("No building selected");
    }
}

void OnCloseRenameWindow(MyGUI::Window *sender, const std::string &name) { gRenameWindow->setVisible(false); }

// UI callback: show the rename window, pre-filled with current building name
// If the window is already visible, close it instead
void OnShowRenameWindow(MyGUI::WidgetPtr sender)
{
    if (gRenameWindow->getVisible())
    {
        gRenameWindow->setVisible(false);
        return;
    }

    Building *building = GetRenameTarget(ou->player->selectedObject.getBuilding());
    if (building != nullptr && building->isThePlayer())
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

// Drag end
void OnRenameButtonMouseRelease(MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton mouseButtonId)
{
    if (mouseButtonId == MyGUI::MouseButton::Left) { gDragging = false; }
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
    std::stringstream ss;
    ss << "=== Building Dump ===";
    DebugLog(ss.str().c_str());
    ss.str("");

    // Pointer
    ss << "  ptr: " << building;
    DebugLog(ss.str().c_str());
    ss.str("");

    // Display name + data string ID
    ss << "  displayName: \"" << building->getName() << "\"";
    DebugLog(ss.str().c_str());
    ss.str("");

    if (building->data != nullptr)
    {
        ss << "  data->name: \"" << building->data->name << "\"";
        DebugLog(ss.str().c_str());
        ss.str("");

        ss << "  data->stringID: \"" << building->data->stringID << "\"";
        DebugLog(ss.str().c_str());
        ss.str("");

        ss << "  data->id: " << building->data->id;
        DebugLog(ss.str().c_str());
        ss.str("");
    }
    else
    {
        DebugLog("  data: nullptr");
    }

    // Class type and designation
    ss << "  buildingClass: " << BuildingClassTypeName(building->getBuildingClass());
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  designation: " << BuildingDesignationName(building->getBuildingDesignation());
    DebugLog(ss.str().c_str());
    ss.str("");

    // Boolean flags
    ss << "  isThePlayer: " << (building->isThePlayer() ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  isSign: " << (building->isSign() ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  isForSale: " << (building->isForSale() ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  isDoor: " << (building->isDoor() ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  isFoliage: " << (building->isFoliage ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  destroyed: " << (building->destroyed ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  imADoor: " << (building->imADoor ? "true" : "false");
    DebugLog(ss.str().c_str());
    ss.str("");

    // Door parent building (if this is a door)
    if (building->isDoor() || building->imADoor)
    {
        Building *parent = building->doorParentBuilding();
        if (parent != nullptr)
        {
            ss << "   doorParent: ptr=" << parent << " name=\"" << parent->getName()
               << "\" class=" << BuildingClassTypeName(parent->getBuildingClass()) << " data=\""
               << (parent->data != nullptr ? parent->data->stringID : "nullptr") << "\"";
            DebugLog(ss.str().c_str());
            ss.str("");
        }
        else
        {
            DebugLog("   doorParent: nullptr");
        }
    }

    // Position
    ss << "  pos: (" << building->pos.x << ", " << building->pos.y << ", " << building->pos.z << ")";
    DebugLog(ss.str().c_str());
    ss.str("");

    // Handle info
    ss << "  handle.index: " << building->handle.index;
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  handle.serial: " << building->handle.serial;
    DebugLog(ss.str().c_str());
    ss.str("");

    ss << "  handle.type: " << static_cast<int>(building->handle.type);
    DebugLog(ss.str().c_str());
    ss.str("");

    // Construction state
    Building::ConstructionState *buildState = building->getBuildState();
    if (buildState != nullptr)
    {
        ss << "  buildState: " << buildState;
        DebugLog(ss.str().c_str());
        ss.str("");

        ss << "  buildState->isComplete: " << (buildState->isComplete ? "true" : "false");
        DebugLog(ss.str().c_str());
        ss.str("");

        ss << "  buildState->constructionProgress: " << buildState->constructionProgress;
        DebugLog(ss.str().c_str());
        ss.str("");
    }
    else
    {
        DebugLog("  buildState: nullptr");
    }

    DebugLog("=== End Building Dump ===");
}

// Main loop hook: check if a player-owned building is selected
void (*GameWorld_mainLoop_orig)(GameWorld *thisptr, float time);
void GameWorld_mainLoop_hook(GameWorld *thisptr, float time)
{
    if (gShowRenameWindowButton != nullptr)
    {
        Building *building = ou->player->selectedObject.getBuilding();

        // Verbose debug: dump building info on new selection (any ownership, any class)
        if (kVerboseDebugLogging)
        {
            static Building *sPreviousBuilding = nullptr;
            if (building != nullptr && building != sPreviousBuilding) { DumpBuildingInfo(building); }
            sPreviousBuilding = building;
        }

        if (building != nullptr && building->isThePlayer()) { gShowRenameWindowButton->setVisible(true); }
        else
        {
            gShowRenameWindowButton->setVisible(false);
            gRenameWindow->setVisible(false);
        }
    }

    GameWorld_mainLoop_orig(thisptr, time);
}

// Title screen constructor hook
TitleScreen *(*TitleScreen_orig)(TitleScreen *) = nullptr;
TitleScreen *TitleScreen_hook(TitleScreen *thisptr)
{
    // Call original constructor
    TitleScreen *titleScreen = TitleScreen_orig(thisptr);

    MyGUI::Gui *gui = MyGUI::Gui::getInstancePtr();

    const float kRenameWindowX = 0.25F;
    const float kRenameWindowY = 0.35F;
    const float kRenameWindowWidth = 0.30F;
    const float kRenameWindowHeight = 0.072F;
    const float kEditBoxX = 0.05F;
    const float kEditBoxY = 0.10F;
    const float kEditBoxWidth = 0.68F;
    const float kEditBoxHeight = 0.70F;
    const float kConfirmButtonX = 0.75F;
    const float kConfirmButtonY = 0.10F;
    const float kConfirmButtonWidth = 0.19F;
    const float kConfirmButtonHeight = 0.70F;
    const float kShowButtonX = 0.01F;
    const float kShowButtonY = 0.75F;
    const float kShowButtonWidth = 0.06F;
    const float kShowButtonHeight = 0.02F;

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