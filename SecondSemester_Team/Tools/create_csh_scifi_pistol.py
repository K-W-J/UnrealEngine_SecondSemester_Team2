import unreal

TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
ROOT = "/Game/CSH/Buleprint/Weapons"
PISTOL_ROOT = f"{ROOT}/SciFiPistol"


def make_blueprint(name, parent_class, folder=ROOT):
    path = f"{folder}/{name}"
    existing = LIB.load_asset(path)
    if existing:
        unreal.log_warning(f"Using existing {path}")
        return existing
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
bullet_bp = LIB.load_asset(f"{ROOT}/AK47/BP_CSH_Bullet")
if not weapon_base or not box_base or not bullet_bp:
    raise RuntimeError("Existing CSH weapon base assets are missing")

pistol_bp = make_blueprint("BP_CSH_SciFiPistol", weapon_base.generated_class(), PISTOL_ROOT)
pistol_box_bp = make_blueprint("BP_CSH_SciFiPistolBox", box_base.generated_class())

pistol_mesh = LIB.load_asset("/Game/Art/CSH/Weapons/Sci-Fi_Pistol/StaticMeshes/scene")
box_mesh = LIB.load_asset("/Engine/BasicShapes/Cube.Cube")
muzzle_fx = LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Particle_FX/FXS_Pistol_MuzzleFlash")
fire_sound = LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Sound/Sfx/SC_Shot_001")
if not pistol_mesh or not box_mesh or not muzzle_fx:
    raise RuntimeError("Required Sci-fi pistol assets are missing")

pistol_cdo = unreal.get_default_object(pistol_bp.generated_class())
pistol_cdo.set_editor_property("bullet_class", bullet_bp.generated_class())
pistol_cdo.set_editor_property("fire_interval", 0.22)
pistol_cdo.set_editor_property("automatic", False)
pistol_cdo.set_editor_property("weapon_kick_distance", 4.0)
pistol_cdo.set_editor_property("weapon_recovery_speed", 28.0)
pistol_cdo.set_editor_property("camera_pitch_kick", 0.0)
pistol_cdo.set_editor_property("camera_yaw_kick", 0.0)
pistol_cdo.set_editor_property("muzzle_flash", muzzle_fx)
if fire_sound:
    pistol_cdo.set_editor_property("fire_sound", fire_sound)

weapon_mesh = find_component(pistol_cdo, "WeaponMesh")
weapon_mesh.set_editor_property("static_mesh", pistol_mesh)
weapon_mesh.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=-90.0))
weapon_mesh.set_editor_property("relative_scale3d", unreal.Vector(0.35, 0.35, 0.35))

muzzle = find_component(pistol_cdo, "MuzzlePoint")
muzzle.set_editor_property("relative_location", unreal.Vector(36.0, 0.0, 5.0))

box_cdo = unreal.get_default_object(pistol_box_bp.generated_class())
box_cdo.set_editor_property("weapon_class", pistol_bp.generated_class())
box_component = find_component(box_cdo, "BoxMesh")
box_component.set_editor_property("static_mesh", box_mesh)
box_component.set_editor_property("relative_scale3d", unreal.Vector(0.7, 0.7, 0.45))
preview = find_component(box_cdo, "PreviewWeaponMesh")
preview.set_editor_property("static_mesh", pistol_mesh)
preview.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 72.0))
preview.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=20.0))
preview.set_editor_property("relative_scale3d", unreal.Vector(0.25, 0.25, 0.25))

for asset in (pistol_bp, pistol_box_bp):
    LIB.save_loaded_asset(asset, only_if_is_dirty=False)

# Place one pickup in each in-game map that exists, without duplicating it.
for map_path in ("/Game/Maps/InGame", "/Game/CSH/Maps/InGame"):
    if not LIB.does_asset_exist(map_path):
        continue
    level = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    if not level:
        continue
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = actor_subsystem.get_all_level_actors()
    if any(actor.get_actor_label() == "CSH_SciFiPistol_WeaponBox" for actor in actors):
        unreal.log_warning(f"Pistol box already exists in {map_path}")
        continue
    starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
    location = unreal.Vector(150.0, 250.0, 100.0)
    if starts:
        location = starts[0].get_actor_location() + unreal.Vector(150.0, 250.0, 20.0)
    actor = actor_subsystem.spawn_actor_from_class(pistol_box_bp.generated_class(), location, unreal.Rotator())
    actor.set_actor_label("CSH_SciFiPistol_WeaponBox")
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
    unreal.log_warning(f"Placed pistol box in {map_path} at {location}")

unreal.log_warning("CSH Sci-fi pistol and pickup box created successfully")