import unreal
LIB = unreal.EditorAssetLibrary
box_path = "/Game/CSH/Buleprint/Weapons/BP_CSH_DoubleBarrelShotgunBox"
weapon_path = "/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun"
map_path = "/Game/CSH/Maps/InGame"
box = LIB.load_asset(box_path)
weapon = LIB.load_asset(weapon_path)
if not box or not weapon:
    raise RuntimeError(f"missing box={box} weapon={weapon}")
cdo = unreal.get_default_object(box.generated_class())
unreal.log_warning(f"BOX_WEAPON={cdo.get_editor_property('weapon_class')}")
unreal.log_warning(f"PICKUP_RADIUS={cdo.get_editor_property('pickup_radius')}")
unreal.log_warning(f"CAN_TICK={cdo.get_editor_property('can_ever_tick') if cdo.has_editor_property('can_ever_tick') else 'native'}")
for c in cdo.get_components_by_class(unreal.ActorComponent):
    unreal.log_warning(f"COMP={c.get_name()} CLASS={c.get_class().get_name()}")
    if c.get_name() == "PickupTrigger":
        unreal.log_warning(f"EXTENT={c.get_unscaled_box_extent()} OVERLAP={c.get_editor_property('generate_overlap_events')} COLLISION={c.get_editor_property('collision_enabled')}")
unreal.EditorLoadingAndSavingUtils.load_map(map_path)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
found = [a for a in actors if a.get_actor_label() == "CSH_DoubleBarrelShotgun_WeaponBox"]
unreal.log_warning(f"COUNT={len(found)}")
for a in found:
    unreal.log_warning(f"ACTOR={a.get_path_name()} CLASS={a.get_class().get_path_name()} LOC={a.get_actor_location()} TICK={a.get_editor_property('allow_tick_before_begin_play')}")