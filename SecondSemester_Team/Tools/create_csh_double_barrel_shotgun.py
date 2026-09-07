import unreal

TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
ROOT = "/Game/CSH/Buleprint/Weapons"
SHOTGUN_ROOT = f"{ROOT}/DoubleBarrelShotgun"


def make_blueprint(name, parent_class, folder):
    path = f"{folder}/{name}"
    if LIB.does_asset_exist(path):
        return LIB.load_asset(path)
    LIB.make_directory(folder)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    result = TOOLS.create_asset(name, folder, unreal.Blueprint, factory)
    if not result:
        raise RuntimeError(f"Could not create {path}")
    return result


def find_component(cdo, name):
    for component in cdo.get_components_by_class(unreal.ActorComponent):
        if component.get_name() == name:
            return component
    raise RuntimeError(f"Missing component: {name}")

weapon_base = LIB.load_asset(f"{ROOT}/BP_CSH_WeaponBase")
box_base = LIB.load_asset(f"{ROOT}/BP_CSH_WeaponBoxBase")
bullet_base = LIB.load_asset(f"{ROOT}/AK47/BP_CSH_Bullet")
if not weapon_base or not box_base or not bullet_base:
    raise RuntimeError("CSH weapon base assets are missing")

shotgun_bullet = make_blueprint("BP_CSH_ShotgunBullet", bullet_base.generated_class(), SHOTGUN_ROOT)
shotgun = make_blueprint("BP_CSH_DoubleBarrelShotgun", weapon_base.generated_class(), SHOTGUN_ROOT)
shotgun_box = make_blueprint("BP_CSH_DoubleBarrelShotgunBox", box_base.generated_class(), ROOT)

shotgun_mesh_asset = LIB.load_asset("/Game/Art/CSH/Weapons/Double_Barrel_Shotgun/double_barrel_shotgun/StaticMeshes/double_barrel_shotgun")
box_mesh_asset = LIB.load_asset("/Engine/BasicShapes/Cube.Cube")
muzzle_fx = LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Particle_FX/FXS_NS_MuzzleFlash_06")
fire_sound = LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Sound/Sfx/SC_Shot_001")
if not shotgun_mesh_asset or not box_mesh_asset:
    raise RuntimeError("Double barrel shotgun assets are missing")

bullet_cdo = unreal.get_default_object(shotgun_bullet.generated_class())
bullet_cdo.set_editor_property("damage", 12.0)
bullet_cdo.set_editor_property("initial_speed", 2000.0)
bullet_cdo.set_editor_property("tracer_color", unreal.LinearColor(1.0, 0.25, 0.02, 1.0))
bullet_cdo.set_editor_property("tracer_light_intensity", 700.0)
bullet_cdo.set_editor_property("life_seconds", 1.5)
bullet_mesh = find_component(bullet_cdo, "BulletMesh")
bullet_mesh.set_editor_property("relative_scale3d", unreal.Vector(8.0, 0.35, 0.35))

shotgun_cdo = unreal.get_default_object(shotgun.generated_class())
shotgun_cdo.set_editor_property("weapon_display_name", unreal.Text("DOUBLE BARREL"))
shotgun_cdo.set_editor_property("weapon_description", unreal.Text("SHOTGUN / 8 PELLETS"))
shotgun_cdo.set_editor_property("bullet_class", shotgun_bullet.generated_class())
shotgun_cdo.set_editor_property("fire_interval", 0.8)
shotgun_cdo.set_editor_property("automatic", False)
shotgun_cdo.set_editor_property("projectiles_per_shot", 8)
shotgun_cdo.set_editor_property("spread_angle_degrees", 6.0)
shotgun_cdo.set_editor_property("weapon_kick_distance", 10.0)
shotgun_cdo.set_editor_property("weapon_recovery_speed", 18.0)
shotgun_cdo.set_editor_property("camera_pitch_kick", 2.2)
shotgun_cdo.set_editor_property("camera_yaw_kick", 0.45)
if muzzle_fx:
    shotgun_cdo.set_editor_property("muzzle_flash", muzzle_fx)
if fire_sound:
    shotgun_cdo.set_editor_property("fire_sound", fire_sound)
weapon_mesh = find_component(shotgun_cdo, "WeaponMesh")
weapon_mesh.set_editor_property("static_mesh", shotgun_mesh_asset)
weapon_mesh.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=-90.0))
weapon_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.28, 0.28, 0.28))
muzzle = find_component(shotgun_cdo, "MuzzlePoint")
muzzle.set_editor_property("relative_location", unreal.Vector(78.0, 0.0, 4.0))

box_cdo = unreal.get_default_object(shotgun_box.generated_class())
box_cdo.set_editor_property("weapon_class", shotgun.generated_class())
box_mesh = find_component(box_cdo, "BoxMesh")
box_mesh.set_editor_property("static_mesh", box_mesh_asset)
box_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.7, 0.7, 0.45))
preview = find_component(box_cdo, "PreviewWeaponMesh")
preview.set_editor_property("static_mesh", shotgun_mesh_asset)
preview.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 80.0))
preview.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=20.0))
preview.set_editor_property("relative_scale3d", unreal.Vector(0.20, 0.20, 0.20))

for asset in (shotgun_bullet, shotgun, shotgun_box):
    LIB.save_loaded_asset(asset, only_if_is_dirty=False)

map_path = "/Game/CSH/Maps/InGame"
unreal.EditorLoadingAndSavingUtils.load_map(map_path)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
if not any(actor.get_actor_label() == "CSH_DoubleBarrelShotgun_WeaponBox" for actor in actors):
    starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
    location = unreal.Vector(300.0, -250.0, 132.0)
    if starts:
        location = starts[0].get_actor_location() + unreal.Vector(300.0, -250.0, 20.0)
    actor = actor_subsystem.spawn_actor_from_class(shotgun_box.generated_class(), location, unreal.Rotator())
    if not actor:
        raise RuntimeError("Failed to place shotgun box")
    actor.set_actor_label("CSH_DoubleBarrelShotgun_WeaponBox")
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()

unreal.log("CSH Double Barrel Shotgun created successfully")