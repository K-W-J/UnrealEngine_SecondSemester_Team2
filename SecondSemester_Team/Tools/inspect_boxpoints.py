import unreal
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/InGame2')
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if 'boxpoint' in str(a.get_folder_path()).lower() or 'boxpoint' in a.get_actor_label().lower() or isinstance(a,unreal.TargetPoint):
        unreal.log_warning('POINT_FOUND '+a.get_actor_label()+' FOLDER '+str(a.get_folder_path())+' CLASS '+a.get_class().get_path_name()+' LOC '+str(a.get_actor_location()))
