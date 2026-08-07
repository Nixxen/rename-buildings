// -----------------------------------------------------------------------
// RenameBuildingsModHub.inl. Emkej's Mod Core (Mod Hub) integration
// Included inline by RenameBuildings.cpp
// For more information on the Mod Hub SDK, see:
// https://github.com/Emkej/Emkejs-Mod-Core/blob/main/docs/mod-hub-sdk.md
// -----------------------------------------------------------------------

#include "emc/mod_hub_client.h"

#include <sstream>
#include <vector>

namespace
{
const char *kHubNamespaceId = "ui";
const char *kHubNamespaceDisplayName = "UI";
const char *kHubModId = "rename_buildings";
const char *kHubModDisplayName = "Rename Buildings";

const char *kSectionOverridesId = "overrides";
const char *kSectionOverridesLabel = "Overrides";
const char *kSectionDebugId = "advanced";
const char *kSectionDebugLabel = "Debug";

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
// Config update helper (save/rollback boilerplate)
// -----------------------------------------------------------------------

template <typename ValueType>
EMC_Result
UpdateConfigField(ValueType RenameBuildingsConfig::*field, ValueType value, char *err_buf, uint32_t err_buf_size)
{
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

EMC_Result UpdateConfigClassRenameable(BuildingClassType classType, bool value, char *err_buf, uint32_t err_buf_size)
{
    const RenameBuildingsConfig previous = gConfig;
    RenameBuildingsConfig updated = previous;
    updated.classRenameable[classType] = value;
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
    return UpdateConfigField(field, value != 0, err_buf, err_buf_size);
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
    return UpdateConfigField(field, value, err_buf, err_buf_size);
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
    return UpdateConfigClassRenameable(classType, value != 0, err_buf, err_buf_size);
}

// -----------------------------------------------------------------------
// Template callbacks (one definition per setting type, instantiated per member)
// -----------------------------------------------------------------------

void RepositionRenameButton()
{
    if (gShowRenameWindowButton != nullptr)
    {
        gShowRenameWindowButton->setRealPosition(gConfig.buttonX, gConfig.buttonY);
    }
}

template <bool RenameBuildingsConfig::*Member> EMC_Result __cdecl GetBoolSetting(void *user_data, int32_t *out_value)
{ return GetHubBoolSetting(user_data, out_value, Member); }

template <bool RenameBuildingsConfig::*Member>
EMC_Result __cdecl SetBoolSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubBoolSetting(user_data, value, err_buf, err_buf_size, Member); }

template <float RenameBuildingsConfig::*Member> EMC_Result __cdecl GetFloatSetting(void *user_data, float *out_value)
{ return GetHubFloatSetting(user_data, out_value, Member); }

// NOTE: All current float settings are button positions, so reposition on any float set.
// If a non-position float setting is added later, split this into a parameterized callback.
template <float RenameBuildingsConfig::*Member>
EMC_Result __cdecl SetFloatSetting(void *user_data, float value, char *err_buf, uint32_t err_buf_size)
{
    EMC_Result result = SetHubFloatSetting(user_data, value, err_buf, err_buf_size, Member);
    if (result == EMC_OK) { RepositionRenameButton(); }
    return result;
}

template <BuildingClassType ClassType> EMC_Result __cdecl GetClassRenameableSetting(void *user_data, int32_t *out_value)
{ return GetHubClassRenameableSetting(user_data, out_value, ClassType); }

template <BuildingClassType ClassType>
EMC_Result __cdecl SetClassRenameableSetting(void *user_data, int32_t value, char *err_buf, uint32_t err_buf_size)
{ return SetHubClassRenameableSetting(user_data, value, err_buf, err_buf_size, ClassType); }

typedef EMC_Result(__cdecl *ClassRenameableGetCallback)(void *, int32_t *);
typedef EMC_Result(__cdecl *ClassRenameableSetCallback)(void *, int32_t, char *, uint32_t);

struct ClassRenameableEntry
{
    BuildingClassType type;
    ClassRenameableGetCallback get;
    ClassRenameableSetCallback set;
    const char *id;
    const char *label;
    const char *description;
};

const ClassRenameableEntry kClassRenameableEntries[] = {
    {BCTYPE_FLUFF, &GetClassRenameableSetting<BCTYPE_FLUFF>, &SetClassRenameableSetting<BCTYPE_FLUFF>,
     "allow_rename_fluff", "Allow rename Fluff", "Allow renaming Fluff buildings"},
    {BCTYPE_USABLE, &GetClassRenameableSetting<BCTYPE_USABLE>, &SetClassRenameableSetting<BCTYPE_USABLE>,
     "allow_rename_usable", "Allow rename Usable", "Allow renaming Usable buildings"},
    {BCTYPE_STORAGE, &GetClassRenameableSetting<BCTYPE_STORAGE>, &SetClassRenameableSetting<BCTYPE_STORAGE>,
     "allow_rename_storage", "Allow rename Storage", "Allow renaming Storage buildings"},
    {BCTYPE_PRODUCTION, &GetClassRenameableSetting<BCTYPE_PRODUCTION>, &SetClassRenameableSetting<BCTYPE_PRODUCTION>,
     "allow_rename_production", "Allow rename Production", "Allow renaming Production buildings"},
    {BCTYPE_RESEARCH, &GetClassRenameableSetting<BCTYPE_RESEARCH>, &SetClassRenameableSetting<BCTYPE_RESEARCH>,
     "allow_rename_research", "Allow rename Research", "Allow renaming Research buildings"},
    {BCTYPE_CRAFTING, &GetClassRenameableSetting<BCTYPE_CRAFTING>, &SetClassRenameableSetting<BCTYPE_CRAFTING>,
     "allow_rename_crafting", "Allow rename Crafting", "Allow renaming Crafting buildings"},
    {BCTYPE_GATEWAY, &GetClassRenameableSetting<BCTYPE_GATEWAY>, &SetClassRenameableSetting<BCTYPE_GATEWAY>,
     "allow_rename_gateway", "Allow rename Gateway", "Allow renaming Gateway buildings"},
    {BCTYPE_TURRET, &GetClassRenameableSetting<BCTYPE_TURRET>, &SetClassRenameableSetting<BCTYPE_TURRET>,
     "allow_rename_turret", "Allow rename Turret", "Allow renaming Turret buildings"},
    {BCTYPE_WALL, &GetClassRenameableSetting<BCTYPE_WALL>, &SetClassRenameableSetting<BCTYPE_WALL>, "allow_rename_wall",
     "Allow rename Wall", "Allow renaming Wall buildings"},
    {BCTYPE_ITEM_FURNACE, &GetClassRenameableSetting<BCTYPE_ITEM_FURNACE>,
     &SetClassRenameableSetting<BCTYPE_ITEM_FURNACE>, "allow_rename_item_furnace", "Allow rename Item Furnace",
     "Allow renaming Item Furnace buildings"},
    {BCTYPE_LIGHT, &GetClassRenameableSetting<BCTYPE_LIGHT>, &SetClassRenameableSetting<BCTYPE_LIGHT>,
     "allow_rename_light", "Allow rename Light", "Allow renaming Light buildings"},
    {BCTYPE_SHELL_WITH_INTERIOR, &GetClassRenameableSetting<BCTYPE_SHELL_WITH_INTERIOR>,
     &SetClassRenameableSetting<BCTYPE_SHELL_WITH_INTERIOR>, "allow_rename_shell_with_interior",
     "Allow rename Shell with Interior", "Allow renaming Shell with Interior buildings"},
    {BCTYPE_FARM, &GetClassRenameableSetting<BCTYPE_FARM>, &SetClassRenameableSetting<BCTYPE_FARM>, "allow_rename_farm",
     "Allow rename Farm", "Allow renaming Farm buildings"}
};

// -----------------------------------------------------------------------
// ModHubBuilder: accumulates setting defs, rows, and condition rules
// -----------------------------------------------------------------------

class ModHubBuilder
{
  public:
    ModHubBuilder(
        emc::ModHubClient *client,
        const char *namespaceId,
        const char *namespaceDisplayName,
        const char *modId,
        const char *modDisplayName
    )
        : mClient(client), mCurrentSectionId(nullptr), mCurrentSectionLabel(nullptr), mDescriptor()
    {
        mDescriptor.namespace_id = namespaceId;
        mDescriptor.namespace_display_name = namespaceDisplayName;
        mDescriptor.mod_id = modId;
        mDescriptor.mod_display_name = modDisplayName;
        mDescriptor.mod_user_data = client;
    }

