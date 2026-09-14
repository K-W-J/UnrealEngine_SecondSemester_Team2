import unreal

lib = unreal.EditorAssetLibrary
root = '/Game/CSH/Buleprint/Weapons'
folder = root + '/BulletLauncher'
def asset(path):
    result = unreal.load_asset(path)
    assert result, path
    return result
def bp(name, parent):
    path = folder + '/' + name
    if lib.does_asset_exist(path):
        return asset(path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    result = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Blueprint, factory)
    assert result, path
    return result
def props(obj, **values):
    for key, value in values.items():
        obj.set_editor_property(key, value)

held = asset('/Game/Art/CSH/Weapons/LiteWeapons_JC/Models/SM_BulletLite_02')
projectile_mesh = asset('/Game/Art/CSH/Weapons/AK47/ak47fbx/StaticMeshes/ak47fbx')
lib.make_directory(folder)
bullet = bp('BP_CSH_AK47Projectile', unreal.load_class(None, '/Script/SecondSemester_Team.CSHBullet'))
weapon = bp('BP_CSH_BulletLauncher', asset(root + '/BP_CSH_WeaponBase').generated_class())
box = bp('BP_CSH_BulletLauncherBox', asset(root + '/BP_CSH_WeaponBoxBase').generated_class())
bc = unreal.get_default_object(bullet.generated_class())
props(bc, damage=40.0, initial_speed=2000.0, gravity_scale=0.0, collision_radius=5.0,
      life_seconds=5.0, explosive=False, piercing=False, homing=False,
      tracer_light_intensity=350.0)
props(bc.get_editor_property('bullet_mesh'), static_mesh=projectile_mesh,
      relative_rotation=unreal.Rotator(pitch=0, yaw=-90, roll=0),
      relative_scale3d=unreal.Vector(0.6, 0.6, 0.6), override_materials=[])

# Normalize the held cartridge to 35 cm along its longest axis.
bounds = held.get_bounds()
extent = bounds.box_extent
axis = max(range(3), key=lambda i: (extent.x, extent.y, extent.z)[i])
scale = 35.0 / (2.0 * (extent.x, extent.y, extent.z)[axis])
rotation = (unreal.Rotator(), unreal.Rotator(yaw=-90), unreal.Rotator(pitch=-90))[axis]
wc = unreal.get_default_object(weapon.generated_class())
props(wc, weapon_display_name=unreal.Text('BULLET LAUNCHER'),
      weapon_description=unreal.Text('AK-47 LAUNCHER'), bullet_class=bullet.generated_class(),
      magazine_capacity=10, infinite_ammo=False, automatic=True, fire_interval=0.35,
      melee_weapon=False, projectiles_per_shot=1, spread_angle_degrees=0.0,
      weapon_kick_distance=6.0, camera_pitch_kick=0.5, camera_yaw_kick=0.1)
wm = wc.get_editor_property('weapon_mesh')
props(wm, static_mesh=held, relative_location=unreal.Vector(), relative_rotation=rotation,
      relative_scale3d=unreal.Vector(scale, scale, scale), override_materials=[])
transform = unreal.Transform(location=unreal.Vector(), rotation=rotation, scale=unreal.Vector(scale,scale,scale))
center = unreal.MathLibrary.transform_location(transform, bounds.origin)
props(wc.get_editor_property('muzzle_point'), relative_location=center + unreal.Vector(17.5,0,0), relative_rotation=unreal.Rotator())
xc = unreal.get_default_object(box.generated_class())
props(xc, weapon_class=weapon.generated_class(), pickup_radius=220.0,
      through_wall_glow_material=asset('/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2'))
props(xc.get_editor_property('box_mesh'), static_mesh=asset('/Engine/BasicShapes/Cube'), relative_scale3d=unreal.Vector(.7,.7,.45))
props(xc.get_editor_property('preview_weapon_mesh'), static_mesh=held,
      relative_location=unreal.Vector(0,0,70), relative_rotation=rotation,
      relative_scale3d=unreal.Vector(scale,scale,scale), override_materials=[])
for item in (bullet, weapon, box):
    assert lib.save_loaded_asset(item, only_if_is_dirty=False)

for path, size in {
    '/RPGLauncher/BP_CSH_RPGRocket': 2.3,
    '/StrelaLauncher/BP_CSH_StrelaMissile': 1.8,
    '/SciFiSniper/BP_CSH_SniperBullet': 1.1,
}.items():
    target = asset(root + path)
    props(unreal.get_default_object(target.generated_class()), impact_explosion_scale=size)
    assert lib.save_loaded_asset(target, only_if_is_dirty=False)

assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/CSH/Maps/InGame')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = actors.get_all_level_actors()
label = 'CSH_BulletLauncher_WeaponBox'
placed = next((a for a in all_actors if a.get_actor_label() == label), None)
if not placed:
    start = next(a for a in all_actors if isinstance(a, unreal.PlayerStart))
    placed = actors.spawn_actor_from_class(box.generated_class(), start.get_actor_location() + unreal.Vector(450,750,20))
    assert placed
    placed.set_actor_label(label)
props(placed, weapon_class=weapon.generated_class(), pickup_radius=220.0)
assert placed.get_editor_property('weapon_class') == weapon.generated_class()
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning('BULLET_LAUNCHER_READY: weapon, AK47 projectile, pickup instance saved; explosion scales doubled')
