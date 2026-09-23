import unreal

lib = unreal.EditorAssetLibrary
root = '/Game/CSH/Buleprint/Weapons'
path = root + '/RandomWeapon/BP_CSH_RandomWeaponBox'
weapons = []
for asset_path in sorted(lib.list_assets(root, recursive=True)):
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Blueprint):
        continue
    cls = asset.generated_class()
    cdo = unreal.get_default_object(cls)
    if isinstance(cdo, unreal.CSHWeaponBase) and asset.get_name() not in ('BP_CSH_WeaponBase','BP_CSH_Fists'):
        if cdo.get_editor_property('weapon_mesh').get_editor_property('static_mesh'):
            weapons.append(cls)
            unreal.log_warning('RANDOM_POOL: ' + cls.get_path_name())
assert weapons, 'No implemented weapons found'
bp = lib.load_asset(path) if lib.does_asset_exist(path) else None
if not bp:
    lib.make_directory(root + '/RandomWeapon')
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', unreal.load_asset(root + '/BP_CSH_WeaponBoxBase').generated_class())
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset('BP_CSH_RandomWeaponBox', root + '/RandomWeapon', unreal.Blueprint, factory)
assert bp
cdo = unreal.get_default_object(bp.generated_class())
def configure(obj):
    obj.set_editor_property('random_weapon', True)
    obj.set_editor_property('random_weapon_classes', weapons)
    obj.set_editor_property('weapon_class', None)
    obj.set_editor_property('pickup_radius', 220.0)
    obj.set_editor_property('destroy_after_pickup', True)
configure(cdo)
mesh = cdo.get_editor_property('box_mesh')
mesh.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
mesh.set_editor_property('relative_scale3d', unreal.Vector(.85,.85,.65))
cdo.get_editor_property('preview_weapon_mesh').set_static_mesh(None)
cdo.set_editor_property('through_wall_glow_material', unreal.load_asset('/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2'))
assert lib.save_loaded_asset(bp, only_if_is_dirty=False)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/CSH/Maps/InGame')
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
label = 'CSH_RandomWeaponBox'
placed = next((a for a in actors if a.get_actor_label() == label), None)
if not placed:
    player = next((a for a in actors if a.get_class().get_name() == 'BP_CSH_Player_C'), None)
    origin = player or next(a for a in actors if isinstance(a, unreal.PlayerStart))
    placed = sub.spawn_actor_from_class(bp.generated_class(), origin.get_actor_location() + unreal.Vector(450,-450,20))
    assert placed
    placed.set_actor_label(label)
configure(placed)
assert len(placed.get_editor_property('random_weapon_classes')) == len(weapons)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning('RANDOM_READY: %d equally likely weapons; %s' % (len(weapons), path))
