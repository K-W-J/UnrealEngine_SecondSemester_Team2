import unreal
lib=unreal.EditorAssetLibrary
folder='/Game/CSH/Buleprint/Weapons/Fists'
path=folder+'/BP_CSH_Fists'
bp=lib.load_asset(path) if lib.does_asset_exist(path) else None
if not bp:
    lib.make_directory(folder)
    factory=unreal.BlueprintFactory()
    cls=unreal.load_class(None,'/Script/SecondSemester_Team.CSHFists')
    assert cls
    factory.set_editor_property('parent_class',cls)
    bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset('BP_CSH_Fists',folder,unreal.Blueprint,factory)
assert bp
cdo=unreal.get_default_object(bp.generated_class())
assert cdo.get_editor_property('infinite_ammo')
assert cdo.get_editor_property('melee_weapon')
assert lib.save_loaded_asset(bp,only_if_is_dirty=False)
unreal.log_warning('FISTS_READY damage=%s interval=%s range=%s; no projectile; starter weapon only' % (cdo.get_editor_property('melee_damage'),cdo.get_editor_property('fire_interval'),cdo.get_editor_property('melee_range')))
