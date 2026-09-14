import unreal

LIB = unreal.EditorAssetLibrary
box_path = "/Game/CSH/Buleprint/Weapons/BP_CSH_WaterGunBox"
weapon_path = "/Game/CSH/Buleprint/Weapons/WaterGun/BP_CSH_WaterGun"
map_path = "/Game/CSH/Maps/InGame"
box = LIB.load_asset(box_path)
weapon = LIB.load_asset(weapon_path)
if not box or not weapon:
    raise RuntimeError(f"Assets missing: box={box}, weapon={weapon}")
box_cdo = unreal.get_default_object(box.generated_class())
unreal.log_warning(f"BOX_CLASS={box.generated_class().get_path_name()}")
unreal.log_warning(f"WEAPON_CLASS={weapon.generated_class().get_path_name()}")
unreal.log_warning(f"BOX_WEAPON_CLASS={box_cdo.get_editor_property('weapon_class')}")
for component in box_cdo.get_components_by_class(unreal.ActorComponent):
    if component.get_name() == "Trigger":
        unreal.log_warning(f"TRIGGER_OVERLAP={component.get_editor_property('generate_overlap_events')}")
        unreal.log_warning(f"TRIGGER_COLLISION={component.get_editor_property('collision_enabled')}")
unreal.EditorLoadingAndSavingUtils.load_map(map_path)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
found = [a for a in actors if a.get_actor_label() == "CSH_WaterGun_WeaponBox"]
unreal.log_warning(f"MAP_WATER_BOX_COUNT={len(found)}")
for actor in found:
    unreal.log_warning(f"ACTOR={actor.get_path_name()} LOCATION={actor.get_actor_location()} CLASS={actor.get_class().get_path_name()}")