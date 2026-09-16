import unreal
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/InGame2')
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=sub.get_all_level_actors()
points=[a for a in actors if 'boxpoint' in str(a.get_folder_path()).lower().split('/') or a.get_actor_label().lower().startswith('boxpoint')]
for point in points:
    unreal.log_warning('SPAWN_POINT '+point.get_actor_label()+' '+str(point.get_folder_path())+' '+point.get_class().get_name())
assert len(points)>=2, 'At least two BoxPoint actors required, found '+str(len(points))
lib=unreal.EditorAssetLibrary
folder='/Game/CSH/Buleprint/Weapons/RandomWeapon'
box=unreal.load_asset(folder+'/BP_CSH_RandomWeaponBox')
assert box
name='BP_CSH_RandomBoxSpawner'
path=folder+'/'+name
bp=lib.load_asset(path) if lib.does_asset_exist(path) else None
if not bp:
    factory=unreal.BlueprintFactory()
    factory.set_editor_property('parent_class',unreal.load_class(None,'/Script/SecondSemester_Team.CSHRandomBoxSpawner'))
    bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,folder,unreal.Blueprint,factory)
assert bp
cdo=unreal.get_default_object(bp.generated_class())
cdo.set_editor_property('box_class',box.generated_class())
assert lib.save_loaded_asset(bp,only_if_is_dirty=False)
managers=[a for a in actors if isinstance(a,unreal.CSHRandomBoxSpawner)]
assert len(managers)<=1, 'Multiple managers already placed; inspect before editing'
manager=managers[0] if managers else sub.spawn_actor_from_class(bp.generated_class(),unreal.Vector(0,0,0))
assert manager
manager.set_actor_label('CSH_RandomBoxSpawner')
manager.set_editor_property('box_class',box.generated_class())
manager.set_editor_property('box_points',points)
manager.set_editor_property('respawn_delay',.2)
manager.set_editor_property('spawn_offset',unreal.Vector(0,0,0))
assert len(manager.get_editor_property('box_points'))==len(points)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning('SPAWNER_READY InGame2 points=%d managers=1; one box at runtime; previous point excluded' % len(points))
