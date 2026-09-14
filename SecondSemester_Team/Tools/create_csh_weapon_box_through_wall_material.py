import unreal

PACKAGE = "/Game/CSH/Materials"
NAME = "M_CSH_WeaponBoxThroughWallMasked"
PATH = f"{PACKAGE}/{NAME}"

material = unreal.EditorAssetLibrary.load_asset(PATH)
if not material:
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

# Rebuilding this tiny generated graph keeps repeated runs deterministic.
unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
emissive = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant3Vector, -260, -40
)
emissive.set_editor_property("constant", unreal.LinearColor(0.0, 5.0, 9.0, 1.0))
opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -260, 100
)
opacity.set_editor_property("r", 0.28)
is_first_person = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionSceneTexture, -260, 220
)
is_first_person.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_IS_FIRST_PERSON)
not_first_person = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionOneMinus, -40, 220
)
masked_opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionMultiply, 160, 100
)
unreal.MaterialEditingLibrary.connect_material_expressions(is_first_person, "Color", not_first_person, "Input")
unreal.MaterialEditingLibrary.connect_material_expressions(opacity, "", masked_opacity, "A")
unreal.MaterialEditingLibrary.connect_material_expressions(not_first_person, "", masked_opacity, "B")
unreal.MaterialEditingLibrary.connect_material_property(
    emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
)
unreal.MaterialEditingLibrary.connect_material_property(
    masked_opacity, "", unreal.MaterialProperty.MP_OPACITY
)

unreal.MaterialEditingLibrary.recompile_material(material)
unreal.EditorAssetLibrary.save_loaded_asset(material)
unreal.log(f"Created through-wall weapon box glow material: {PATH}")
