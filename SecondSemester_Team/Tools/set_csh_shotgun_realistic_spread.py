import unreal
path = "/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun"
bp = unreal.EditorAssetLibrary.load_asset(path)
if not bp:
    raise RuntimeError("Shotgun BP missing")
cdo = unreal.get_default_object(bp.generated_class())
cdo.set_editor_property("projectiles_per_shot", 8)
cdo.set_editor_property("spread_angle_degrees", 6.0)
unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
unreal.log("Shotgun set to realistic 8-pellet, 6-degree center-weighted spread")