    void BeginSection(const char *id, const char *label)
    {
        mCurrentSectionId = id;
        mCurrentSectionLabel = label;
    }

    void EndSection()
    {
        mCurrentSectionId = nullptr;
        mCurrentSectionLabel = nullptr;
    }

    template <bool RenameBuildingsConfig::*Member>
    void AddBool(
        const char *id,
        const char *label,
        const char *description,
        const char *sectionId = nullptr,
        const char *sectionLabel = nullptr
    )
    {
        const char *secId = sectionId != nullptr ? sectionId : mCurrentSectionId;
        const char *secLabel = sectionLabel != nullptr ? sectionLabel : mCurrentSectionLabel;
        EMC_BoolSettingDefV1 def = {id, label, description, mClient, &GetBoolSetting<Member>, &SetBoolSetting<Member>};
        mBoolDefs.push_back(def);
        RowMeta meta = {emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, id, mBoolDefs.size() - 1, secId, secLabel};
        mRowMeta.push_back(meta);
    }

    template <float RenameBuildingsConfig::*Member>
    void AddFloat(
        const char *id,
        const char *label,
        const char *description,
        float minValue,
        float maxValue,
        float step,
        const char *sectionId = nullptr,
        const char *sectionLabel = nullptr
    )
    {
        const char *secId = sectionId != nullptr ? sectionId : mCurrentSectionId;
        const char *secLabel = sectionLabel != nullptr ? sectionLabel : mCurrentSectionLabel;
        EMC_FloatSettingDefV1 def = {
            id,
            label,
            description,
            mClient,
            minValue,
            maxValue,
            step,
            EMC_FLOAT_DISPLAY_DECIMALS_DEFAULT,
            &GetFloatSetting<Member>,
            &SetFloatSetting<Member>
        };
        mFloatDefs.push_back(def);
        RowMeta meta = {emc::MOD_HUB_CLIENT_SETTING_KIND_FLOAT, id, mFloatDefs.size() - 1, secId, secLabel};
        mRowMeta.push_back(meta);
    }

