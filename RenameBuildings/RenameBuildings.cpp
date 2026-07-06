#include <Debug.h>

#include <kenshi/gui/TitleScreen.h>
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/Building/Building.h>

#include <mygui/MyGUI_Gui.h>
#include <mygui/MyGUI_Window.h>
#include <mygui/MyGUI_Button.h>
#include <mygui/MyGUI_EditBox.h>
#include <mygui/MyGUI_Delegate.h>

#include <core/Functions.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Global UI widgets
MyGUI::Window *g_rename_window = nullptr;
MyGUI::Button *g_show_rename_window_button = nullptr;
bool g_dragging = false;
int g_drag_start_x = 0;
int g_drag_start_y = 0;
int g_button_start_x = 0;
int g_button_start_y = 0;

// UI callback: attempt to rename the currently selected building
void OnRenameButtonPress(MyGUI::WidgetPtr sender)
{
    MyGUI::EditBox *edit = dynamic_cast<MyGUI::EditBox *>(
        g_rename_window->findWidget("RenameBuildingEdit"));
    if (!edit)
    {
        DebugLog("Edit box not found");
        return;
    }

    std::string new_name = edit->getCaption();
    // Trim whitespace from both ends
    size_t start = new_name.find_first_not_of(" \t\r\n");
    size_t end = new_name.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        new_name.clear();
    }
    else
    {
        new_name = new_name.substr(start, end - start + 1);
    }
    if (new_name.empty())
    {
        DebugLog("New name is empty or whitespace only");
        return;
    }

    Building *building = ou->player->selectedObject.getBuilding();
    if (building)
    {
        if (building->isThePlayer())
        {
            building->setName(new_name);
            DebugLog(("Renamed to " + new_name).c_str());
            g_rename_window->setVisible(false);
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

void OnCloseRenameWindow(MyGUI::Window *sender, const std::string &name)
{
    g_rename_window->setVisible(false);
}

// UI callback: show the rename window, pre-filled with current building name
// If the window is already visible, close it instead
void OnShowRenameWindow(MyGUI::WidgetPtr sender)
{
    if (g_rename_window->getVisible())
    {
        g_rename_window->setVisible(false);
        return;
    }

    Building *building = ou->player->selectedObject.getBuilding();
    if (building && building->isThePlayer())
    {
        DebugLog("Showing rename window");
        MyGUI::EditBox *edit = dynamic_cast<MyGUI::EditBox *>(
            g_rename_window->findWidget("RenameBuildingEdit"));
        if (edit)
        {
            edit->setCaption(building->getName());
        }
        g_rename_window->setVisible(true);
    }
}

// Drag start: record button position and mouse offset at press
void OnRenameButtonMousePress(
    MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton id)
{
    if (id == MyGUI::MouseButton::Left)
    {
        g_dragging = true;
        MyGUI::IntPoint pos = sender->getPosition();
        g_button_start_x = pos.left;
        g_button_start_y = pos.top;
        g_drag_start_x = left;
        g_drag_start_y = top;
    }
}

// Drag move: reposition button with mouse delta, clamped to screen
void OnRenameButtonMouseDrag(
    MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton id)
{
    if (g_dragging && id == MyGUI::MouseButton::Left)
    {
        int delta_x = left - g_drag_start_x;
        int delta_y = top - g_drag_start_y;
        int new_left = g_button_start_x + delta_x;
        int new_top = g_button_start_y + delta_y;

        // Clamp to parent (screen) edges
        MyGUI::IntSize parent_size = sender->getParentSize();
        MyGUI::IntSize button_size = sender->getSize();
        if (new_left < 0)
        {
            new_left = 0;
        }
        if (new_top < 0)
        {
            new_top = 0;
        }
        if (new_left + button_size.width > parent_size.width)
        {
            new_left = parent_size.width - button_size.width;
        }
        if (new_top + button_size.height > parent_size.height)
        {
            new_top = parent_size.height - button_size.height;
        }

        sender->setPosition(new_left, new_top);
    }
}

// Drag end
void OnRenameButtonMouseRelease(
    MyGUI::WidgetPtr sender, int left, int top, MyGUI::MouseButton id)
{
    if (id == MyGUI::MouseButton::Left)
    {
        g_dragging = false;
    }
}

// Main loop hook: check if a player-owned building is selected
void (*GameWorld_main_loop_orig)(GameWorld *thisptr, float time);
void GameWorld_main_loop_hook(GameWorld *thisptr, float time)
{
    if (g_show_rename_window_button)
    {
        Building *building = ou->player->selectedObject.getBuilding();
        if (building && building->isThePlayer())
        {
            g_show_rename_window_button->setVisible(true);
        }
        else
        {
            g_show_rename_window_button->setVisible(false);
            g_rename_window->setVisible(false);
        }
    }

    GameWorld_main_loop_orig(thisptr, time);
}

// Title screen constructor hook
TitleScreen *(*TitleScreen_orig)(TitleScreen *) = NULL;
TitleScreen *TitleScreen_hook(TitleScreen *thisptr)
{
    // Call original constructor
    TitleScreen *titleScreen = TitleScreen_orig(thisptr);

    MyGUI::Gui *gui = MyGUI::Gui::getInstancePtr();

    // Create rename window (hidden initially)
    g_rename_window = gui->createWidgetReal<MyGUI::Window>(
        "Kenshi_WindowCX", 0.25f, 0.35f, 0.30f, 0.072f,
        MyGUI::Align::Center, "Window", "RenameBuildingWindow");
    g_rename_window->setCaption("Rename Building");
    g_rename_window->setVisible(false);
    g_rename_window->eventWindowButtonPressed +=
        MyGUI::newDelegate(OnCloseRenameWindow);

    MyGUI::EditBox *edit =
        g_rename_window->getClientWidget()->createWidgetReal<MyGUI::EditBox>(
            "Kenshi_EditBox", 0.05f, 0.1f, 0.68f, 0.7f,
            MyGUI::Align::Default, "RenameBuildingEdit");
    edit->setCaption("");

    MyGUI::Button *confirm_rename_button =
        g_rename_window->getClientWidget()->createWidgetReal<MyGUI::Button>(
            "Kenshi_Button1", 0.75f, 0.1f, 0.19f, 0.7f,
            MyGUI::Align::Center, "ConfirmRenameBuildingButton");
    confirm_rename_button->setCaption("Rename");
    confirm_rename_button->eventMouseButtonClick += MyGUI::newDelegate(OnRenameButtonPress);

    // Create the "Rename" selection button (hidden initially)
    // Shows the main rename window when clicked, and can be dragged around the screen
    g_show_rename_window_button = gui->createWidgetReal<MyGUI::Button>(
        "Kenshi_Button1", 0.01f, 0.75f, 0.06f, 0.02f,
        MyGUI::Align::Default, "Window", "ShowRenameBuildingWindowButton");
    g_show_rename_window_button->setCaption("Rename");
    g_show_rename_window_button->setVisible(false);
    g_show_rename_window_button->eventMouseButtonClick +=
        MyGUI::newDelegate(OnShowRenameWindow);
    g_show_rename_window_button->eventMouseButtonPressed +=
        MyGUI::newDelegate(OnRenameButtonMousePress);
    g_show_rename_window_button->eventMouseDrag +=
        MyGUI::newDelegate(OnRenameButtonMouseDrag);
    g_show_rename_window_button->eventMouseButtonReleased +=
        MyGUI::newDelegate(OnRenameButtonMouseRelease);

    return titleScreen;
}

__declspec(dllexport) void startPlugin()
{
    if (KenshiLib::SUCCESS !=
        KenshiLib::AddHook(
            KenshiLib::GetRealAddress(&TitleScreen::_CONSTRUCTOR),
            TitleScreen_hook, &TitleScreen_orig))
    {
        ErrorLog("Could not add hook!");
    }
    else
    {
        DebugLog("Hook installed");
    }

    if (KenshiLib::SUCCESS !=
        KenshiLib::AddHook(
            KenshiLib::GetRealAddress(
                &GameWorld::_NV_mainLoop_GPUSensitiveStuff),
            GameWorld_main_loop_hook, &GameWorld_main_loop_orig))
    {
        ErrorLog("Could not add main loop hook!");
    }
    else
    {
        DebugLog("Main loop hook installed");
    }
}