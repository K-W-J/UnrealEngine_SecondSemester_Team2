import unreal
bp=unreal.load_asset('/Game/CSH/Buleprint/UI/WBP_CSH_Radar')
assert bp
cdo=unreal.get_default_object(bp.generated_class())
cdo.set_editor_property('show_walls',True)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
assert cdo.get_editor_property('show_walls')
unreal.log_warning('THIN_WALLS_ENABLED range=%s size=%s' % (cdo.get_editor_property('detection_range'),cdo.get_editor_property('radar_size')))
