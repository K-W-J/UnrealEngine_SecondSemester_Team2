import unreal
LIB = unreal.EditorAssetLibrary
pairs = (
    ("/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_AK47", "AK-47", "ASSAULT RIFLE / AUTO"),
    ("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol", "SCI-FI PISTOL", "SIDEARM / SEMI-AUTO"),
)
for path, name, description in pairs:
    bp = LIB.load_asset(path)
    if not bp:
        raise RuntimeError("Missing weapon BP: " + path)
    cdo = unreal.get_default_object(bp.generated_class())
    cdo.set_editor_property("weapon_display_name", name)
    cdo.set_editor_property("weapon_description", description)
    LIB.save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log_warning(f"Updated weapon UI info: {name}")