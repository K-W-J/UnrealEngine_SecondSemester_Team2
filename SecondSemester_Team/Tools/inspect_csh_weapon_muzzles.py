import unreal

root = "/Game/CSH/Buleprint/Weapons"
assets = unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)
for path in assets:
    if not path.rsplit("/", 1)[-1].startswith("BP_CSH_"):
        continue
    bp = unreal.EditorAssetLibrary.load_asset(path)
    if not isinstance(bp, unreal.Blueprint):
        continue
    try:
        cdo = unreal.get_default_object(bp.generated_class())
        mesh_comp = cdo.get_editor_property("weapon_mesh")
        muzzle = cdo.get_editor_property("muzzle_point")
    except Exception:
        continue
    mesh = mesh_comp.get_editor_property("static_mesh")
    if not mesh:
        continue
    unreal.log(
        f"MUZZLE|{path}|mesh={mesh.get_path_name()}|"
        f"mesh_loc={mesh_comp.get_editor_property('relative_location')}|"
        f"mesh_rot={mesh_comp.get_editor_property('relative_rotation')}|"
        f"mesh_scale={mesh_comp.get_editor_property('relative_scale3d')}|"
        f"muzzle_loc={muzzle.get_editor_property('relative_location')}|"
        f"bounds_origin={mesh.get_bounds().origin}|bounds_extent={mesh.get_bounds().box_extent}"
    )
