// -----------------------------------------------------------------------
// RenameBuildingsModHub.inl. Emkej's Mod Core (Mod Hub) integration
// Included inline by RenameBuildings.cpp
// For more information on the Mod Hub SDK, see:
// https://github.com/Emkej/Emkejs-Mod-Core/blob/main/docs/mod-hub-sdk.md
// -----------------------------------------------------------------------

#include "emc/mod_hub_client.h"

#include <sstream>

namespace
{
const char *kHubNamespaceId = "tools";
const char *kHubNamespaceDisplayName = "Tools";
const char *kHubModId = "rename_buildings";
const char *kHubModDisplayName = "Rename Buildings";

typedef bool RenameBuildingsConfig::*ConfigBoolField;
typedef float RenameBuildingsConfig::*ConfigFloatField;

emc::ModHubClient gModHubClient;
bool gModHubClientConfigured = false;

void WriteHubErrorText(char *err_buf, uint32_t err_buf_size, const char *text)
{
    if (err_buf == nullptr || err_buf_size == 0U) { return; }

    if (text == nullptr)
    {
        err_buf[0] = '\0';
        return;
    }

    uint32_t index = 0U;
    while (index + 1U < err_buf_size && text[index] != '\0')
    {
        err_buf[index] = text[index];
        ++index;
    }
    err_buf[index] = '\0';
}

bool IsValidHubUserData(void *user_data) { return user_data == &gModHubClient; }

// -----------------------------------------------------------------------
// Generic get/set helpers (pointer-to-member dispatch)
// -----------------------------------------------------------------------

EMC_Result GetHubBoolSetting(void *user_data, int32_t *out_value, ConfigBoolField field)
{
    if (!IsValidHubUserData(user_data) || out_value == nullptr) { return EMC_ERR_INVALID_ARGUMENT; }
    *out_value = (gConfig.*field) ? 1 : 0;
    return EMC_OK;
}

EMC_Result
SetHubBoolSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size, ConfigBoolField field)
{
    if (!IsValidHubUserData(user_data))
    {
        WriteHubErrorText(err_buf, err_buf_size, "invalid_user_data");
        return EMC_ERR_INVALID_ARGUMENT;
    }

    if (value != 0 && value != 1)
    {
        WriteHubErrorText(err_buf, err_buf_size, "invalid_bool");
        return EMC_ERR_INVALID_ARGUMENT;
    }

    const RenameBuildingsConfig previous = gConfig;
    RenameBuildingsConfig updated = previous;
    updated.*field = value != 0;

    gConfig = updated;
    if (!SaveConfigState())
    {
        gConfig = previous;
        WriteHubErrorText(err_buf, err_buf_size, "persist_failed");
        return EMC_ERR_INTERNAL;
    }

    gConfigNeedsWriteBack = false;
    WriteHubErrorText(err_buf, err_buf_size, nullptr);
    return EMC_OK;
}

EMC_Result GetHubFloatSetting(void *user_data, float *out_value, ConfigFloatField field)
{
    if (!IsValidHubUserData(user_data) || out_value == nullptr) { return EMC_ERR_INVALID_ARGUMENT; }
    *out_value = gConfig.*field;
    return EMC_OK;
}

EMC_Result
SetHubFloatSetting(void *user_data, float value, char *err_buf, uint32_t err_buf_size, ConfigFloatField field)
{
    if (!IsValidHubUserData(user_data))
    {
        WriteHubErrorText(err_buf, err_buf_size, "invalid_user_data");
        return EMC_ERR_INVALID_ARGUMENT;
    }

    const RenameBuildingsConfig previous = gConfig;
    RenameBuildingsConfig updated = previous;
    updated.*field = value;

    gConfig = updated;
    if (!SaveConfigState())
    {
        gConfig = previous;
        WriteHubErrorText(err_buf, err_buf_size, "persist_failed");
        return EMC_ERR_INTERNAL;
    }

    gConfigNeedsWriteBack = false;
    WriteHubErrorText(err_buf, err_buf_size, nullptr);
    return EMC_OK;
}

// -----------------------------------------------------------------------
// Generic get/set helpers for classRenameable[] (indexed by BuildingClassType)
// -----------------------------------------------------------------------

