import unreal

LIB = unreal.EditorAssetLibrary
MOVES = (
    ("/Game/CSH/Buleprint/Weapons/BP_CSH_AK47", "/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_AK47"),
    ("/Game/CSH/Buleprint/Weapons/BP_CSH_Bullet", "/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_Bullet"),
    ("/Game/CSH/Buleprint/Weapons/BP_CSH_SciFiPistol", "/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol"),
    ("/Game/CSH/Buleprint/Weapons/BP_CSH_PistolBullet", "/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_PistolBullet"),
)
for source, destination in MOVES:
    if LIB.does_asset_exist(destination):
        unreal.log(f"Already organized: {destination}")
        continue
    if not LIB.does_asset_exist(source):
        raise RuntimeError(f"Missing source asset: {source}")
    if not LIB.rename_asset(source, destination):
        raise RuntimeError(f"Failed to move {source} -> {destination}")
ak = LIB.load_asset("/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_AK47")
ak_bullet = LIB.load_asset("/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_Bullet")
pistol = LIB.load_asset("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol")
pistol_bullet = LIB.load_asset("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_PistolBullet")
unreal.get_default_object(ak.generated_class()).set_editor_property("bullet_class", ak_bullet.generated_class())
unreal.get_default_object(pistol.generated_class()).set_editor_property("bullet_class", pistol_bullet.generated_class())
for asset in (ak, ak_bullet, pistol, pistol_bullet):
    LIB.save_loaded_asset(asset, only_if_is_dirty=False)
unreal.log("CSH weapon assets organized by weapon name.")