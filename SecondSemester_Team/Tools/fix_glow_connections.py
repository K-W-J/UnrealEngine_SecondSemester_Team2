import unreal
m = unreal.load_asset('/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2')
assert m
lib = unreal.MaterialEditingLibrary
def node(cls):
    return lib.create_material_expression(m, cls)
def connect(a, output, b, input_name):
    assert lib.connect_material_expressions(a, output, b, input_name), input_name
stencil = node(unreal.MaterialExpressionSceneTexture)
stencil.set_editor_property('scene_texture_id', unreal.SceneTextureId.PPI_CUSTOM_STENCIL)
mask = node(unreal.MaterialExpressionComponentMask)
mask.set_editor_property('r', True)
mask.set_editor_property('g', False)
mask.set_editor_property('b', False)
mask.set_editor_property('a', False)
connect(stencil, 'Color', mask, '')
def constant(value):
    n = node(unreal.MaterialExpressionConstant)
    n.set_editor_property('r', value)
    return n
select = node(unreal.MaterialExpressionIf)
connect(mask, '', select, 'A')
connect(constant(42), '', select, 'B')
connect(constant(0.32), '', select, 'A > B')
connect(constant(0), '', select, 'A == B')
connect(constant(0.32), '', select, 'A < B')
color = node(unreal.MaterialExpressionConstant3Vector)
color.set_editor_property('constant', unreal.LinearColor(0, 5, 9, 1))
assert lib.connect_material_property(color, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
assert lib.connect_material_property(select, '', unreal.MaterialProperty.MP_OPACITY)
lib.recompile_material(m)
assert unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log_warning('GLOW_FIXED: All material input connections verified and saved')
