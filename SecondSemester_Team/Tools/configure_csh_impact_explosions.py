import unreal

CONFIG = {
    "/Game/CSH/Buleprint/Weapons/RPGLauncher/BP_CSH_RPGRocket": (1.15, 300.0, 2400.0),
    "/Game/CSH/Buleprint/Weapons/StrelaLauncher/BP_CSH_StrelaMissile": (0.9, 250.0, 2000.0),
    "/Game/CSH/Buleprint/Weapons/SciFiSniper/BP_CSH_SniperBullet": (0.55, 150.0, 1400.0),
}

for path, (scale, inner_radius, outer_radius) in CONFIG.items():
    bp = unreal.EditorAssetLibrary.load_asset(path)
    if not bp:
        raise RuntimeError(f"Missing projectile blueprint: {path}")
    cdo = unreal.get_default_object(bp.generated_class())
    cdo.set_editor_property("spawn_impact_explosion", True)
    cdo.set_editor_property("impact_explosion_scale", scale)
    cdo.set_editor_property("camera_shake_inner_radius", inner_radius)
    cdo.set_editor_property("camera_shake_outer_radius", outer_radius)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    unreal.log(
        f"Impact explosion enabled: {path}, scale={scale}, "
        f"shake={inner_radius}-{outer_radius}"
    )

unreal.log("RPG, Strela and sniper impact explosions configured.")
