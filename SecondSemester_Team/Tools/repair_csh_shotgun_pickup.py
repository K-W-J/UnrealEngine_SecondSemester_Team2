import unreal
LIB = unreal.EditorAssetLibrary
box = LIB.load_asset("/Game/CSH/Buleprint/Weapons/BP_CSH_DoubleBarrelShotgunBox")
weapon = LIB.load_asset("/Game/CSH/Buleprint/Weapons/DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun")
if not box or not weapon:
    raise RuntimeError("Shotgun pickup assets missing")
cdo = unreal.get_default_object(box.generated_class())
cdo.set_editor_property("weapon_class", weapon.generated_class())
cdo.set_editor_property("pickup_radius", 250.0)
LIB.save_loaded_asset(box, only_if_is_dirty=False)
map_path = "/Game/CSH/Maps/InGame"
unreal.EditorLoadingAndSavingUtils.load_map(map_path)
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()
old = [a for a in actors if a.get_actor_label() == "CSH_DoubleBarrelShotgun_WeaponBox"]
location = old[0].get_actor_location() if old else unreal.Vector(300.0, -250.0, 132.0)
for actor in old:
    subsystem.destroy_actor(actor)
new_actor = subsystem.spawn_actor_from_class(box.generated_class(), location, unreal.Rotator())
if not new_actor:
    raise RuntimeError("Failed to respawn shotgun pickup")
new_actor.set_actor_label("CSH_DoubleBarrelShotgun_WeaponBox")
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"Shotgun pickup repaired at {location}, radius=250")