import unreal

lib = unreal.EditorAssetLibrary
root = '/Game/CSH/Buleprint/Weapons'
folder = root + '/BananaPistol'


def asset(path):
    result = unreal.load_asset(path)
    assert result, path
    return result


def props(obj, **values):
    for key, value in values.items():
        obj.set_editor_property(key, value)


def blueprint(name, parent):
    path = folder + '/' + name
    if lib.does_asset_exist(path):
        return asset(path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    result = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, folder, unreal.Blueprint, factory)
    assert result, path
    return result


mesh = asset('/Game/Art/CSH/Weapons/banana_3d_scan/StaticMeshes/banana_3d_scan')
bounds = mesh.get_bounds()
extents = (bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z)
axis = max(range(3), key=lambda i: extents[i])
assert extents[axis] > 0
rotation = (unreal.Rotator(), unreal.Rotator(yaw=-90), unreal.Rotator(pitch=-90))[axis]


def configure_mesh(component, length, position):
    scale = length / (2.0 * extents[axis])
    transform = unreal.Transform(rotation=rotation, scale=unreal.Vector(scale, scale, scale))
    center = unreal.MathLibrary.transform_location(transform, bounds.origin)
    props(component, static_mesh=mesh, relative_location=position - center,
          relative_rotation=rotation, relative_scale3d=unreal.Vector(scale, scale, scale),
          override_materials=[])


lib.make_directory(folder)
bullet = blueprint('BP_CSH_BananaBullet', unreal.load_class(None, '/Script/SecondSemester_Team.CSHBullet'))
weapon = blueprint('BP_CSH_BananaPistol', asset(root + '/BP_CSH_WeaponBase').generated_class())
box = blueprint('BP_CSH_BananaPistolBox', asset(root + '/BP_CSH_WeaponBoxBase').generated_class())
bc = unreal.get_default_object(bullet.generated_class())
props(bc, damage=25.0, initial_speed=1500.0, gravity_scale=1.0,
      collision_radius=6.0, life_seconds=8.0, explosive=False,
      spawn_impact_explosion=False, piercing=False, homing=False,
      tracer_color=unreal.LinearColor(1.0, 0.65, 0.02, 1.0), tracer_light_intensity=0.0)
configure_mesh(bc.get_editor_property('bullet_mesh'), 18.0, unreal.Vector())
props(bc, spin_rate=unreal.Rotator(pitch=420, yaw=90, roll=180))
props(bc.get_editor_property('projectile_movement'), should_bounce=True,
      bounciness=0.3, friction=0.65, bounce_velocity_stop_simulating_threshold=35.0,
      rotation_follows_velocity=False)

wc = unreal.get_default_object(weapon.generated_class())
props(wc, weapon_display_name=unreal.Text('BANANA PISTOL'),
      weapon_description=unreal.Text('BANANA / SEMI-AUTO'),
      bullet_class=bullet.generated_class(), magazine_capacity=12,
      infinite_ammo=False, automatic=False, fire_interval=0.3,
      melee_weapon=False, projectiles_per_shot=1, spread_angle_degrees=0.0,
      weapon_kick_distance=3.0, camera_pitch_kick=0.0, camera_yaw_kick=0.0,
      muzzle_flash=None)
configure_mesh(wc.get_editor_property('weapon_mesh'), 30.0, unreal.Vector())
props(wc.get_editor_property('muzzle_point'), relative_location=unreal.Vector(16, 0, 0),
      relative_rotation=unreal.Rotator())

xc = unreal.get_default_object(box.generated_class())
props(xc, weapon_class=weapon.generated_class(), random_weapon=False,
      pickup_radius=220.0, destroy_after_pickup=True,
      through_wall_glow_material=asset('/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2'))
props(xc.get_editor_property('box_mesh'), static_mesh=asset('/Engine/BasicShapes/Cube'),
      relative_scale3d=unreal.Vector(0.7, 0.7, 0.45))
configure_mesh(xc.get_editor_property('preview_weapon_mesh'), 30.0, unreal.Vector(0, 0, 70))
for item in (bullet, weapon, box):
    assert lib.save_loaded_asset(item, only_if_is_dirty=False)

# Append without rebuilding the existing pool or modifying either map.
random_box = asset(root + '/RandomWeapon/BP_CSH_RandomWeaponBox')
rc = unreal.get_default_object(random_box.generated_class())
pool = list(rc.get_editor_property('random_weapon_classes'))
if weapon.generated_class() not in pool:
    pool.append(weapon.generated_class())
props(rc, random_weapon_classes=pool)
assert lib.save_loaded_asset(random_box, only_if_is_dirty=False)
assert wc.get_editor_property('bullet_class') == bullet.generated_class()
assert xc.get_editor_property('weapon_class') == weapon.generated_class()
assert weapon.generated_class() in rc.get_editor_property('random_weapon_classes')
unreal.log_warning('BANANA_READY: 3 BPs saved, pool=%d, mesh bounds=%s, axis=%d' % (len(pool), extents, axis))
