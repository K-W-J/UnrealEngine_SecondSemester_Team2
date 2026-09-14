import unreal
lib = unreal.EditorAssetLibrary
root = '/Game/CSH/Buleprint/Weapons'
folder = root + '/FeignDeath'
def load(path):
    obj = unreal.load_asset(path)
    assert obj, path
    return obj
def make(name, parent):
    path = folder + '/' + name
    if lib.does_asset_exist(path): return load(path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    result = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Blueprint, factory)
    assert result
    return result
def setp(obj, **values):
    for key, value in values.items(): obj.set_editor_property(key,value)

lib.make_directory(folder)
native = unreal.load_class(None, '/Script/SecondSemester_Team.CSHFeignDeathWeapon')
assert native
weapon = make('BP_CSH_FeignDeath', native)
bullet = make('BP_CSH_FeignDeathBullet', unreal.load_class(None,'/Script/SecondSemester_Team.CSHBullet'))
box = make('BP_CSH_FeignDeathBox', load(root+'/BP_CSH_WeaponBoxBase').generated_class())
pistol = load('/Game/Art/CSH/Weapons/Sci-Fi_Pistol/StaticMeshes/scene')
wc = unreal.get_default_object(weapon.generated_class())
setp(wc, weapon_display_name=unreal.Text('PLAY DEAD'), weapon_description=unreal.Text('5 SEC / SPIN BURST'),
     bullet_class=bullet.generated_class(), magazine_capacity=3, infinite_ammo=False, automatic=False,
     feign_duration=5.0, spray_interval=.1, spin_speed=360.0, recovery_cooldown=2.0,
     camera_pitch_kick=0.0, camera_yaw_kick=0.0)
setp(wc.get_editor_property('weapon_mesh'), static_mesh=pistol,
     relative_rotation=unreal.Rotator(yaw=90), relative_scale3d=unreal.Vector(.2,.2,.2))
setp(wc.get_editor_property('muzzle_point'), relative_location=unreal.Vector(-20,0,5), relative_rotation=unreal.Rotator(yaw=180))
setp(wc, muzzle_flash=load('/Game/Art/CSH/NW_MuzzleFX/Particle_FX/FXS_NS_MuzzleFlash_02_v2'))
bc = unreal.get_default_object(bullet.generated_class())
setp(bc, damage=12.0, initial_speed=2000.0, gravity_scale=0.0, collision_radius=2.0,
     life_seconds=3.0, tracer_light_intensity=1000.0)
setp(bc.get_editor_property('bullet_mesh'), static_mesh=load('/Engine/BasicShapes/Cube'), relative_scale3d=unreal.Vector(.18,.025,.025))
xc = unreal.get_default_object(box.generated_class())
setp(xc, weapon_class=weapon.generated_class(), pickup_radius=220.0,
     through_wall_glow_material=load('/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2'))
setp(xc.get_editor_property('box_mesh'),static_mesh=load('/Engine/BasicShapes/Cube'),relative_scale3d=unreal.Vector(.7,.7,.45))
setp(xc.get_editor_property('preview_weapon_mesh'),static_mesh=pistol,relative_location=unreal.Vector(0,0,70),relative_rotation=unreal.Rotator(yaw=90),relative_scale3d=unreal.Vector(.3,.3,.3))
for item in (bullet,weapon,box): assert lib.save_loaded_asset(item,only_if_is_dirty=False)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/CSH/Maps/InGame')
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
label = 'CSH_FeignDeath_WeaponBox'
placed = next((a for a in actors if a.get_actor_label()==label),None)
if not placed:
    start = next(a for a in actors if isinstance(a,unreal.PlayerStart))
    placed = sub.spawn_actor_from_class(box.generated_class(),start.get_actor_location()+unreal.Vector(450,1000,20))
    assert placed
    placed.set_actor_label(label)
setp(placed,weapon_class=weapon.generated_class(),pickup_radius=220.0)
assert placed.get_editor_property('weapon_class')==weapon.generated_class()
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning('FEIGN_READY: reverse pistol, 5-second recovery, bullet and pickup configured and saved')
