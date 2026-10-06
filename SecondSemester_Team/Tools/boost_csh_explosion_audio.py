"""Increase only the shared weapon explosion SoundWave volume."""
import unreal

lib = unreal.EditorAssetLibrary
sound = unreal.load_asset('/Game/CSH/Audio/Effects/S_CSH_Explosion')
assert isinstance(sound, unreal.SoundWave)
targets = []
for path in lib.list_assets('/Game/CSH/Buleprint/Weapons', recursive=True):
    bp = unreal.load_asset(path)
    if not isinstance(bp, unreal.Blueprint):
        continue
    cdo = unreal.get_default_object(bp.generated_class())
    if isinstance(cdo, (unreal.CSHBullet, unreal.CSHTowedCar)):
        if cdo.get_editor_property('impact_explosion_sound') == sound:
            targets.append(bp.get_name())
assert all(name in targets for name in (
    'BP_CSH_RPGRocket', 'BP_CSH_StrelaMissile', 'BP_CSH_SniperBullet', 'BP_CSH_TowedCar'))
before = sound.get_editor_property('volume')
sound.set_editor_property('volume', 2.6)
assert lib.save_loaded_asset(sound, only_if_is_dirty=False)
assert abs(sound.get_editor_property('volume') - 2.6) < .001
unreal.log_warning('EXPLOSION_VOLUME_OK: {} -> {}; targets={}'.format(before, 2.6, ', '.join(targets)))
