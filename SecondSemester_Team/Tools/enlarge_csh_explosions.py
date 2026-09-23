import unreal

lib = unreal.EditorAssetLibrary
changed = 0
for path in lib.list_assets('/Game/CSH/Buleprint/Weapons', recursive=True):
    asset = unreal.load_asset(path)
    if not isinstance(asset, unreal.Blueprint):
        continue
    cdo = unreal.get_default_object(asset.generated_class())
    prop = None
    if isinstance(cdo, unreal.CSHBullet):
        if cdo.get_editor_property('explosive') or cdo.get_editor_property('spawn_impact_explosion'):
            prop = 'impact_explosion_scale'
    elif isinstance(cdo, unreal.CSHTowedCar):
        prop = 'effect_scale'
    if prop:
        before = cdo.get_editor_property(prop)
        after = before * 2.0
        cdo.set_editor_property(prop, after)
        assert lib.save_loaded_asset(asset, only_if_is_dirty=False)
        assert abs(cdo.get_editor_property(prop) - after) < .001
        unreal.log_warning('EXPLOSION_SIZE %s: %s -> %s' % (path, before, after))
        changed += 1
assert changed > 0
unreal.log_warning('EXPLOSIONS_ENLARGED %d assets; damage and radius unchanged' % changed)
