import unreal

MATERIAL_PATH = "/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2"
ROOT = "/Game/CSH/Buleprint/Weapons"
material = unreal.EditorAssetLibrary.load_asset(MATERIAL_PATH)
if not material:
    raise RuntimeError(f"Missing glow material: {MATERIAL_PATH}")

count = 0
for path in unreal.EditorAssetLibrary.list_assets(ROOT, recursive=True, include_folder=False):
    bp = unreal.EditorAssetLibrary.load_asset(path)
    if not isinstance(bp, unreal.Blueprint):
        continue
    try:
        cdo = unreal.get_default_object(bp.generated_class())
        cdo.get_editor_property("pickup_trigger")
        cdo.set_editor_property("through_wall_glow_material", material)
    except Exception:
        continue
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    count += 1
    unreal.log(f"Assigned stencil glow: {path}")

if count == 0:
    raise RuntimeError("No weapon box blueprints were updated")
unreal.log(f"Assigned working glow material to {count} weapon box blueprints")
