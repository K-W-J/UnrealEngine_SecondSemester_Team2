import unreal
bp = unreal.EditorAssetLibrary.load_asset("/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterBullet")
if not bp:
    raise RuntimeError("Water bullet missing")
cdo = unreal.get_default_object(bp.generated_class())
unreal.log_warning(f"TRACER_SCALE={cdo.get_editor_property('tracer_scale')}")
for c in cdo.get_components_by_class(unreal.ActorComponent):
    if c.get_name() == "BulletMesh":
        unreal.log_warning(f"MESH_RELATIVE_SCALE={c.get_editor_property('relative_scale3d')}")