import unreal

LIB = unreal.EditorAssetLibrary
box_path = "/Game/CSH/Buleprint/Weapons/BP_CSH_WaterGunBox"
weapon_path = "/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterGun"
map_path = "/Game/CSH/Maps/InGame"
box = LIB.load_asset(box_path)
weapon = LIB.load_asset(weapon_path)
if not box or not weapon:
    raise RuntimeError("Water gun assets are missing")

box_cdo = unreal.get_default_object(box.generated_class())
box_cdo.set_editor_property("weapon_class", weapon.generated_class())
for component in box_cdo.get_components_by_class(unreal.ActorComponent):
    if component.get_name() == "PickupTrigger":
        component.set_box_extent(unreal.Vector(150.0, 150.0, 150.0), False)
        component.set_editor_property("generate_overlap_events", True)
        component.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)

LIB.save_loaded_asset(weapon, only_if_is_dirty=False)
LIB.save_loaded_asset(box, only_if_is_dirty=False)

unreal.EditorLoadingAndSavingUtils.load_map(map_path)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
old = [actor for actor in actors if actor.get_actor_label() == "CSH_WaterGun_WeaponBox"]
location = old[0].get_actor_location() if old else unreal.Vector(-150.0, -300.0, 132.0)
for actor in old:
    actor_subsystem.destroy_actor(actor)
new_actor = actor_subsystem.spawn_actor_from_class(box.generated_class(), location, unreal.Rotator())
if not new_actor:
    raise RuntimeError("Failed to respawn Water Gun pickup")
new_actor.set_actor_label("CSH_WaterGun_WeaponBox")
for component in new_actor.get_components_by_class(unreal.ActorComponent):
    if component.get_name() == "PickupTrigger":
        component.set_box_extent(unreal.Vector(150.0, 150.0, 150.0), True)
        component.set_editor_property("generate_overlap_events", True)
        component.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"Water Gun pickup repaired at {location}")