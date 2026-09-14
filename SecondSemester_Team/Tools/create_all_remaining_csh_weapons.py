import unreal

TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
ROOT = "/Game/CSH/Buleprint/Weapons"

WEAPONS = [
    dict(folder="EtherealBow", weapon="BP_CSH_EtherealBow", bullet="BP_CSH_EtherealArrow", box="BP_CSH_EtherealBowBox",
         mesh="/Game/Art/CSH/Weapons/ethereal_recurve_bow/StaticMeshes/ethereal_recurve_bow",
         name="ETHEREAL BOW", desc="ARC SHOT / HEAVY ARROW", capacity=1, interval=0.75, automatic=False,
         damage=85.0, speed=3200.0, gravity=0.35, color=(0.2,0.85,1.0), bullet_shape="cube", bullet_scale=(0.65,0.025,0.025),
         weapon_scale=0.42, muzzle=(75,0,2), kick=4.0, pitch=0.45, pos=(450,-750,20)),
    dict(folder="RPGLauncher", weapon="BP_CSH_RPGLauncher", bullet="BP_CSH_RPGRocket", box="BP_CSH_RPGLauncherBox",
         mesh="/Game/Art/CSH/Weapons/PBR_Game_Ready_RPG_Launcher/pbr_game_ready_rpg_launcher/StaticMeshes/pbr_game_ready_rpg_launcher",
         name="RPG LAUNCHER", desc="EXPLOSIVE / HIGH KNOCKBACK", capacity=1, interval=1.15, automatic=False,
         damage=0.0, speed=2100.0, gravity=0.0, color=(1.0,0.22,0.02), bullet_shape="cone", bullet_scale=(0.32,0.09,0.09),
         explosive=True, explosion_damage=140.0, explosion_radius=480.0, knockback=150000.0,
         weapon_scale=0.38, muzzle=(95,0,1), kick=14.0, pitch=3.2, pos=(450,-500,20)),
    dict(folder="RotaryCannon", weapon="BP_CSH_RotaryCannon", bullet="BP_CSH_RotaryBullet", box="BP_CSH_RotaryCannonBox",
         mesh="/Game/Art/CSH/Weapons/rotarycannonfbx/StaticMeshes/rotarycannonfbx",
         name="ROTARY CANNON", desc="MINIGUN / EXTREME FIRE RATE", capacity=120, interval=0.045, automatic=True,
         damage=8.0, speed=6500.0, gravity=0.0, color=(1.0,0.4,0.03), bullet_shape="cube", bullet_scale=(0.25,0.018,0.018),
         weapon_scale=0.22, muzzle=(105,0,3), kick=2.0, pitch=0.16, pos=(450,-250,20)),
    dict(folder="SciFiSniper", weapon="BP_CSH_SciFiSniper", bullet="BP_CSH_SniperBullet", box="BP_CSH_SciFiSniperBox",
         mesh="/Game/Art/CSH/Weapons/Sci-Fi_Sniper/sci_fi_gun/StaticMeshes/sci_fi_gun",
         name="SCI-FI SNIPER", desc="RAIL SHOT / PIERCING", capacity=5, interval=1.0, automatic=False,
         damage=125.0, speed=15000.0, gravity=0.0, color=(0.1,0.75,1.0), bullet_shape="cube", bullet_scale=(0.6,0.018,0.018),
         piercing=True, penetrations=5, weapon_scale=0.32, muzzle=(110,0,1), kick=12.0, pitch=4.0, pos=(450,0,20)),
    dict(folder="StrelaLauncher", weapon="BP_CSH_StrelaLauncher", bullet="BP_CSH_StrelaMissile", box="BP_CSH_StrelaLauncherBox",
         mesh="/Game/Art/CSH/Weapons/Strela_missile_launcher/9k32_strela_missile_launcher/StaticMeshes/9k32_strela_missile_launcher",
         name="STRELA", desc="HOMING MISSILE / EXPLOSIVE", capacity=1, interval=1.4, automatic=False,
         damage=0.0, speed=1700.0, gravity=0.0, color=(0.15,1.0,0.35), bullet_shape="cone", bullet_scale=(0.38,0.08,0.08),
         explosive=True, explosion_damage=105.0, explosion_radius=380.0, knockback=90000.0, homing=True, homing_accel=9500.0,
         weapon_scale=0.28, muzzle=(105,0,0), kick=10.0, pitch=2.4, pos=(450,250,20)),
    dict(folder="TemplarSword", weapon="BP_CSH_TemplarSword", bullet="BP_CSH_SwordSlash", box="BP_CSH_TemplarSwordBox",
         mesh="/Game/Art/CSH/Weapons/templar_sword/StaticMeshes/templar_sword",
         name="TEMPLAR SWORD", desc="MELEE / WIDE CLEAVE", capacity=1, interval=0.55, automatic=False,
         damage=0.0, speed=2000.0, gravity=0.0, color=(0.65,0.85,1.0), bullet_shape="cube", bullet_scale=(0.01,0.01,0.01),
         melee=True, melee_damage=95.0, melee_range=260.0, melee_radius=105.0, melee_knockback=1200.0,
         weapon_scale=0.48, muzzle=(70,0,0), kick=8.0, pitch=0.0, pos=(450,500,20)),
]

