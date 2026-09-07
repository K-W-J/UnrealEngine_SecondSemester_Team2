import unreal
LIB=unreal.EditorAssetLibrary
box_bp=LIB.load_asset("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistolBox")
pistol_bp=LIB.load_asset("/Game/CSH/Buleprint/Weapons/SciFiPistol/BP_CSH_SciFiPistol")
unreal.log_warning("BOX_BP="+str(box_bp))
unreal.log_warning("PISTOL_BP="+str(pistol_bp))
if box_bp:
    cdo=unreal.get_default_object(box_bp.generated_class())
    unreal.log_warning("WEAPON_CLASS="+str(cdo.get_editor_property("weapon_class")))
    unreal.log_warning("DESTROY_AFTER="+str(cdo.get_editor_property("destroy_after_pickup")))
    for c in cdo.get_components_by_class(unreal.ActorComponent):
        unreal.log_warning("COMP="+c.get_name()+" CLASS="+c.get_class().get_name())
        if c.get_name()=="PickupTrigger":
            unreal.log_warning("EXTENT="+str(c.get_editor_property("box_extent")))
            unreal.log_warning("COLLISION_ENABLED="+str(c.get_editor_property("collision_enabled")))
for map_path in ("/Game/Maps/InGame","/Game/CSH/Maps/InGame"):
    if not LIB.does_asset_exist(map_path): continue
    unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    for a in actors:
        if "SciFiPistol" in a.get_actor_label():
            unreal.log_warning(f"MAP={map_path} ACTOR={a} LOC={a.get_actor_location()} CLASS={a.get_class().get_name()}")