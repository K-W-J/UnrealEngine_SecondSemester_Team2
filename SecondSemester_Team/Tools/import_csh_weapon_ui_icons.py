import os
import unreal

project_dir = unreal.Paths.project_dir()
source_dir = os.path.join(project_dir, "Content", "CSH", "UI", "WeaponIcons_Source")
destination = "/Game/CSH/UI/WeaponIcons"
library = unreal.EditorAssetLibrary
library.make_directory(destination)

entries = {
    "T_UI_AK47": "/Game/CSH/Buleprint/Weapons/AK47/BP_CSH_AK47",
    "T_UI_SciFiPistol": "/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol",
    "T_UI_DoubleBarrelShotgun": "/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun",
    "T_UI_WaterGun": "/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterGun",
}

for texture_name, weapon_path in entries.items():
    texture_path = f"{destination}/{texture_name}"
    texture = library.load_asset(texture_path) if library.does_asset_exist(texture_path) else None
    if not texture:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", os.path.join(source_dir, texture_name + ".png"))
        task.set_editor_property("destination_path", destination)
        task.set_editor_property("destination_name", texture_name)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", True)
        task.set_editor_property("replace_existing", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = library.load_asset(texture_path)
    if not texture:
        raise RuntimeError(f"Icon import failed: {texture_name}")

    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
    library.save_loaded_asset(texture, only_if_is_dirty=False)

    bp = library.load_asset(weapon_path)
    class_name = weapon_path.rsplit("/", 1)[-1] + "_C"
    generated_class = unreal.load_class(None, weapon_path + "." + class_name)
    if not bp or not generated_class:
        raise RuntimeError(f"Weapon load failed: {weapon_path}")
    unreal.get_default_object(generated_class).set_editor_property("weapon_icon", texture)
    library.save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log_warning(f"Weapon HUD icon assigned: {weapon_path} -> {texture_path}")
