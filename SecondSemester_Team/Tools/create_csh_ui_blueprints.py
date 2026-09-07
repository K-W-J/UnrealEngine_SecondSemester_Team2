import unreal

ROOT = "/Game/CSH/Buleprint/UI"
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary

LIB.make_directory(ROOT)

def create_widget_bp(name, parent_path):
    asset_path = f"{ROOT}/{name}"
    existing = LIB.load_asset(asset_path)
    if existing:
        unreal.log_warning(f"Using existing widget BP: {asset_path}")
        return existing

    parent_class = unreal.load_class(None, parent_path)
    if not parent_class:
        raise RuntimeError(f"Missing parent class: {parent_path}")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = TOOLS.create_asset(name, ROOT, unreal.WidgetBlueprint, factory)
    if not asset:
        raise RuntimeError(f"Failed to create: {asset_path}")
    LIB.save_loaded_asset(asset, only_if_is_dirty=False)
    unreal.log_warning(f"Created widget BP: {asset_path}")
    return asset

weapon_ui = create_widget_bp("WBP_CSH_WeaponUI", "/Script/SecondSemester_Team.CSHWeaponHUDWidget")
main_ui = create_widget_bp("WBP_CSH_MainUI", "/Script/SecondSemester_Team.CSHMainUIWidget")

LIB.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
unreal.log_warning("CSH widget blueprints are ready. Main UI owns Weapon UI at runtime.")