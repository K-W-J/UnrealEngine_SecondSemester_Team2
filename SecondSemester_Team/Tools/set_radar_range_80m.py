import unreal
path='/Game/CSH/Buleprint/UI/WBP_CSH_Radar'
bp=unreal.load_asset(path)
assert bp,path
cdo=unreal.get_default_object(bp.generated_class())
cdo.set_editor_property('detection_range',8000.0)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
assert cdo.get_editor_property('detection_range')==8000.0
unreal.log_warning('RADAR_RANGE_SAVED 80m; size=%s walls=%s' % (cdo.get_editor_property('radar_size'),cdo.get_editor_property('show_walls')))
