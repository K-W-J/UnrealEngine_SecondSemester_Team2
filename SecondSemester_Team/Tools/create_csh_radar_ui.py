import unreal
path='/Game/CSH/Buleprint/UI/WBP_CSH_Radar'
lib=unreal.EditorAssetLibrary
asset=lib.load_asset(path) if lib.does_asset_exist(path) else None
if not asset:
    factory=unreal.WidgetBlueprintFactory()
    parent=unreal.load_class(None,'/Script/SecondSemester_Team.CSHRadarWidget')
    assert parent
    factory.set_editor_property('parent_class',parent)
    asset=unreal.AssetToolsHelpers.get_asset_tools().create_asset('WBP_CSH_Radar','/Game/CSH/Buleprint/UI',unreal.WidgetBlueprint,factory)
assert asset
cdo=unreal.get_default_object(asset.generated_class())
cdo.set_editor_property('detection_range',5000.0)
cdo.set_editor_property('radar_size',240.0)
assert cdo.get_editor_property('screen_texture')
assert lib.save_loaded_asset(asset,only_if_is_dirty=False)
unreal.log_warning('RADAR_READY '+path+' range=5000 size=240 texture='+str(cdo.get_editor_property('screen_texture')))
