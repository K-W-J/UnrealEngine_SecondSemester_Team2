import unreal

path = '/Game/CSH/Buleprint/Weapons/BulletLauncher/BP_CSH_AK47Projectile'
bp = unreal.load_asset(path)
assert bp, path
cdo = unreal.get_default_object(bp.generated_class())
mesh = cdo.get_editor_property('bullet_mesh')
# AK47's barrel points along local Y. Zero mesh yaw leaves it perpendicular
# to the projectile actor's forward X direction; movement is unchanged.
mesh.set_editor_property('relative_rotation', unreal.Rotator(pitch=0, yaw=0, roll=0))
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
rotation = mesh.get_editor_property('relative_rotation')
assert abs(rotation.yaw) < 0.001
unreal.log_warning(f'AK47_SIDEWAYS_SAVED: {path}, mesh rotation={rotation}')
