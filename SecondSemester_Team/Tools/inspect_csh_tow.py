import unreal
unreal.EditorLoadingAndSavingUtils.load_map('/Game/CSH/Maps/InGame')
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in sub.get_all_level_actors():
    if isinstance(a, unreal.Pawn) or 'car' in a.get_class().get_name().lower() or 'vehicle' in a.get_class().get_name().lower() or isinstance(a,unreal.WorldSettings):
        unreal.log_warning('ACTOR '+a.get_actor_label()+' '+a.get_class().get_path_name())
        if isinstance(a,unreal.WorldSettings): unreal.log_warning('GAMEMODE '+str(a.get_editor_property('default_game_mode')))
for path in unreal.EditorAssetLibrary.list_assets('/Game/CSH', recursive=True):
    if 'Player' in path or 'GameMode' in path:
        obj=unreal.load_asset(path)
        if isinstance(obj,unreal.Blueprint):
            cdo=unreal.get_default_object(obj.generated_class())
            unreal.log_warning('BP '+path+' '+str(cdo.get_class()))
            if isinstance(cdo,unreal.Actor):
                for c in cdo.get_components_by_class(unreal.MeshComponent): unreal.log_warning('MESH '+c.get_name()+' visible='+str(c.get_editor_property('visible')))
            if isinstance(cdo,unreal.GameModeBase): unreal.log_warning('DEFAULTPAWN '+str(cdo.get_editor_property('default_pawn_class')))
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
unreal.log_warning('MODE '+str(world.get_world_settings().get_editor_property('default_game_mode')))
for a in sub.get_all_level_actors():
    if isinstance(a,unreal.Pawn): unreal.log_warning('POSSESS '+a.get_actor_label()+' '+str(a.get_editor_property('auto_possess_player')))
