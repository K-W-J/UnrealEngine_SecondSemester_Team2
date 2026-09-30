import unreal

root = '/Game/CSH/Buleprint/Weapons'
folder = root + '/BananaPistol'
def defaults(path):
    bp = unreal.load_asset(path)
    assert bp, path
    return bp.generated_class(), unreal.get_default_object(bp.generated_class())

weapon, wc = defaults(folder + '/BP_CSH_BananaPistol')
bullet, bc = defaults(folder + '/BP_CSH_BananaBullet')
_, box = defaults(folder + '/BP_CSH_BananaPistolBox')
_, random_box = defaults(root + '/RandomWeapon/BP_CSH_RandomWeaponBox')
assert wc.get_editor_property('bullet_class') == bullet
assert box.get_editor_property('weapon_class') == weapon
assert weapon in random_box.get_editor_property('random_weapon_classes')
assert wc.get_editor_property('magazine_capacity') == 12
assert not wc.get_editor_property('automatic')
assert bc.get_editor_property('initial_speed') == 1500
assert bc.get_editor_property('gravity_scale') == 1.0
assert bc.get_editor_property('tracer_light_intensity') == 0.0
assert wc.get_editor_property('muzzle_flash') is None
assert bc.get_editor_property('collision_radius') == 6.0
assert bc.get_editor_property('life_seconds') == 8.0
assert bc.get_editor_property('projectile_movement').get_editor_property('should_bounce')
assert not bc.get_editor_property('projectile_movement').get_editor_property('rotation_follows_velocity')
assert bc.get_editor_property('spin_rate').pitch == 420
for obj, component in ((wc, 'weapon_mesh'), (bc, 'bullet_mesh'), (box, 'preview_weapon_mesh')):
    mesh = obj.get_editor_property(component)
    assert mesh.get_editor_property('static_mesh').get_name() == 'banana_3d_scan'
    assert not mesh.get_editor_property('override_materials')
    assert mesh.get_editor_property('relative_scale3d').x > 0
unreal.log_warning('BANANA_VERIFIED: saved assets reloaded; weapon, bullet, pickup and random pool links OK')
