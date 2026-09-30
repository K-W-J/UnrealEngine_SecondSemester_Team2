import unreal

bp = unreal.load_asset('/Game/CSH/Buleprint/Weapons/BananaPistol/BP_CSH_BananaBullet')
assert bp
cdo = unreal.get_default_object(bp.generated_class())
cdo.set_editor_property('collision_radius', 6.0)
cdo.set_editor_property('life_seconds', 8.0)
cdo.set_editor_property('spin_rate', unreal.Rotator(pitch=420, yaw=90, roll=180))
collision = cdo.get_editor_property('collision')
collision.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
collision.set_collision_profile_name('BlockAllDynamic')
movement = cdo.get_editor_property('projectile_movement')
movement.set_editor_property('should_bounce', True)
movement.set_editor_property('rotation_follows_velocity', False)
movement.set_editor_property('bounciness', 0.3)
movement.set_editor_property('friction', 0.65)
movement.set_editor_property('bounce_velocity_stop_simulating_threshold', 35.0)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
unreal.log_warning('BANANA_COLLISION_READY: bounce .3, friction .65, radius 6, lifespan 8')
