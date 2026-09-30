import unreal
path='/Game/CSH/Buleprint/UI/WBP_CSH_Radar'
bp=unreal.load_asset(path)
assert bp,path
cdo=unreal.get_default_object(bp.generated_class())
cdo.set_editor_property('radar_size',400.0)
cdo.set_editor_property('show_walls',False)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
assert cdo.get_editor_property('radar_size')==400.0
assert not cdo.get_editor_property('show_walls')
unreal.log_warning('RADAR_UPDATED size=400 show_walls=False; enemy settings unchanged')
