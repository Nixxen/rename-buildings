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

// Global UI widgets
MyGUI::Window *gRenameWindow = nullptr;
MyGUI::Button *gShowRenameWindowButton = nullptr;
bool gDragging = false;
int gDragStartX = 0;
int gDragStartY = 0;
int gButtonStartX = 0;
int gButtonStartY = 0;

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

    Building *building = ou->player->selectedObject.getBuilding();
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

    Building *building = ou->player->selectedObject.getBuilding();
    if (building != nullptr && building->isThePlayer() && building->getBuildingClass() != BCTYPE_DOOR)
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

// Main loop hook: check if a player-owned building is selected
void (*GameWorld_mainLoop_orig)(GameWorld *thisptr, float time);
void GameWorld_mainLoop_hook(GameWorld *thisptr, float time)
{
    if (gShowRenameWindowButton != nullptr)
    {
        Building *building = ou->player->selectedObject.getBuilding();
        if (building != nullptr && building->isThePlayer() && building->getBuildingClass() != BCTYPE_DOOR)
        {
            gShowRenameWindowButton->setVisible(true);
        }
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