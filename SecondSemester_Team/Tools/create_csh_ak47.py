import unreal

TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
ROOT = "/Game/CSH/Buleprint/Weapons"
AK_ROOT = f"{ROOT}/AK47"


def load_class(path):
    cls = unreal.load_class(None, path)
    if cls is None:
        raise RuntimeError(f"Missing class: {path}")
    return cls


def make_blueprint(name, parent_class, folder=ROOT):
    path = f"{folder}/{name}"
    existing = LIB.load_asset(path)
    if existing:
        unreal.log(f"Using existing {path}")
        return existing
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = TOOLS.create_asset(name, folder, unreal.Blueprint, factory)
    if asset is None:
        raise RuntimeError(f"Could not create {path}")
    return asset


def find_component(cdo, component_name):
    for component in cdo.get_components_by_class(unreal.ActorComponent):
        if component.get_name() == component_name:
            return component
    raise RuntimeError(f"Missing component {component_name} on {cdo.get_name()}")


LIB.make_directory(ROOT)
bullet_native = load_class("/Script/SecondSemester_Team.CSHBullet")
weapon_native = load_class("/Script/SecondSemester_Team.CSHWeaponBase")
box_native = load_class("/Script/SecondSemester_Team.CSHWeaponBox")

bullet_bp = make_blueprint("BP_CSH_Bullet", bullet_native, AK_ROOT)
weapon_base_bp = make_blueprint("BP_CSH_WeaponBase", weapon_native)
ak_bp = make_blueprint("BP_CSH_AK47", weapon_base_bp.generated_class(), AK_ROOT)
box_base_bp = make_blueprint("BP_CSH_WeaponBoxBase", box_native)
ak_box_bp = make_blueprint("BP_CSH_AK47Box", box_base_bp.generated_class())

ak_mesh = LIB.load_asset("/Game/Art/CSH/Weapons/AK47/ak47fbx/StaticMeshes/ak47fbx")
bullet_mesh = LIB.load_asset("/Game/Art/CSH/Weapons/LiteWeapons_JC/Models/SM_BulletLite_02")
bullet_material = LIB.load_asset("/Game/Art/CSH/Weapons/LiteWeapons_JC/Materials/MI_LiteWepons_Emissive")
box_mesh_asset = LIB.load_asset("/Engine/BasicShapes/Cube.Cube")
muzzle_fx = LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Particle_FX/FXS_NS_MuzzleFlash_02_v2")
fire_sound = LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Sound/Sfx/SW_5_56_new_01_v2")

for required_name, required_asset in (("AK mesh", ak_mesh), ("bullet mesh", bullet_mesh), ("box mesh", box_mesh_asset), ("muzzle FX", muzzle_fx)):
    if required_asset is None:
        raise RuntimeError(f"Missing {required_name}")

bullet_cdo = unreal.get_default_object(bullet_bp.generated_class())
bullet_mesh_component = find_component(bullet_cdo, "BulletMesh")
bullet_mesh_component.set_editor_property("static_mesh", bullet_mesh)
bullet_mesh_component.set_editor_property("relative_scale3d", unreal.Vector(30.0, 0.7, 0.7))
if bullet_material:
    bullet_mesh_component.set_material(0, bullet_material)

ak_cdo = unreal.get_default_object(ak_bp.generated_class())
ak_cdo.set_editor_property("bullet_class", bullet_bp.generated_class())
ak_cdo.set_editor_property("fire_interval", 0.1)
ak_cdo.set_editor_property("automatic", True)
ak_cdo.set_editor_property("weapon_kick_distance", 7.0)
ak_cdo.set_editor_property("weapon_recovery_speed", 22.0)
ak_cdo.set_editor_property("camera_pitch_kick", 0.7)
ak_cdo.set_editor_property("camera_yaw_kick", 0.25)
ak_cdo.set_editor_property("muzzle_flash", muzzle_fx)
if fire_sound:
    ak_cdo.set_editor_property("fire_sound", fire_sound)
ak_mesh_component = find_component(ak_cdo, "WeaponMesh")
ak_mesh_component.set_editor_property("static_mesh", ak_mesh)
ak_mesh_component.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=-90.0))
ak_mesh_component.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))
muzzle_component = find_component(ak_cdo, "MuzzlePoint")
muzzle_component.set_editor_property("relative_location", unreal.Vector(54.0, 0.0, 2.0))

ak_box_cdo = unreal.get_default_object(ak_box_bp.generated_class())
ak_box_cdo.set_editor_property("weapon_class", ak_bp.generated_class())
box_component = find_component(ak_box_cdo, "BoxMesh")
box_component.set_editor_property("static_mesh", box_mesh_asset)
box_component.set_editor_property("relative_scale3d", unreal.Vector(0.7, 0.7, 0.45))
preview_component = find_component(ak_box_cdo, "PreviewWeaponMesh")
preview_component.set_editor_property("static_mesh", ak_mesh)
preview_component.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 75.0))
preview_component.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=20.0))
preview_component.set_editor_property("relative_scale3d", unreal.Vector(0.75, 0.75, 0.75))

for asset in (bullet_bp, weapon_base_bp, ak_bp, box_base_bp, ak_box_bp):
    LIB.save_loaded_asset(asset, only_if_is_dirty=False)

level = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CSH/Maps/InGame")
if level is None:
    raise RuntimeError("Could not load /Game/CSH/Maps/InGame")
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
existing_boxes = [actor for actor in actors if actor.get_actor_label() == "CSH_AK47_WeaponBox"]
if not existing_boxes:
    starts = [actor for actor in actors if isinstance(actor, unreal.PlayerStart)]
    spawn_location = unreal.Vector(300.0, 0.0, 100.0)
    if starts:
        start_location = starts[0].get_actor_location()
        spawn_location = start_location + unreal.Vector(300.0, 0.0, 20.0)
    box_actor = actor_subsystem.spawn_actor_from_class(ak_box_bp.generated_class(), spawn_location, unreal.Rotator())
    box_actor.set_actor_label("CSH_AK47_WeaponBox")
    unreal.log(f"Placed AK box at {spawn_location}")
else:
    unreal.log("AK box already exists in the level")

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log("CSH AK47 weapon system assets created successfully")


