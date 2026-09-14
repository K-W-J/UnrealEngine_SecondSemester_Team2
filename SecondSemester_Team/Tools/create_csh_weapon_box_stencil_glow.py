import unreal

PACKAGE = "/Game/CSH/Materials"
NAME = "M_CSH_WeaponBoxWallsOnlyV2"
PATH = f"{PACKAGE}/{NAME}"

material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    NAME, PACKAGE, unreal.Material, unreal.MaterialFactoryNew()
)
if not material:
    raise RuntimeError(f"Could not create material: {PATH}")

material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
material.set_editor_property("disable_depth_test", True)
material.set_editor_property("two_sided", True)
material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)

emissive = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant3Vector, -500, -100
)
emissive.set_editor_property("constant", unreal.LinearColor(0.0, 5.0, 9.0, 1.0))
opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 40
)
opacity.set_editor_property("r", 0.32)
stencil = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionSceneTexture, -500, 180
)
stencil.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_CUSTOM_STENCIL)
player_stencil = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 300
)
player_stencil.set_editor_property("r", 42.0)
zero = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 420
)
zero.set_editor_property("r", 0.0)
one = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 500
)
one.set_editor_property("r", 1.0)
stencil_select = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionIf, -180, 260
)
masked_opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionMultiply, 100, 80
)

# Only stencil 42 (player and equipped weapon) blocks the glow. Weapon boxes
# write stencil 41, so they no longer hide their own glow.
unreal.MaterialEditingLibrary.connect_material_expressions(stencil, "Color", stencil_select, "A")
unreal.MaterialEditingLibrary.connect_material_expressions(player_stencil, "", stencil_select, "B")
unreal.MaterialEditingLibrary.connect_material_expressions(one, "", stencil_select, "ALessThanB")
unreal.MaterialEditingLibrary.connect_material_expressions(one, "", stencil_select, "AGreaterThanB")
unreal.MaterialEditingLibrary.connect_material_expressions(zero, "", stencil_select, "AEqualsB")
unreal.MaterialEditingLibrary.connect_material_expressions(opacity, "", masked_opacity, "A")
unreal.MaterialEditingLibrary.connect_material_expressions(stencil_select, "", masked_opacity, "B")
unreal.MaterialEditingLibrary.connect_material_property(
    emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
)
unreal.MaterialEditingLibrary.connect_material_property(
    masked_opacity, "", unreal.MaterialProperty.MP_OPACITY
)

unreal.MaterialEditingLibrary.recompile_material(material)
unreal.EditorAssetLibrary.save_loaded_asset(material)
unreal.log(f"Created stencil-filtered weapon box glow: {PATH}")
