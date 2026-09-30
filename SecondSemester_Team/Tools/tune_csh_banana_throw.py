import unreal

# Update only flight/effects; preserve any hand-adjusted component transforms.
folder = '/Game/CSH/Buleprint/Weapons/BananaPistol/'
bullet = unreal.load_asset(folder + 'BP_CSH_BananaBullet')
weapon = unreal.load_asset(folder + 'BP_CSH_BananaPistol')
assert bullet and weapon
bc = unreal.get_default_object(bullet.generated_class())
wc = unreal.get_default_object(weapon.generated_class())
bc.set_editor_property('initial_speed', 1500.0)
bc.set_editor_property('gravity_scale', 1.0)
bc.set_editor_property('tracer_light_intensity', 0.0)
wc.set_editor_property('muzzle_flash', None)
for bp in (bullet, weapon):
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
unreal.log_warning('BANANA_TUNED: speed=1500 gravity=1 light=0 muzzle flash removed')
