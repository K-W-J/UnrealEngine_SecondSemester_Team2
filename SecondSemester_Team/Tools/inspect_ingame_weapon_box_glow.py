import unreal

MAP = "/Game/CSH/Maps/InGame.InGame"
if not unreal.EditorLoadingAndSavingUtils.load_map(MAP):
    raise RuntimeError(f"Could not load map: {MAP}")

actors = unreal.EditorLevelLibrary.get_all_level_actors()
boxes = []
for actor in actors:
    try:
        trigger = actor.get_editor_property("pickup_trigger")
        material = actor.get_editor_property("through_wall_glow_material")
        box_glow = actor.get_editor_property("box_glow_mesh")
        preview_glow = actor.get_editor_property("preview_weapon_glow_mesh")
    except Exception:
        continue
    boxes.append(actor)
    unreal.log(
        "INGAME_GLOW|"
        f"actor={actor.get_actor_label()}|class={actor.get_class().get_path_name()}|"
        f"material={material.get_path_name() if material else 'None'}|"
        f"box_glow_exists={bool(box_glow)}|"
        f"preview_glow_exists={bool(preview_glow)}|"
        f"box_mesh={box_glow.get_editor_property('static_mesh').get_path_name() if box_glow and box_glow.get_editor_property('static_mesh') else 'None'}|"
        f"preview_mesh={preview_glow.get_editor_property('static_mesh').get_path_name() if preview_glow and preview_glow.get_editor_property('static_mesh') else 'None'}"
    )

unreal.log(f"INGAME_GLOW_SUMMARY|map={MAP}|weapon_boxes={len(boxes)}")
if not boxes:
    raise RuntimeError("No CSH weapon boxes found in InGame map")
