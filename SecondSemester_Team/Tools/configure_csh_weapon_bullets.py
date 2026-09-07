import unreal


ROOT = "/Game/CSH/Buleprint/Weapons"
AK_ROOT = f"{ROOT}/AK47"
PISTOL_ROOT = f"{ROOT}/SciFiPistol"
RIFLE_PATH = f"{AK_ROOT}/BP_CSH_Bullet"
PISTOL_BULLET_PATH = f"{PISTOL_ROOT}/BP_CSH_PistolBullet"
AK_PATH = f"{AK_ROOT}/BP_CSH_AK47"
PISTOL_PATH = f"{PISTOL_ROOT}/BP_CSH_SciFiPistol"


def load_required(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Required asset not found: {path}")
    return asset


rifle_bullet = load_required(RIFLE_PATH)

if unreal.EditorAssetLibrary.does_asset_exist(PISTOL_BULLET_PATH):
    pistol_bullet = load_required(PISTOL_BULLET_PATH)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", rifle_bullet.generated_class())
    pistol_bullet = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "BP_CSH_PistolBullet", PISTOL_ROOT, unreal.Blueprint, factory
    )
    if not pistol_bullet:
        raise RuntimeError("Failed to create BP_CSH_PistolBullet")


rifle_cdo = unreal.get_default_object(rifle_bullet.generated_class())
rifle_cdo.set_editor_property("damage", 20.0)
rifle_cdo.set_editor_property("initial_speed", 2000.0)
rifle_cdo.set_editor_property("tracer_color", unreal.LinearColor(1.0, 0.18, 0.01, 1.0))
rifle_cdo.set_editor_property("tracer_light_intensity", 1200.0)

pistol_cdo = unreal.get_default_object(pistol_bullet.generated_class())
pistol_cdo.set_editor_property("damage", 35.0)
pistol_cdo.set_editor_property("initial_speed", 2000.0)
pistol_cdo.set_editor_property("tracer_color", unreal.LinearColor(0.04, 0.35, 1.0, 1.0))
pistol_cdo.set_editor_property("tracer_light_intensity", 1800.0)

ak = load_required(AK_PATH)
pistol = load_required(PISTOL_PATH)
unreal.get_default_object(ak.generated_class()).set_editor_property(
    "bullet_class", rifle_bullet.generated_class()
)
unreal.get_default_object(pistol.generated_class()).set_editor_property(
    "bullet_class", pistol_bullet.generated_class()
)

for asset in (rifle_bullet, pistol_bullet, ak, pistol):
    unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)

unreal.log("Configured distinct AK and pistol bullet classes successfully.")
