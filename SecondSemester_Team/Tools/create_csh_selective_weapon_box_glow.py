import unreal

PACKAGE = "/Game/CSH/Materials"
NAME = "M_CSH_WeaponBoxThroughWallsOnly"
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
    material, unreal.MaterialExpressionConstant3Vector, -500, -120
)
emissive.set_editor_property("constant", unreal.LinearColor(0.0, 5.0, 9.0, 1.0))
opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 40
)
opacity.set_editor_property("r", 0.28)
custom_depth = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionSceneTexture, -500, 180
)
custom_depth.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_CUSTOM_DEPTH)
pixel_depth = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionPixelDepth, -500, 300
)
zero = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 420
)
zero.set_editor_property("r", 0.0)
one = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -500, 500
)
one.set_editor_property("r", 1.0)
depth_select = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionIf, -180, 260
)
masked_opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionMultiply, 100, 80
)

# If a player/held-weapon custom-depth pixel is closer than this glow pixel,
# hide the glow. Ordinary walls do not write custom depth, so they are ignored.
unreal.MaterialEditingLibrary.connect_material_expressions(custom_depth, "Color", depth_select, "A")
unreal.MaterialEditingLibrary.connect_material_expressions(pixel_depth, "", depth_select, "B")
unreal.MaterialEditingLibrary.connect_material_expressions(zero, "", depth_select, "ALessThanB")
unreal.MaterialEditingLibrary.connect_material_expressions(one, "", depth_select, "AGreaterThanB")
unreal.MaterialEditingLibrary.connect_material_expressions(one, "", depth_select, "AEqualsB")
unreal.MaterialEditingLibrary.connect_material_expressions(opacity, "", masked_opacity, "A")
unreal.MaterialEditingLibrary.connect_material_expressions(depth_select, "", masked_opacity, "B")
unreal.MaterialEditingLibrary.connect_material_property(
    emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
)
unreal.MaterialEditingLibrary.connect_material_property(
    masked_opacity, "", unreal.MaterialProperty.MP_OPACITY
)

unreal.MaterialEditingLibrary.recompile_material(material)
unreal.EditorAssetLibrary.save_loaded_asset(material)
unreal.log(f"Created selective through-wall glow material: {PATH}")
