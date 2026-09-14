import unreal

root="/Game/CSH/Buleprint/Weapons"
folders=["EtherealBow","RPGLauncher","RotaryCannon","SciFiSniper","StrelaLauncher","TemplarSword"]
lib=unreal.EditorAssetLibrary
weapons={}
for folder in folders:
    weapon_name=f"BP_CSH_{folder}"
    weapon_path=f"{root}/{folder}/{weapon_name}"
    weapon_class=unreal.load_class(None,weapon_path+f".{weapon_name}_C")
    box_path=f"{root}/{folder}/BP_CSH_{folder}Box"
    box_class=unreal.load_class(None,box_path+f".BP_CSH_{folder}Box_C")
    if not weapon_class or not box_class: raise RuntimeError(f"Missing class for {folder}")
    box_cdo=unreal.get_default_object(box_class)
    box_cdo.set_editor_property("weapon_class",weapon_class)
    box_cdo.set_editor_property("pickup_radius",220.0)
    lib.save_asset(box_path,only_if_is_dirty=False)
    weapons["CSH_"+folder+"_WeaponBox"]=(weapon_class,box_class)

unreal.EditorLoadingAndSavingUtils.load_map("/Game/CSH/Maps/InGame")
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
found=set()
for actor in actors:
    label=actor.get_actor_label()
    if label not in weapons: continue
    weapon_class,box_class=weapons[label]
    actor.set_editor_property("weapon_class",weapon_class)
    actor.set_editor_property("pickup_radius",220.0)
    actor.modify()
    found.add(label)
    unreal.log_warning(f"REPAIRED {label}: weapon={actor.get_editor_property('weapon_class')}")
missing=set(weapons)-found
if missing: raise RuntimeError(f"Missing actors: {missing}")
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning("All new CSH weapon box instances repaired and saved")
