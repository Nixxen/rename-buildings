// -----------------------------------------------------------------------
// RenameBuildingsUI.inl. Rename window UI and drag handling
// Included inline by RenameBuildings.cpp
// -----------------------------------------------------------------------

// Global UI widgets
MyGUI::Window *gRenameWindow = nullptr;
MyGUI::Button *gShowRenameWindowButton = nullptr;
bool gDragging = false;
int gDragStartX = 0;
int gDragStartY = 0;
int gButtonStartX = 0;
int gButtonStartY = 0;

// Prevents the rename window from opening on the first click after a drag
static bool gWasDragged = false;

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
    if (mouseButtonId == MyGUI::MouseButton::Left)
    {
        gDragging = false;

        MyGUI::IntPoint pos = sender->getPosition();
        MyGUI::IntSize parentSize = sender->getParentSize();
        if (parentSize.width <= 0 || parentSize.height <= 0) { return; }

        float newX = static_cast<float>(pos.left) / static_cast<float>(parentSize.width);
        float newY = static_cast<float>(pos.top) / static_cast<float>(parentSize.height);

        float deltaX = newX - gConfig.buttonX;
        float deltaY = newY - gConfig.buttonY;
        float distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));

        if (distance < kMinimumDragDistance)
        {
            // Treat as a click, not a drag: revert to saved position
            sender->setPosition(
                static_cast<int>(gConfig.buttonX * static_cast<float>(parentSize.width)),
                static_cast<int>(gConfig.buttonY * static_cast<float>(parentSize.height))
            );
            return;
        }

        gWasDragged = true;
        gConfig.buttonX = newX;
        gConfig.buttonY = newY;
        SaveConfigState();
    }
}