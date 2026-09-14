import unreal

LIB=unreal.EditorAssetLibrary
root="/Game/CSH/Buleprint/Weapons"
items=[
 ("EtherealBow","BP_CSH_EtherealBow","BP_CSH_EtherealArrow"),
 ("RPGLauncher","BP_CSH_RPGLauncher","BP_CSH_RPGRocket"),
 ("RotaryCannon","BP_CSH_RotaryCannon","BP_CSH_RotaryBullet"),
 ("SciFiSniper","BP_CSH_SciFiSniper","BP_CSH_SniperBullet"),
 ("StrelaLauncher","BP_CSH_StrelaLauncher","BP_CSH_StrelaMissile"),
 ("TemplarSword","BP_CSH_TemplarSword","BP_CSH_SwordSlash"),
]
for folder,weapon,bullet in items:
    wp=f"{root}/{folder}/{weapon}"; bp=f"{root}/{folder}/{bullet}"; box=f"{root}/{folder}/BP_CSH_{folder}Box"
    if not all(LIB.does_asset_exist(p) for p in (wp,bp,box)): raise RuntimeError(f"Missing asset in {folder}")
    cls=unreal.load_class(None,wp+"."+weapon+"_C"); cdo=unreal.get_default_object(cls)
    unreal.log_warning(f"VERIFY {folder}: ammo={cdo.get_editor_property('magazine_capacity')} infinite={cdo.get_editor_property('infinite_ammo')} melee={cdo.get_editor_property('melee_weapon')} bullet={cdo.get_editor_property('bullet_class')}")
unreal.EditorLoadingAndSavingUtils.load_map("/Game/CSH/Maps/InGame")
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
labels={a.get_actor_label() for a in actors}
for folder,_,_ in items:
    label="CSH_"+folder+"_WeaponBox"
    if label not in labels: raise RuntimeError(f"Missing InGame actor {label}")
unreal.log_warning("VERIFY COMPLETE: all weapons and InGame boxes exist")
