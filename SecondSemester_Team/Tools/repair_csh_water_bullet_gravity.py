import unreal

PATH = "/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterBullet"
bp = unreal.EditorAssetLibrary.load_asset(PATH)
if not bp:
    raise RuntimeError(f"Missing water bullet blueprint: {PATH}")

cdo = unreal.get_default_object(bp.generated_class())
cdo.set_editor_property("gravity_scale", 1.0)

# Keep the component default consistent as well. C++ copies GravityScale into
# ProjectileMovement when the projectile starts playing.
movement = cdo.get_editor_property("projectile_movement")
movement.set_editor_property("projectile_gravity_scale", 1.0)

unreal.EditorAssetLibrary.save_loaded_asset(bp)
unreal.log(
    f"Water bullet gravity repaired: GravityScale={cdo.get_editor_property('gravity_scale')}, "
    f"ProjectileGravityScale={movement.get_editor_property('projectile_gravity_scale')}"
)
