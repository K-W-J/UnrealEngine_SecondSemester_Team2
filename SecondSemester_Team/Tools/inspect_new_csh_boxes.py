import unreal

root="/Game/CSH/Buleprint/Weapons"
folders=["EtherealBow","RPGLauncher","RotaryCannon","SciFiSniper","StrelaLauncher","TemplarSword"]
lib=unreal.EditorAssetLibrary
for folder in folders:
    path=f"{root}/{folder}/BP_CSH_{folder}Box"
    bp=lib.load_asset(path); cls=unreal.load_class(None,path+f".BP_CSH_{folder}Box_C")
    cdo=unreal.get_default_object(cls)
    unreal.log_warning(f"BOX {folder}: class={cls} weapon={cdo.get_editor_property('weapon_class')} radius={cdo.get_editor_property('pickup_radius')}")
unreal.EditorLoadingAndSavingUtils.load_map("/Game/CSH/Maps/InGame")
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
for actor in actors:
    if actor.get_actor_label().startswith("CSH_") and actor.get_actor_label().endswith("_WeaponBox"):
        unreal.log_warning(f"ACTOR {actor.get_actor_label()}: class={actor.get_class()} loc={actor.get_actor_location()} weapon={actor.get_editor_property('weapon_class')}")
