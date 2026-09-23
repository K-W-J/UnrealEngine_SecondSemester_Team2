import unreal
lib=unreal.EditorAssetLibrary
source=unreal.load_asset('/Game/RadarSystem/Blueprints/BP_InvisibleRadar')
cdo=unreal.get_default_object(source.generated_class())
for key in ['Range','Detection Mode','Detect Tag','Detected Object Types','Render Target','Start Enabled','Dots Color','Dot Radius','Rotations Per Seconds']:
    unreal.log_warning('RADAR_DEFAULT '+key+'='+str(cdo.get_editor_property(key)))
folder='/Game/CSH/Materials'
path=folder+'/M_CSH_RadarSystemUI'
mat=lib.load_asset(path) if lib.does_asset_exist(path) else lib.duplicate_asset('/Game/RadarSystem/Materials/M_Radar',path)
assert mat
mat.set_editor_property('material_domain',unreal.MaterialDomain.MD_UI)
mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_ADDITIVE)
unreal.MaterialEditingLibrary.recompile_material(mat)
assert lib.save_loaded_asset(mat,only_if_is_dirty=False)
unreal.log_warning('RADAR_UI_MATERIAL_READY')