EMC_Result GetHubClassRenameableSetting(void *user_data, int32_t *out_value, BuildingClassType classType)
{
    if (!IsValidHubUserData(user_data) || out_value == nullptr) { return EMC_ERR_INVALID_ARGUMENT; }
    *out_value = gConfig.classRenameable[classType] ? 1 : 0;
    return EMC_OK;
}

EMC_Result SetHubClassRenameableSetting(
    void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size, BuildingClassType classType
)
{
    if (!IsValidHubUserData(user_data))
    {
        WriteHubErrorText(err_buf, err_buf_size, "invalid_user_data");
        return EMC_ERR_INVALID_ARGUMENT;
    }

    if (value != 0 && value != 1)
    {
        WriteHubErrorText(err_buf, err_buf_size, "invalid_bool");
        return EMC_ERR_INVALID_ARGUMENT;
    }

    const RenameBuildingsConfig previous = gConfig;
    RenameBuildingsConfig updated = previous;
    updated.classRenameable[classType] = value != 0;

    gConfig = updated;
    if (!SaveConfigState())
    {
        gConfig = previous;
        WriteHubErrorText(err_buf, err_buf_size, "persist_failed");
        return EMC_ERR_INTERNAL;
    }

    gConfigNeedsWriteBack = false;
    WriteHubErrorText(err_buf, err_buf_size, nullptr);
    return EMC_OK;
}

// -----------------------------------------------------------------------
// Per-setting callbacks: bool (main)
// -----------------------------------------------------------------------

EMC_Result __cdecl GetEnabledSetting(void *user_data, int32_t *out_value)
{ return GetHubBoolSetting(user_data, out_value, &RenameBuildingsConfig::enabled); }

EMC_Result __cdecl SetEnabledSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubBoolSetting(user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::enabled); }

// -----------------------------------------------------------------------
// Per-setting callbacks: float (main)
// -----------------------------------------------------------------------

EMC_Result __cdecl GetButtonXSetting(void *user_data, float *out_value)
{ return GetHubFloatSetting(user_data, out_value, &RenameBuildingsConfig::buttonX); }

EMC_Result __cdecl SetButtonXSetting(void *user_data, float value, char *err_buf, uint32_t err_buf_size)
{ return SetHubFloatSetting(user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::buttonX); }

EMC_Result __cdecl GetButtonYSetting(void *user_data, float *out_value)
{ return GetHubFloatSetting(user_data, out_value, &RenameBuildingsConfig::buttonY); }

EMC_Result __cdecl SetButtonYSetting(void *user_data, float value, char *err_buf, uint32_t err_buf_size)
{ return SetHubFloatSetting(user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::buttonY); }

// -----------------------------------------------------------------------
// Per-setting callbacks: bool (advanced)
// -----------------------------------------------------------------------

EMC_Result __cdecl GetAllowRenamingNonPlayerBuildingsSetting(void *user_data, int32_t *out_value)
{ return GetHubBoolSetting(user_data, out_value, &RenameBuildingsConfig::allowRenamingNonPlayerBuildings); }

EMC_Result __cdecl
SetAllowRenamingNonPlayerBuildingsSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{
    return SetHubBoolSetting(
        user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::allowRenamingNonPlayerBuildings
    );
}

EMC_Result __cdecl GetAllowSelectiveRenamesSetting(void *user_data, int32_t *out_value)
{ return GetHubBoolSetting(user_data, out_value, &RenameBuildingsConfig::allowSelectiveRenames); }

EMC_Result __cdecl SetAllowSelectiveRenamesSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubBoolSetting(user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::allowSelectiveRenames); }

// -----------------------------------------------------------------------
// Per-setting callbacks: classRenameable[] (advanced)
// -----------------------------------------------------------------------

EMC_Result __cdecl GetAllowRenameFluffSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_FLUFF); }

EMC_Result __cdecl SetAllowRenameFluffSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_FLUFF); }

EMC_Result __cdecl GetAllowRenameUsableSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_USABLE); }

EMC_Result __cdecl SetAllowRenameUsableSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_USABLE); }

EMC_Result __cdecl GetAllowRenameStorageSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_STORAGE); }

EMC_Result __cdecl SetAllowRenameStorageSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_STORAGE); }