    void AddClassRenameable(const ClassRenameableEntry &entry)
    {
        EMC_BoolSettingDefV1 def = {entry.id, entry.label, entry.description, mClient, entry.get, entry.set};
        mBoolDefs.push_back(def);
        RowMeta meta = {
            emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL, entry.id, mBoolDefs.size() - 1, kSectionOverridesId,
            kSectionOverridesLabel
        };
        mRowMeta.push_back(meta);

        EMC_BoolConditionRuleDefV1 rule = {entry.id, "allow_selective_renames", EMC_BOOL_CONDITION_EFFECT_HIDE, 0};
        mConditionRules.push_back(rule);
    }

    void Finalize()
    {
        // Build rows after all defs are fully populated so pointers are stable
        mRows.clear();
        mRows.reserve(mRowMeta.size());
        for (size_t i = 0; i < mRowMeta.size(); ++i)
        {
            const RowMeta &meta = mRowMeta[i];
            const void *def = meta.kind == emc::MOD_HUB_CLIENT_SETTING_KIND_BOOL
                                  ? static_cast<const void *>(&mBoolDefs[meta.defIndex])
                                  : static_cast<const void *>(&mFloatDefs[meta.defIndex]);
            emc::ModHubClientSettingRowV1 row = {meta.kind, meta.id, def, meta.sectionId, meta.sectionLabel};
            mRows.push_back(row);
        }

        mRegistration.mod_desc = &mDescriptor;
        mRegistration.rows = mRows.empty() ? nullptr : &mRows[0];
        mRegistration.row_count = static_cast<uint32_t>(mRows.size());
        mRegistration.bool_condition_rules = mConditionRules.empty() ? nullptr : &mConditionRules[0];
        mRegistration.bool_condition_rule_count = static_cast<uint32_t>(mConditionRules.size());

        emc::ModHubClient::Config config;
        config.table_registration = &mRegistration;
        mClient->SetConfig(config);
    }

