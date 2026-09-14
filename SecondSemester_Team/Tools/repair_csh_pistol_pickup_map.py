import unreal

LIB = unreal.EditorAssetLibrary
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
box_bp = LIB.load_asset("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistolBox")
pistol_bp = LIB.load_asset("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol")
if not box_bp or not pistol_bp:
    raise RuntimeError("Pistol assets are missing")

# Remove only the pistol pickup previously added by this task to the wrong map.
wrong_map = "/Game/Maps/InGame"
if LIB.does_asset_exist(wrong_map):
    unreal.EditorLoadingAndSavingUtils.load_map(wrong_map)
    wrong_actors = [a for a in actor_subsystem.get_all_level_actors() if a.get_actor_label() == "CSH_SciFiPistol_WeaponBox"]
    for actor in wrong_actors:
        actor_subsystem.destroy_actor(actor)
    if wrong_actors:
        level_subsystem.save_current_level()
        unreal.log_warning(f"Removed {len(wrong_actors)} task-created pistol pickup(s) from {wrong_map}")

# Recreate a clean pickup only in the requested CSH map.
target_map = "/Game/CSH/Maps/InGame"
if not LIB.does_asset_exist(target_map):
    raise RuntimeError("Missing target map: " + target_map)
unreal.EditorLoadingAndSavingUtils.load_map(target_map)
actors = actor_subsystem.get_all_level_actors()
for actor in [a for a in actors if a.get_actor_label() == "CSH_SciFiPistol_WeaponBox"]:
    actor_subsystem.destroy_actor(actor)

actors = actor_subsystem.get_all_level_actors()
starts = [a for a in actors if isinstance(a, unreal.PlayerStart)]
if starts:
    start = starts[0].get_actor_location()
    spawn_location = start + unreal.Vector(0.0, 300.0, 0.0)
else:
    spawn_location = unreal.Vector(0.0, 300.0, 100.0)

new_box = actor_subsystem.spawn_actor_from_class(box_bp.generated_class(), spawn_location, unreal.Rotator())
if not new_box:
    raise RuntimeError("Failed to spawn pistol pickup")
new_box.set_actor_label("CSH_SciFiPistol_WeaponBox")
level_subsystem.save_current_level()
unreal.log_warning(f"Recreated pistol pickup ONLY in {target_map}")
unreal.log_warning(f"LOCATION={new_box.get_actor_location()}")
unreal.log_warning(f"WEAPON_CLASS={new_box.get_editor_property('weapon_class')}")