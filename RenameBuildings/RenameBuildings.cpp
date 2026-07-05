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

// UI callback: attempt to rename the currently selected building
void OnRenameButtonPress(MyGUI::WidgetPtr sender)
{
    DebugLog("------- BUTTON PRESS WORKS! --------");
    MyGUI::EditBox *edit = dynamic_cast<MyGUI::EditBox *>(sender->getParent()->findWidget("RenameEdit"));
    if (!edit)
    {
        DebugLog("RenameBuildings: Edit box not found");
        return;
    }

    std::string newName = edit->getCaption();
    if (newName.empty())
    {
        DebugLog("RenameBuildings: New name is empty");
        return;
    }

    Building* building = ou->player->selectedObject.getBuilding();
    if (building)
    {
        if (building->isThePlayer())
        {
            building->setName(newName);
            DebugLog(("RenameBuildings: Renamed to " + newName).c_str());
        }
        else
        {
            DebugLog("RenameBuildings: Building is not player-owned");
        }
    }
    else
    {
        DebugLog("RenameBuildings: No building selected");
    }
}

// Title screen constructor hook
TitleScreen* (*TitleScreen_orig)(TitleScreen*) = NULL;
TitleScreen* TitleScreen_hook(TitleScreen* thisptr)
{
    // Call original constructor
    TitleScreen* titleScreen = TitleScreen_orig(thisptr);

    // create UI: window with edit box and rename button
    MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
    MyGUI::Window* window = gui->createWidgetReal<MyGUI::Window>("Kenshi_WindowCX", 0.25f, 0.25f, 0.30f, 0.18f, MyGUI::Align::Center, "Window", "RenameWindow");
    window->setCaption("Rename Building");

    MyGUI::EditBox* edit = window->getClientWidget()->createWidgetReal<MyGUI::EditBox>("Kenshi_EditBox", 0.05f, 0.15f, 0.6f, 0.7f, MyGUI::Align::Default, "RenameEdit");
    edit->setCaption("");

    MyGUI::Button* renameButton = window->getClientWidget()->createWidgetReal<MyGUI::Button>("Kenshi_Button1", 0.68f, 0.15f, 0.27f, 0.7f, MyGUI::Align::Center, "RenameButton");
    renameButton->setCaption("Rename");
    renameButton->eventMouseButtonClick += MyGUI::newDelegate(OnRenameButtonPress);

    return titleScreen;
}

__declspec(dllexport) void startPlugin()
{
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&TitleScreen::_CONSTRUCTOR), TitleScreen_hook, &TitleScreen_orig))
    {
        ErrorLog("RenameBuildings: Could not add hook!");
    }
    else
    {
        DebugLog("RenameBuildings: Hook installed");
    }
}