EMC_Result __cdecl GetAllowRenameProductionSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_PRODUCTION); }

EMC_Result __cdecl SetAllowRenameProductionSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_PRODUCTION); }

EMC_Result __cdecl GetAllowRenameResearchSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_RESEARCH); }

EMC_Result __cdecl SetAllowRenameResearchSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_RESEARCH); }

EMC_Result __cdecl GetAllowRenameCraftingSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_CRAFTING); }

EMC_Result __cdecl SetAllowRenameCraftingSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_CRAFTING); }

EMC_Result __cdecl GetAllowRenameGatewaySetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_GATEWAY); }

EMC_Result __cdecl SetAllowRenameGatewaySetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_GATEWAY); }

EMC_Result __cdecl GetAllowRenameTurretSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_TURRET); }

EMC_Result __cdecl SetAllowRenameTurretSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_TURRET); }

EMC_Result __cdecl GetAllowRenameWallSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_WALL); }

EMC_Result __cdecl SetAllowRenameWallSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_WALL); }

EMC_Result __cdecl GetAllowRenameItemFurnaceSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_ITEM_FURNACE); }

EMC_Result __cdecl
SetAllowRenameItemFurnaceSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_ITEM_FURNACE); }

EMC_Result __cdecl GetAllowRenameLightSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_LIGHT); }

EMC_Result __cdecl SetAllowRenameLightSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_LIGHT); }

EMC_Result __cdecl GetAllowRenameShellWithInteriorSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_SHELL_WITH_INTERIOR); }

EMC_Result __cdecl
SetAllowRenameShellWithInteriorSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_SHELL_WITH_INTERIOR); }

EMC_Result __cdecl GetAllowRenameFarmSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, BCTYPE_FARM); }

EMC_Result __cdecl SetAllowRenameFarmSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, BCTYPE_FARM); }

// -----------------------------------------------------------------------
// Per-setting callbacks: bool (debug)
// -----------------------------------------------------------------------

EMC_Result __cdecl GetVerboseDebugLoggingSetting(void *user_data, int32_t *out_value)
{ return GetHubBoolSetting(user_data, out_value, &RenameBuildingsConfig::verboseDebugLogging); }

EMC_Result __cdecl SetVerboseDebugLoggingSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubBoolSetting(user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::verboseDebugLogging); }

EMC_Result __cdecl GetDeveloperDebugSetting(void *user_data, int32_t *out_value)
{ return GetHubBoolSetting(user_data, out_value, &RenameBuildingsConfig::developerDebug); }

EMC_Result __cdecl SetDeveloperDebugSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubBoolSetting(user_data, value, err_buf, err_buf_size, &RenameBuildingsConfig::developerDebug); }

// -----------------------------------------------------------------------
// Client configuration (static const, matching FixShoppingWages pattern)
// -----------------------------------------------------------------------

