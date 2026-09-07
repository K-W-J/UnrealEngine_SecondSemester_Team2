import unreal

root = "/Game/CSH/Buleprint/UI"
name = "WBP_CSH_PlayerStatusUI"
path = f"{root}/{name}"
library = unreal.EditorAssetLibrary

library.make_directory(root)
asset = library.load_asset(path)
if not asset:
    parent_class = unreal.load_class(None, "/Script/SecondSemester_Team.CSHPlayerStatusWidget")
    if not parent_class:
        raise RuntimeError("CSHPlayerStatusWidget class could not be loaded")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, root, unreal.WidgetBlueprint, factory
    )
    if not asset:
        raise RuntimeError(f"Failed to create {path}")

library.save_loaded_asset(asset, only_if_is_dirty=False)
unreal.log_warning(f"Player status UI ready: {path}")
