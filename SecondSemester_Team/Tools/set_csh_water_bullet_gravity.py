import unreal

path = "/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterBullet"
bp = unreal.EditorAssetLibrary.load_asset(path)
if not bp:
    raise RuntimeError(f"Missing water bullet: {path}")
cdo = unreal.get_default_object(bp.generated_class())
movement = None
for component in cdo.get_components_by_class(unreal.ActorComponent):
    if component.get_name() == "ProjectileMovement":
        movement = component
        break
if not movement:
    raise RuntimeError("ProjectileMovement component missing")
movement.set_editor_property("projectile_gravity_scale", 1.0)
unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
unreal.log("Water bullet gravity scale set to 1.0")