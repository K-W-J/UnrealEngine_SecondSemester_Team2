import unreal

# Locations are in RecoilRoot space and were calculated from each transformed
# static-mesh bound, then placed at the visible barrel/tube endpoint.
MUZZLES = {
    "/Game/CSH/Buleprint/Weapons/RPGLauncher/BP_CSH_RPGLauncher": unreal.Vector(119.5, 0.0, 5.5),
    "/Game/CSH/Buleprint/Weapons/StrelaLauncher/BP_CSH_StrelaLauncher": unreal.Vector(41.5, 0.0, 18.0),
    "/Game/CSH/Buleprint/Weapons/SciFiSniper/BP_CSH_SciFiSniper": unreal.Vector(0.0, -34.0, 12.0),
    "/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol": unreal.Vector(20.0, 0.0, 5.0),
}

for path, location in MUZZLES.items():
    bp = unreal.EditorAssetLibrary.load_asset(path)
    if not bp:
        raise RuntimeError(f"Missing weapon blueprint: {path}")
    cdo = unreal.get_default_object(bp.generated_class())
    muzzle = cdo.get_editor_property("muzzle_point")
    muzzle.set_editor_property("relative_location", location)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    unreal.log(f"Muzzle repaired: {path} -> {location}")

unreal.log("Weapon muzzle point repair complete.")
