import unreal

path = "/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol"
weapon = unreal.EditorAssetLibrary.load_asset(path)
if not weapon:
    raise RuntimeError(f"Missing pistol Blueprint: {path}")
cdo = unreal.get_default_object(weapon.generated_class())
cdo.set_editor_property("weapon_kick_distance", 4.0)
cdo.set_editor_property("camera_pitch_kick", 0.0)
cdo.set_editor_property("camera_yaw_kick", 0.0)
unreal.EditorAssetLibrary.save_loaded_asset(weapon, only_if_is_dirty=False)
unreal.log("Sci-fi pistol model kick enabled; camera recoil disabled.")