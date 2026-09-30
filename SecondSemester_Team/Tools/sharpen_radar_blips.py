import unreal
path='/Game/CSH/Materials/M_CSH_RadarSystemUI'
mat=unreal.load_asset(path)
assert mat
edit=unreal.MaterialEditingLibrary
def node(name):
    result=unreal.load_object(None,path+'.M_CSH_RadarSystemUI:'+name)
    assert result,name
    return result
pulse=node('MaterialExpressionCollectionParameter_0')
dots=node('MaterialExpressionMultiply_6')
# Preserve the asset's timeline/MPC signal, shaping only the blip branch.
def create(cls,desc,x,y):
    name=desc.replace(' ','_')
    obj=unreal.find_object(None,path+'.M_CSH_RadarSystemUI:'+name)
    if obj: return obj
    result=edit.create_material_expression(mat,cls,x,y)
    result.rename(name)
    result.set_editor_property('desc',desc)
    return result
cut=create(unreal.MaterialExpressionSubtract,'CSH Blip Black Level',0,700)
cut.set_editor_property('const_b',.15)
contrast=create(unreal.MaterialExpressionMultiply,'CSH Blip Contrast',200,700)
contrast.set_editor_property('const_b',1.8)
clamp=create(unreal.MaterialExpressionClamp,'CSH Blip Clamp',400,700)
clamp.set_editor_property('min_default',0.0)
clamp.set_editor_property('max_default',1.0)
gain=create(unreal.MaterialExpressionMultiply,'CSH Blip Brightness',600,700)
gain.set_editor_property('const_b',3.0)
for src,out,dst,inp in [(pulse,'',cut,'A'),(cut,'',contrast,'A'),(contrast,'',clamp,''),(clamp,'',gain,'A'),(gain,'',dots,'B')]:
    assert edit.connect_material_expressions(src,out,dst,inp),(str(src),inp)
edit.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
unreal.log_warning('RADAR_BLIPS_SHARPENED: peak gain 3; contrast 1.8; black level .15; original scan timing retained')
