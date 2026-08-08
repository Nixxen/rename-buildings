// -----------------------------------------------------------------------
// RenameBuildingsDebug.inl. Diagnostic and verbose debug helpers
// Included inline by RenameBuildings.cpp
// -----------------------------------------------------------------------

// Ctrl+T debug hotkey edge-detection state
static bool gCtrlTPressedLast = false;

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