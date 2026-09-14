import unreal
mesh = unreal.EditorAssetLibrary.load_asset("/Game/Art/CSH/Weapons/Sci-Fi_Pistol/StaticMeshes/scene")
unreal.log_warning("MESH=" + str(mesh))
unreal.log_warning("BOUNDS=" + str(mesh.get_bounds()))
unreal.log_warning("MATERIALS=" + str(mesh.get_editor_property("static_materials")))