#pragma once

#include <kenshi/Building/Building.h>

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
    bool foundEnabled;
    bool invalidEnabled;
    bool foundButtonX;
    bool invalidButtonX;
    bool clampedButtonX;
    bool foundButtonY;
    bool invalidButtonY;
    bool clampedButtonY;
    bool foundAllowRenamingNonPlayerBuildings;
    bool invalidAllowRenamingNonPlayerBuildings;
    bool foundAllowSelectiveRenames;
    bool invalidAllowSelectiveRenames;
    bool foundAllowFluff;
    bool invalidAllowFluff;
    bool foundAllowUsable;
    bool invalidAllowUsable;
    bool foundAllowStorage;
    bool invalidAllowStorage;
    bool foundAllowProduction;
    bool invalidAllowProduction;
    bool foundAllowResearch;
    bool invalidAllowResearch;
    bool foundAllowCrafting;
    bool invalidAllowCrafting;
    bool foundAllowGateway;
    bool invalidAllowGateway;
    bool foundAllowTurret;
    bool invalidAllowTurret;
    bool foundAllowWall;
    bool invalidAllowWall;
    bool foundAllowItemFurnace;
    bool invalidAllowItemFurnace;
    bool foundAllowLight;
    bool invalidAllowLight;
    bool foundAllowShellWithInterior;
    bool invalidAllowShellWithInterior;
    bool foundAllowFarm;
    bool invalidAllowFarm;
    bool foundVerboseDebugLogging;
    bool invalidVerboseDebugLogging;
    bool foundDeveloperDebug;
    bool invalidDeveloperDebug;
    bool syntaxError;
    size_t syntaxErrorOffset;
};