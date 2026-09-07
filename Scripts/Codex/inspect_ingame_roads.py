import unreal


MAP_PATH = "/Game/Maps/InGame"

unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
unreal.SystemLibrary.execute_console_command(world, "wp.Editor.LoadAll")
if hasattr(unreal, "WorldPartitionEditorSubsystem"):
    subsystem = unreal.get_editor_subsystem(unreal.WorldPartitionEditorSubsystem)
    unreal.log(
        "CODEX_ROAD_SCAN wp_methods="
        f"{[name for name in dir(subsystem) if 'load' in name.lower()]}"
    )
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
unreal.log(f"CODEX_ROAD_SCAN actor_count={len(actors)}")
found = 0

for actor in actors:
    label = actor.get_actor_label()
    actor_name = actor.get_name()
    actor_matches = "road" in label.lower() or "road" in actor_name.lower() or "도로" in label
    components = actor.get_components_by_class(unreal.StaticMeshComponent)
    for component in components:
        mesh = component.get_editor_property("static_mesh")
        mesh_path = mesh.get_path_name() if mesh is not None else "None"
        mesh_matches = "road" in mesh_path.lower()
        if actor_matches or mesh_matches:
            unreal.log(
                f"CODEX_ROAD_SCAN actor_label={label} actor_name={actor_name} "
                f"component={component.get_name()} mesh={mesh_path} "
                f"collision={component.get_collision_enabled()} "
                f"profile={component.get_collision_profile_name()}"
            )
            found += 1

unreal.log(f"CODEX_ROAD_SCAN total={found}")

registry = unreal.AssetRegistryHelpers.get_asset_registry()
external_assets = registry.get_assets_by_path(
    "/Game/__ExternalActors__/Maps/InGame", recursive=True
)
unreal.log(f"CODEX_ROAD_SCAN external_asset_count={len(external_assets)}")
for data in external_assets:
    tag_values = {}
    for tag in (
        "ActorLabel",
        "ActorClass",
        "ActorPath",
        "ActorPackage",
        "FolderPath",
        "ParentActor",
    ):
        try:
            tag_values[tag] = data.get_tag_value(tag)
        except Exception:
            tag_values[tag] = None
    text = (
        f"package={data.package_name} asset={data.asset_name} "
        f"class={data.asset_class_path} tags={tag_values}"
    )
    lowered = text.lower()
    if "road" in lowered or "도로" in text or "_3" in text:
        unreal.log(f"CODEX_ROAD_SCAN external={text}")
