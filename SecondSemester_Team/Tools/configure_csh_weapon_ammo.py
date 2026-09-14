import unreal

WEAPONS = {
    "/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_AK47": (30, False),
    "/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol": (12, False),
    "/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun": (2, False),
    "/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterGun": (100, True),
}

library = unreal.EditorAssetLibrary
for asset_path, (capacity, infinite) in WEAPONS.items():
    asset = library.load_asset(asset_path)
    generated_class = unreal.load_class(None, asset_path + "." + asset_path.rsplit("/", 1)[-1] + "_C")
    if not asset or not generated_class:
        raise RuntimeError(f"Could not load weapon blueprint: {asset_path}")

    default_object = unreal.get_default_object(generated_class)
    default_object.set_editor_property("magazine_capacity", capacity)
    default_object.set_editor_property("infinite_ammo", infinite)
    library.save_loaded_asset(asset, only_if_is_dirty=False)
    unreal.log_warning(f"Ammo configured: {asset_path}, magazine={capacity}, infinite={infinite}")
