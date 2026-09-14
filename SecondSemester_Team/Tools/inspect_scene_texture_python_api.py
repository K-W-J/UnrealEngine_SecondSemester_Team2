import unreal

unreal.log("SCENE_TEXTURE_TYPES=" + repr([n for n in dir(unreal) if "SceneTexture" in n or "SceneAttribute" in n]))
unreal.log("SCENE_TEXTURE_EXPR=" + repr([n for n in dir(unreal.MaterialExpressionSceneTexture) if not n.startswith("_")]))
for enum_name in ("SceneTextureId", "MaterialSceneAttribute"):
    enum_type = getattr(unreal, enum_name, None)
    if enum_type:
        unreal.log(enum_name + "=" + repr([n for n in dir(enum_type) if n.startswith("PPI") or n.isupper()]))
