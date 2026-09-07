import unreal
path = "/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun"
bp = unreal.EditorAssetLibrary.load_asset(path)
if not bp:
    raise RuntimeError("Shotgun BP missing")
cdo = unreal.get_default_object(bp.generated_class())
cdo.set_editor_property("spread_angle_degrees", 18.0)
unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
unreal.log("Shotgun spread set to 18 degrees")