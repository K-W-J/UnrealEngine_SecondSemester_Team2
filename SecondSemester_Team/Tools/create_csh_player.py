import unreal

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EDITOR_ASSET_LIBRARY = unreal.EditorAssetLibrary


def duplicate_asset(source_path, destination_path):
    if EDITOR_ASSET_LIBRARY.does_asset_exist(destination_path):
        unreal.log(f"Already exists: {destination_path}")
        return EDITOR_ASSET_LIBRARY.load_asset(destination_path)

    destination_package, destination_name = destination_path.rsplit("/", 1)
    source_asset = EDITOR_ASSET_LIBRARY.load_asset(source_path)
    if source_asset is None:
        raise RuntimeError(f"Could not load source asset: {source_path}")

    duplicated_asset = ASSET_TOOLS.duplicate_asset(
        destination_name,
        destination_package,
        source_asset,
    )
    if duplicated_asset is None:
        raise RuntimeError(f"Could not duplicate {source_path} to {destination_path}")
    return duplicated_asset


EDITOR_ASSET_LIBRARY.make_directory("/Game/CSH")
player_blueprint = duplicate_asset(
    "/Game/Templates/FirstPerson/Blueprints/BP_FirstPersonCharacter",
    "/Game/CSH/BP_CSH_Player",
)
controller_blueprint = duplicate_asset(
    "/Game/Templates/FirstPerson/Blueprints/BP_FirstPersonPlayerController",
    "/Game/CSH/BP_CSH_PlayerController",
)
game_mode_blueprint = duplicate_asset(
    "/Game/Templates/FirstPerson/Blueprints/BP_FirstPersonGameMode",
    "/Game/CSH/BP_CSH_GameMode",
)

player_class = player_blueprint.generated_class()
controller_class = controller_blueprint.generated_class()
game_mode_class = game_mode_blueprint.generated_class()
game_mode_defaults = unreal.get_default_object(game_mode_class)
game_mode_defaults.set_editor_property("default_pawn_class", player_class)
game_mode_defaults.set_editor_property("player_controller_class", controller_class)


for asset in (player_blueprint, controller_blueprint, game_mode_blueprint):
    EDITOR_ASSET_LIBRARY.save_loaded_asset(asset, only_if_is_dirty=False)

unreal.log("Created /Game/CSH player assets and connected the default pawn/controller.")

