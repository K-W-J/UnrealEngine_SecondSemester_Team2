import unreal

entries = (
    ("/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_Bullet", unreal.Vector(30.0, 0.7, 0.7)),
    ("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_PistolBullet", unreal.Vector(19.0, 0.5, 0.5)),
    ("/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterBullet", unreal.Vector(55.0, 0.28, 0.28)),
)
for path, scale in entries:
    bp = unreal.EditorAssetLibrary.load_asset(path)
    if not bp:
        raise RuntimeError(f"Missing bullet BP: {path}")
    cdo = unreal.get_default_object(bp.generated_class())
    mesh = None
    for component in cdo.get_components_by_class(unreal.ActorComponent):
        if component.get_name() == "BulletMesh":
            mesh = component
            break
    if not mesh:
        raise RuntimeError(f"BulletMesh missing: {path}")
    mesh.set_editor_property("relative_scale3d", scale)
    unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log(f"Baked BulletMesh scale {scale} into {path}")