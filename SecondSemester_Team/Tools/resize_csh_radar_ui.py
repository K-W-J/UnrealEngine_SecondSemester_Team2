import unreal
path='/Game/CSH/Buleprint/UI/WBP_CSH_Radar'
bp=unreal.load_asset(path)
assert bp, path
cdo=unreal.get_default_object(bp.generated_class())
before=cdo.get_editor_property('radar_size')
cdo.set_editor_property('radar_size',320.0)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
assert cdo.get_editor_property('radar_size')==320.0
unreal.log_warning('RADAR_RESIZED %s -> 320; top-right anchor unchanged' % before)
