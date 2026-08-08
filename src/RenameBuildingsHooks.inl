// -----------------------------------------------------------------------
// RenameBuildingsHooks.inl. KenshiLib hook wiring
// Included inline by RenameBuildings.cpp
// -----------------------------------------------------------------------

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

        if (IsValidBuildingRename(GetRenameTarget(building))) { gShowRenameWindowButton->setVisible(true); }
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