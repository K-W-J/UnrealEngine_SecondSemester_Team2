import unreal

TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
ROOT = "/Game/CSH/Buleprint/Weapons"
WATER_ROOT = f"{ROOT}/WaterGun"


def make_blueprint(name, parent_class, folder):
    path = f"{folder}/{name}"
    if LIB.does_asset_exist(path):
        return LIB.load_asset(path)
    LIB.make_directory(folder)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = TOOLS.create_asset(name, folder, unreal.Blueprint, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    return asset


def find_component(cdo, component_name):
    for component in cdo.get_components_by_class(unreal.ActorComponent):
        if component.get_name() == component_name:
            return component
    raise RuntimeError(f"Missing component {component_name}")

weapon_base = LIB.load_asset(f"{ROOT}/BP_CSH_WeaponBase")
box_base = LIB.load_asset(f"{ROOT}/BP_CSH_WeaponBoxBase")
bullet_base = LIB.load_asset(f"{ROOT}/AK47/BP_CSH_Bullet")
if not weapon_base or not box_base or not bullet_base:
    raise RuntimeError("Existing CSH weapon base assets are missing")

water_bullet = make_blueprint("BP_CSH_WaterBullet", bullet_base.generated_class(), WATER_ROOT)
water_gun = make_blueprint("BP_CSH_WaterGun", weapon_base.generated_class(), WATER_ROOT)
water_box = make_blueprint("BP_CSH_WaterGunBox", box_base.generated_class(), ROOT)

gun_mesh_asset = LIB.load_asset("/Game/Art/CSH/Weapons/Super_Soaker/supersoakerv1/StaticMeshes/supersoakerv1")
water_material = LIB.load_asset("/Game/Art/CSH/Weapons/Sci-Fi_Sniper/sci_fi_gun/Materials/blue")
box_mesh_asset = LIB.load_asset("/Engine/BasicShapes/Cube.Cube")
if not gun_mesh_asset or not water_material or not box_mesh_asset:
    raise RuntimeError("Required Super Soaker assets are missing")

bullet_cdo = unreal.get_default_object(water_bullet.generated_class())
bullet_cdo.set_editor_property("damage", 2.0)
bullet_cdo.set_editor_property("initial_speed", 2000.0)
bullet_cdo.set_editor_property("tracer_color", unreal.LinearColor(0.0, 0.45, 1.0, 1.0))
bullet_cdo.set_editor_property("tracer_light_intensity", 900.0)
bullet_cdo.set_editor_property("life_seconds", 2.0)
bullet_mesh = find_component(bullet_cdo, "BulletMesh")
bullet_mesh.set_editor_property("relative_scale3d", unreal.Vector(55.0, 0.28, 0.28))
bullet_mesh.set_material(0, water_material)
projectile_movement = find_component(bullet_cdo, "ProjectileMovement")
projectile_movement.set_editor_property("projectile_gravity_scale", 1.0)

weapon_cdo = unreal.get_default_object(water_gun.generated_class())
weapon_cdo.set_editor_property("weapon_display_name", unreal.Text("WATER GUN"))
weapon_cdo.set_editor_property("weapon_description", unreal.Text("CONTINUOUS WATER STREAM"))
weapon_cdo.set_editor_property("bullet_class", water_bullet.generated_class())
weapon_cdo.set_editor_property("fire_interval", 0.02)
weapon_cdo.set_editor_property("automatic", True)
weapon_cdo.set_editor_property("weapon_kick_distance", 0.0)
weapon_cdo.set_editor_property("camera_pitch_kick", 0.0)
weapon_cdo.set_editor_property("camera_yaw_kick", 0.0)
weapon_cdo.set_editor_property("muzzle_flash", None)
weapon_cdo.set_editor_property("fire_sound", None)
weapon_mesh = find_component(weapon_cdo, "WeaponMesh")
weapon_mesh.set_editor_property("static_mesh", gun_mesh_asset)
weapon_mesh.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=-90.0))
weapon_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.22, 0.22, 0.22))
muzzle = find_component(weapon_cdo, "MuzzlePoint")
muzzle.set_editor_property("relative_location", unreal.Vector(76.0, 0.0, 4.0))

box_cdo = unreal.get_default_object(water_box.generated_class())
box_cdo.set_editor_property("weapon_class", water_gun.generated_class())
box_mesh = find_component(box_cdo, "BoxMesh")
box_mesh.set_editor_property("static_mesh", box_mesh_asset)
box_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.7, 0.7, 0.45))
preview = find_component(box_cdo, "PreviewWeaponMesh")
preview.set_editor_property("static_mesh", gun_mesh_asset)
preview.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 80.0))
preview.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=20.0))
preview.set_editor_property("relative_scale3d", unreal.Vector(0.16, 0.16, 0.16))

for asset in (water_bullet, water_gun, water_box):
    LIB.save_loaded_asset(asset, only_if_is_dirty=False)

map_path = "/Game/CSH/Maps/InGame"
if LIB.does_asset_exist(map_path):
    LIB.load_asset(map_path)
    unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    if not any(actor.get_actor_label() == "CSH_WaterGun_WeaponBox" for actor in actors):
        starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
        location = unreal.Vector(-150.0, -300.0, 120.0)
        if starts:
            location = starts[0].get_actor_location() + unreal.Vector(-150.0, -300.0, 20.0)
        actor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
            water_box.generated_class(), location, unreal.Rotator()
        )
        actor.set_actor_label("CSH_WaterGun_WeaponBox")
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()

unreal.log("CSH continuous Water Gun created successfully.")