def make_bp(name, parent, folder):
    path=f"{folder}/{name}"
    if LIB.does_asset_exist(path): return LIB.load_asset(path)
    LIB.make_directory(folder)
    factory=unreal.BlueprintFactory(); factory.set_editor_property("parent_class", parent)
    asset=TOOLS.create_asset(name, folder, unreal.Blueprint, factory)
    if not asset: raise RuntimeError(f"Failed to create {path}")
    return asset

def component(cdo, name):
    for item in cdo.get_components_by_class(unreal.ActorComponent):
        if item.get_name()==name: return item
    raise RuntimeError(f"Missing component {name}")

weapon_base=LIB.load_asset(f"{ROOT}/BP_CSH_WeaponBase")
box_base=LIB.load_asset(f"{ROOT}/BP_CSH_WeaponBoxBase")
bullet_native=unreal.load_class(None,"/Script/SecondSemester_Team.CSHBullet")
if not weapon_base or not box_base or not bullet_native: raise RuntimeError("CSH base assets missing")
cube=LIB.load_asset("/Engine/BasicShapes/Cube.Cube")
cone=LIB.load_asset("/Engine/BasicShapes/Cone.Cone")
box_mesh=LIB.load_asset("/Engine/BasicShapes/Cube.Cube")
muzzle_fx=LIB.load_asset("/Game/Art/CSH/NW_MuzzleFX/Particle_FX/FXS_NS_MuzzleFlash_02_v2")
created=[]

