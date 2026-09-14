import unreal
bp = unreal.EditorAssetLibrary.load_asset("/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun")
if not bp:
    raise RuntimeError("Shotgun BP missing")
cdo = unreal.get_default_object(bp.generated_class())
unreal.log_warning(f"PROJECTILES={cdo.get_editor_property('projectiles_per_shot')}")
unreal.log_warning(f"SPREAD_DEGREES={cdo.get_editor_property('spread_angle_degrees')}")
unreal.log_warning(f"BULLET_CLASS={cdo.get_editor_property('bullet_class')}")