void EnsureModHubClientConfigured()
{
    if (gModHubClientConfigured) { return; }

    static const EMC_ModDescriptorV1 kModHubDescriptor = {
        kHubNamespaceId, kHubNamespaceDisplayName, kHubModId, kHubModDisplayName, &gModHubClient
    };

    static const EMC_BoolSettingDefV1 kEnabledSetting = {"enabled",      "Enabled",          "Enable the mod",
                                                         &gModHubClient, &GetEnabledSetting, &SetEnabledSetting};

    static const EMC_FloatSettingDefV1 kButtonXSetting = {
        "button_x",
        "Button X",
        "Horizontal position of the rename button (0.0-1.0)",
        &gModHubClient,
        0.0F,
        1.0F,
        0.001F,
        EMC_FLOAT_DISPLAY_DECIMALS_DEFAULT,
        &GetButtonXSetting,
        &SetButtonXSetting
    };

    static const EMC_FloatSettingDefV1 kButtonYSetting = {
        "button_y",
        "Button Y",
        "Vertical position of the rename button (0.0-1.0)",
        &gModHubClient,
        0.0F,
        1.0F,
        0.001F,
        EMC_FLOAT_DISPLAY_DECIMALS_DEFAULT,
        &GetButtonYSetting,
        &SetButtonYSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenamingNonPlayerBuildingsSetting = {
        "allow_renaming_non_player_buildings",
        "Allow non-player buildings",
        "Allow renaming buildings that are not owned by the player",
        &gModHubClient,
        &GetAllowRenamingNonPlayerBuildingsSetting,
        &SetAllowRenamingNonPlayerBuildingsSetting
    };

    static const EMC_BoolSettingDefV1 kAllowSelectiveRenamesSetting = {
        "allow_selective_renames",
        "Allow selective renames",
        "Enable per-building-class rename filtering",
        &gModHubClient,
        &GetAllowSelectiveRenamesSetting,
        &SetAllowSelectiveRenamesSetting
    };

    // Condition rules: hide all class flags when allow_selective_renames is false
    static const EMC_BoolConditionRuleDefV1 kSelectiveRenameConditionRules[] = {
        {"allow_rename_fluff", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_usable", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_storage", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_production", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_research", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_crafting", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_gateway", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_turret", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_wall", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_item_furnace", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_light", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_shell_with_interior", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0},
        {"allow_rename_farm", "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0}
    };

    static const EMC_BoolSettingDefV1 kAllowRenameFluffSetting = {
        "allow_rename_fluff", "Allow rename Fluff",        "Allow renaming Fluff buildings",
        &gModHubClient,       &GetAllowRenameFluffSetting, &SetAllowRenameFluffSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameUsableSetting = {
        "allow_rename_usable", "Allow rename Usable",        "Allow renaming Usable buildings",
        &gModHubClient,        &GetAllowRenameUsableSetting, &SetAllowRenameUsableSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameStorageSetting = {
        "allow_rename_storage", "Allow rename Storage",        "Allow renaming Storage buildings",
        &gModHubClient,         &GetAllowRenameStorageSetting, &SetAllowRenameStorageSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameProductionSetting = {
        "allow_rename_production",
        "Allow rename Production",
        "Allow renaming Production buildings",
        &gModHubClient,
        &GetAllowRenameProductionSetting,
        &SetAllowRenameProductionSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameResearchSetting = {
        "allow_rename_research", "Allow rename Research",        "Allow renaming Research buildings",
        &gModHubClient,          &GetAllowRenameResearchSetting, &SetAllowRenameResearchSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameCraftingSetting = {
        "allow_rename_crafting", "Allow rename Crafting",        "Allow renaming Crafting buildings",
        &gModHubClient,          &GetAllowRenameCraftingSetting, &SetAllowRenameCraftingSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameGatewaySetting = {
        "allow_rename_gateway", "Allow rename Gateway",        "Allow renaming Gateway buildings",
        &gModHubClient,         &GetAllowRenameGatewaySetting, &SetAllowRenameGatewaySetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameTurretSetting = {
        "allow_rename_turret", "Allow rename Turret",        "Allow renaming Turret buildings",
        &gModHubClient,        &GetAllowRenameTurretSetting, &SetAllowRenameTurretSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameWallSetting = {
        "allow_rename_wall", "Allow rename Wall",        "Allow renaming Wall buildings",
        &gModHubClient,      &GetAllowRenameWallSetting, &SetAllowRenameWallSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameItemFurnaceSetting = {
        "allow_rename_item_furnace",
        "Allow rename Item Furnace",
        "Allow renaming Item Furnace buildings",
        &gModHubClient,
        &GetAllowRenameItemFurnaceSetting,
        &SetAllowRenameItemFurnaceSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameLightSetting = {
        "allow_rename_light", "Allow rename Light",        "Allow renaming Light buildings",
        &gModHubClient,       &GetAllowRenameLightSetting, &SetAllowRenameLightSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameShellWithInteriorSetting = {
        "allow_rename_shell_with_interior",
        "Allow rename Shell with Interior",
        "Allow renaming Shell with Interior buildings",
        &gModHubClient,
        &GetAllowRenameShellWithInteriorSetting,
        &SetAllowRenameShellWithInteriorSetting
    };

    static const EMC_BoolSettingDefV1 kAllowRenameFarmSetting = {
        "allow_rename_farm", "Allow rename Farm",        "Allow renaming Farm buildings",
        &gModHubClient,      &GetAllowRenameFarmSetting, &SetAllowRenameFarmSetting
    };

    static const EMC_BoolSettingDefV1 kVerboseDebugLoggingSetting = {
        "verbose_debug_logging",
        "Verbose debug",
        "Enable excessively verbose diagnostic logging",
        &gModHubClient,
        &GetVerboseDebugLoggingSetting,
        &SetVerboseDebugLoggingSetting
    };

    static const EMC_BoolSettingDefV1 kDeveloperDebugSetting = {
        "developer_debug", "Developer hotkeys",       "Enable CTRL+T debug hotkey",
        &gModHubClient,    &GetDeveloperDebugSetting, &SetDeveloperDebugSetting
    };

    static const char *kSectionAdvancedId = "overrides";
    static const char *kSectionAdvancedLabel = "Overrides";
    static const char *kSectionDebugId = "advanced";
    static const char *kSectionDebugLabel = "Debug";

    static const emc::ModHubClientSettingRowV1 kModHubRows[] = {
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "enabled", &kEnabledSetting, nullptr, nullptr},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_FLOAT, "button_x", &kButtonXSetting, nullptr, nullptr},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_FLOAT, "button_y", &kButtonYSetting, nullptr, nullptr},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_renaming_non_player_buildings",
         &kAllowRenamingNonPlayerBuildingsSetting, kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_selective_renames", &kAllowSelectiveRenamesSetting,
         kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_fluff", &kAllowRenameFluffSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_usable", &kAllowRenameUsableSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_storage", &kAllowRenameStorageSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_production", &kAllowRenameProductionSetting,
         kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_research", &kAllowRenameResearchSetting,
         kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_crafting", &kAllowRenameCraftingSetting,
         kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_gateway", &kAllowRenameGatewaySetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_turret", &kAllowRenameTurretSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_wall", &kAllowRenameWallSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_item_furnace", &kAllowRenameItemFurnaceSetting,
         kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_light", &kAllowRenameLightSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_shell_with_interior",
         &kAllowRenameShellWithInteriorSetting, kSectionAdvancedId, kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "allow_rename_farm", &kAllowRenameFarmSetting, kSectionAdvancedId,
         kSectionAdvancedLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "verbose_debug_logging", &kVerboseDebugLoggingSetting, kSectionDebugId,
         kSectionDebugLabel},
        {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, "developer_debug", &kDeveloperDebugSetting, kSectionDebugId,
         kSectionDebugLabel}
    };

    static const emc::ModHubClientTableRegistrationV1 kModHubRegistration = {
        &kModHubDescriptor, kModHubRows, static_cast<uint32_t>(sizeof(kModHubRows) / sizeof(kModHubRows[0])),
        kSelectiveRenameConditionRules,
        static_cast<uint32_t>(sizeof(kSelectiveRenameConditionRules) / sizeof(kSelectiveRenameConditionRules[0]))
    };

    emc::ModHubClient::Config config;
    config.table_registration = &kModHubRegistration;
    gModHubClient.SetConfig(config);
    gModHubClientConfigured = true;
}
} // namespace

void RenameBuildingsModHub_OnStartup()
{
    EnsureModHubClientConfigured();

    const emc::ModHubClient::AttemptResult result = gModHubClient.OnStartup();
    if (result == emc::ModHubClient::ATTACH_SUCCESS)
    {
        DebugLog("INFO: event=mod_hub_attached use_hub_ui=1");
        return;
    }

    if (result == emc::ModHubClient::ATTACH_FAILED)
    {
        if (gModHubClient.IsAttachRetryPending())
        {
            DebugLog("INFO: event=mod_hub_attach_retry_pending use_hub_ui=0");
            return;
        }

        std::stringstream line;
        line << "WARN: event=mod_hub_fallback reason=get_api_failed"
             << " result=" << gModHubClient.LastAttemptFailureResult() << " use_hub_ui=0";
        ErrorLog(line.str().c_str());
        return;
    }

    if (result == emc::ModHubClient::REGISTRATION_FAILED)
    {
        std::stringstream line;
        line << "WARN: event=mod_hub_fallback reason=register_mod_or_setting_failed"
             << " result=" << gModHubClient.LastAttemptFailureResult() << " use_hub_ui=0";
        ErrorLog(line.str().c_str());
        return;
    }

    ErrorLog("WARN: event=mod_hub_fallback reason=invalid_client_configuration use_hub_ui=0");
}