  private:
    struct RowMeta
    {
        int32_t kind;
        const char *id;
        size_t defIndex;
        const char *sectionId;
        const char *sectionLabel;
    };

    emc::ModHubClient *mClient;
    const char *mCurrentSectionId;
    const char *mCurrentSectionLabel;
    EMC_ModDescriptorV1 mDescriptor;
    std::vector<emc::ModHubClientSettingRowV1> mRows;
    emc::ModHubClientTableRegistrationV1 mRegistration;

    std::vector<EMC_BoolSettingDefV1> mBoolDefs;
    std::vector<EMC_FloatSettingDefV1> mFloatDefs;
    std::vector<EMC_BoolConditionRuleDefV1> mConditionRules;
    std::vector<RowMeta> mRowMeta;
};

// -----------------------------------------------------------------------
// Client configuration (declarative, one Add* call per setting)
// -----------------------------------------------------------------------

void EnsureModHubClientConfigured()
{
    if (gModHubClientConfigured) { return; }

    static ModHubBuilder builder(
        &gModHubClient, kHubNamespaceId, kHubNamespaceDisplayName, kHubModId, kHubModDisplayName
    );

    builder.AddBool<&RenameBuildingsConfig::enabled>("enabled", "Enabled", "Enable the mod");

    static const float kButtonPositionIncrement = 0.001F;
    builder.AddFloat<&RenameBuildingsConfig::buttonX>(
        "button_x", "Button X", "Horizontal position of the rename button (0.0-1.0)", 0.0F, 1.0F,
        kButtonPositionIncrement
    );

    builder.AddFloat<&RenameBuildingsConfig::buttonY>(
        "button_y", "Button Y", "Vertical position of the rename button (0.0-1.0)", 0.0F, 1.0F, kButtonPositionIncrement
    );

    builder.BeginSection(kSectionOverridesId, kSectionOverridesLabel);

    builder.AddBool<&RenameBuildingsConfig::allowRenamingNonPlayerBuildings>(
        "allow_renaming_non_player_buildings", "Allow non-player buildings",
        "Allow renaming buildings that are not owned by the player"
    );

    builder.AddBool<&RenameBuildingsConfig::allowSelectiveRenames>(
        "allow_selective_renames", "Allow selective renames", "Enable per-building-class rename filtering"
    );

    for (size_t i = 0; i < sizeof(kClassRenameableEntries) / sizeof(kClassRenameableEntries[0]); ++i)
    {
        builder.AddClassRenameable(kClassRenameableEntries[i]);
    }

    builder.EndSection();

    builder.BeginSection(kSectionDebugId, kSectionDebugLabel);

    builder.AddBool<&RenameBuildingsConfig::verboseDebugLogging>(
        "verbose_debug_logging", "Verbose debug", "Enable excessively verbose diagnostic logging"
    );

    builder.AddBool<&RenameBuildingsConfig::developerDebug>(
        "developer_debug", "Developer hotkeys", "Enable CTRL+T debug hotkey"
    );

    builder.EndSection();

    builder.Finalize();
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