for data in WEAPONS:
    folder=f"{ROOT}/{data['folder']}"
    bullet=make_bp(data['bullet'],bullet_native,folder)
    weapon=make_bp(data['weapon'],weapon_base.generated_class(),folder)
    box=make_bp(data['box'],box_base.generated_class(),folder)
    mesh=LIB.load_asset(data['mesh'])
    if not mesh: raise RuntimeError(f"Missing mesh {data['mesh']}")

    bcdo=unreal.get_default_object(bullet.generated_class())
    bcdo.set_editor_property("damage",data['damage']); bcdo.set_editor_property("initial_speed",data['speed'])
    bcdo.set_editor_property("gravity_scale",data['gravity']); bcdo.set_editor_property("collision_radius",6.0 if data.get('explosive') else 2.0)
    bcdo.set_editor_property("tracer_color",unreal.LinearColor(*data['color'],1.0)); bcdo.set_editor_property("tracer_light_intensity",1800.0)
    bcdo.set_editor_property("life_seconds",6.0 if data.get('homing') else 4.0)
    bcdo.set_editor_property("explosive",data.get('explosive',False)); bcdo.set_editor_property("explosion_damage",data.get('explosion_damage',80.0))
    bcdo.set_editor_property("explosion_radius",data.get('explosion_radius',300.0)); bcdo.set_editor_property("knockback_impulse",data.get('knockback',0.0))
    bcdo.set_editor_property("piercing",data.get('piercing',False)); bcdo.set_editor_property("max_penetrations",data.get('penetrations',3))
    bcdo.set_editor_property("homing",data.get('homing',False)); bcdo.set_editor_property("homing_acceleration",data.get('homing_accel',8000.0))
    bullet_comp=component(bcdo,"BulletMesh"); bullet_comp.set_editor_property("static_mesh",cube if data['bullet_shape']=="cube" else cone)
    bullet_comp.set_editor_property("relative_scale3d",unreal.Vector(*data['bullet_scale']))
    if data['bullet_shape']=="cone": bullet_comp.set_editor_property("relative_rotation",unreal.Rotator(0.0,90.0,0.0))

    wcdo=unreal.get_default_object(weapon.generated_class())
    wcdo.set_editor_property("weapon_display_name",unreal.Text(data['name'])); wcdo.set_editor_property("weapon_description",unreal.Text(data['desc']))
    wcdo.set_editor_property("bullet_class",None if data.get('melee') else bullet.generated_class())
    wcdo.set_editor_property("magazine_capacity",data['capacity']); wcdo.set_editor_property("infinite_ammo",data.get('melee',False))
    wcdo.set_editor_property("fire_interval",data['interval']); wcdo.set_editor_property("automatic",data['automatic'])
    wcdo.set_editor_property("weapon_kick_distance",data['kick']); wcdo.set_editor_property("camera_pitch_kick",data['pitch'])
    wcdo.set_editor_property("camera_yaw_kick",0.18 if data['automatic'] else 0.35)
    wcdo.set_editor_property("melee_weapon",data.get('melee',False)); wcdo.set_editor_property("melee_damage",data.get('melee_damage',65.0))
    wcdo.set_editor_property("melee_range",data.get('melee_range',240.0)); wcdo.set_editor_property("melee_radius",data.get('melee_radius',85.0))
    wcdo.set_editor_property("melee_knockback",data.get('melee_knockback',900.0))
    if muzzle_fx and not data['folder'] in ("EtherealBow","TemplarSword"): wcdo.set_editor_property("muzzle_flash",muzzle_fx)
    wm=component(wcdo,"WeaponMesh"); wm.set_editor_property("static_mesh",mesh); wm.set_editor_property("relative_rotation",unreal.Rotator(0.0,0.0,-90.0)); wm.set_editor_property("relative_scale3d",unreal.Vector(data['weapon_scale'],data['weapon_scale'],data['weapon_scale']))
    component(wcdo,"MuzzlePoint").set_editor_property("relative_location",unreal.Vector(*data['muzzle']))

    xcdo=unreal.get_default_object(box.generated_class()); xcdo.set_editor_property("weapon_class",weapon.generated_class())
    bm=component(xcdo,"BoxMesh"); bm.set_editor_property("static_mesh",box_mesh); bm.set_editor_property("relative_scale3d",unreal.Vector(.7,.7,.45))
    preview=component(xcdo,"PreviewWeaponMesh"); preview.set_editor_property("static_mesh",mesh); preview.set_editor_property("relative_location",unreal.Vector(0,0,80)); preview.set_editor_property("relative_rotation",unreal.Rotator(0,0,20)); preview.set_editor_property("relative_scale3d",unreal.Vector(data['weapon_scale']*.7,data['weapon_scale']*.7,data['weapon_scale']*.7))
    for asset in (bullet,weapon,box): LIB.save_loaded_asset(asset,only_if_is_dirty=False)
    created.append((data,box))

unreal.EditorLoadingAndSavingUtils.load_map("/Game/CSH/Maps/InGame")
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem); existing=actors.get_all_level_actors()
starts=[a for a in existing if isinstance(a,unreal.PlayerStart)]; origin=starts[0].get_actor_location() if starts else unreal.Vector()
changed=False
for data,box in created:
    label="CSH_"+data['folder']+"_WeaponBox"
    if any(a.get_actor_label()==label for a in existing): continue
    offset=data['pos']; location=origin+unreal.Vector(*offset)
    actor=actors.spawn_actor_from_class(box.generated_class(),location,unreal.Rotator())
    if actor: actor.set_actor_label(label); changed=True
if changed: unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning("All remaining CSH weapons, projectiles, boxes, and InGame placements are ready")
