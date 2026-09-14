import unreal
lib=unreal.EditorAssetLibrary
root='/Game/CSH/Buleprint/Weapons'
folder=root+'/TowSword'
def load(path):
    result=unreal.load_asset(path)
    assert result, path
    return result
def make(name,parent):
    path=folder+'/'+name
    if lib.does_asset_exist(path): return load(path)
    factory=unreal.BlueprintFactory()
    factory.set_editor_property('parent_class',parent)
    result=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,folder,unreal.Blueprint,factory)
    assert result
    return result
def setp(obj,**values):
    for key,value in values.items(): obj.set_editor_property(key,value)
lib.make_directory(folder)
weapon=make('BP_CSH_TowSword',unreal.load_class(None,'/Script/SecondSemester_Team.CSHTowSword'))
payload=make('BP_CSH_TowedCar',unreal.load_class(None,'/Script/SecondSemester_Team.CSHTowedCar'))
box=make('BP_CSH_TowSwordBox',load(root+'/BP_CSH_WeaponBoxBase').generated_class())
old=unreal.get_default_object(load(root+'/TemplarSword/BP_CSH_TemplarSword').generated_class())
wc=unreal.get_default_object(weapon.generated_class())
setp(wc,towed_car_class=payload.generated_class(),weapon_display_name=unreal.Text('PARKING ENFORCER'),
     weapon_description=unreal.Text('LMB: TOW / R: THROW'),max_cars=5,capture_range=650.0,
     capture_radius=150.0,held_scale=.12,stack_spacing=26.0,throw_speed=2000.0,
     impact_damage=150.0,impact_radius=450.0,infinite_ammo=True,fire_interval=.5)
for name in ['weapon_mesh','muzzle_point']:
    src=old.get_editor_property(name)
    dst=wc.get_editor_property(name)
    for prop in ['relative_location','relative_rotation','relative_scale3d']:
        dst.set_editor_property(prop,src.get_editor_property(prop))
    if name=='weapon_mesh': dst.set_editor_property('static_mesh',src.get_editor_property('static_mesh'))
pc=unreal.get_default_object(payload.generated_class())
rpg=unreal.get_default_object(load(root+'/RPGLauncher/BP_CSH_RPGRocket').generated_class())
setp(pc,impact_effect=rpg.get_editor_property('impact_explosion_effect'),effect_scale=3.0)
xc=unreal.get_default_object(box.generated_class())
setp(xc,weapon_class=weapon.generated_class(),pickup_radius=220.0,
     through_wall_glow_material=load('/Game/CSH/Materials/M_CSH_WeaponBoxWallsOnlyV2'))
setp(xc.get_editor_property('box_mesh'),static_mesh=load('/Engine/BasicShapes/Cube'),relative_scale3d=unreal.Vector(.7,.7,.45))
setp(xc.get_editor_property('preview_weapon_mesh'),static_mesh=old.get_editor_property('weapon_mesh').get_editor_property('static_mesh'),
     relative_location=unreal.Vector(0,0,80),relative_rotation=unreal.Rotator(yaw=20),relative_scale3d=unreal.Vector(.35,.35,.35))
for asset in [payload,weapon,box]: assert lib.save_loaded_asset(asset,only_if_is_dirty=False)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/CSH/Maps/InGame')
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=sub.get_all_level_actors()
players=[a for a in actors if a.get_class().get_path_name()=='/Game/CSH/Buleprint/BP_CSH_Player.BP_CSH_Player_C']
assert len(players)==1, 'Expected exactly one placed CSH player'
player=players[0]
player.set_editor_property('auto_possess_player',unreal.AutoReceiveInput.PLAYER0)
for cam in player.get_components_by_class(unreal.CameraComponent):
    is_death=cam.get_name()=='CSHDeathCamera'
    cam.set_editor_property('auto_activate',not is_death)
    cam.set_active(not is_death)
bp=load('/Game/CSH/Buleprint/BP_CSH_Player')
cdo=unreal.get_default_object(bp.generated_class())
for cam in cdo.get_components_by_class(unreal.CameraComponent):
    if cam.get_name()=='CSHDeathCamera': cam.set_editor_property('auto_activate',False)
assert lib.save_loaded_asset(bp,only_if_is_dirty=False)
label='CSH_TowSword_WeaponBox'
placed=next((a for a in actors if a.get_actor_label()==label),None)
if not placed:
    placed=sub.spawn_actor_from_class(box.generated_class(),player.get_actor_location()+unreal.Vector(450,1250,20))
    assert placed
    placed.set_actor_label(label)
setp(placed,weapon_class=weapon.generated_class(),pickup_radius=220.0)
assert placed.get_editor_property('weapon_class')==weapon.generated_class()
assert player.get_editor_property('auto_possess_player')==unreal.AutoReceiveInput.PLAYER0
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log_warning('TOW_READY: weapon, payload, pickup saved; InGame placed player possessed by Player0; death camera auto-activation disabled')
