"""Run in Unreal Python; pass --verify to check saved assets without modifying them."""
import sys
from pathlib import Path
import unreal

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.dont_write_bytecode = True
from create_csh_audio import OUT, WEAPONS, EXTRA_SOUNDS

lib = unreal.EditorAssetLibrary
audio_root = '/Game/CSH/Audio'
weapon_root = '/Game/CSH/Buleprint/Weapons/'
verify = '--verify' in sys.argv


def props(obj, **values):
    for key, value in values.items():
        obj.set_editor_property(key, value)


def load(path):
    obj = unreal.load_asset(path)
    assert obj, path
    return obj


if not verify:
    tasks = []
    for name in list(WEAPONS.values()) + EXTRA_SOUNDS:
        task = unreal.AssetImportTask()
        props(task, filename=str(OUT / ('S_CSH_' + name + '.wav')),
              destination_path=audio_root + ('/Player' if name.startswith('Hurt') else '/Effects' if name == 'Explosion' else '/Weapons'),
              destination_name='S_CSH_' + name, automated=True,
              replace_existing=True, save=True, factory=unreal.SoundFactory())
        tasks.append(task)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    for task in tasks:
        assert task.get_editor_property('imported_object_paths'), task.get_editor_property('filename')

    attenuation_path = audio_root + '/SA_CSH_Weapon'
    attenuation = lib.load_asset(attenuation_path) if lib.does_asset_exist(attenuation_path) else unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        'SA_CSH_Weapon', audio_root, unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
    settings = attenuation.get_editor_property('attenuation')
    props(settings, attenuate=True, spatialize=True,
          attenuation_shape_extents=unreal.Vector(250, 0, 0), falloff_distance=4500.0)
    props(attenuation, attenuation=settings)
    assert lib.save_loaded_asset(attenuation, only_if_is_dirty=False)

for relative_path, name in WEAPONS.items():
    sound = load(audio_root + '/Weapons/S_CSH_' + name)
    bp = load(weapon_root + relative_path)
    cdo = unreal.get_default_object(bp.generated_class())
    if not verify:
        concurrency = sound.get_editor_property('concurrency_overrides')
        props(concurrency, max_count=3, voice_steal_release_time=0.02)
        props(sound, looping=name == 'WaterGun', override_concurrency=True, concurrency_overrides=concurrency,
              attenuation_settings=attenuation, volume=1.4)
        props(cdo, fire_sound=sound, fire_sound_volume=.48 if name in ('WaterGun', 'Rotary') else .7,
              fire_sound_min_interval=.065, loop_fire_sound=name == 'WaterGun')
        assert lib.save_loaded_asset(sound, only_if_is_dirty=False)
        assert lib.save_loaded_asset(bp, only_if_is_dirty=False)
    assert cdo.get_editor_property('fire_sound') == sound
    assert abs(sound.get_editor_property('volume') - 1.4) < .001
    assert sound.get_editor_property('duration') > 0
    assert sound.get_editor_property('looping') == (name == 'WaterGun')
    assert cdo.get_editor_property('loop_fire_sound') == (name == 'WaterGun')
    assert sound.get_editor_property('override_concurrency')
    assert sound.get_editor_property('concurrency_overrides').get_editor_property('max_count') == 3
    assert sound.get_editor_property('attenuation_settings')
    unreal.log_warning('CSH_AUDIO_WEAPON: ' + relative_path + ' -> ' + sound.get_path_name())

player = load('/Game/CSH/Buleprint/BP_CSH_Player')
pcdo = unreal.get_default_object(player.generated_class())
for prop, name in (('hurt_sound', 'Hurt'), ('heavy_hurt_sound', 'HurtHeavy')):
    sound = load(audio_root + '/Player/S_CSH_' + name)
    if not verify:
        concurrency = sound.get_editor_property('concurrency_overrides')
        props(concurrency, max_count=2, voice_steal_release_time=.025)
        props(sound, looping=False, override_concurrency=True, concurrency_overrides=concurrency, volume=1.3)
        pcdo.set_editor_property(prop, sound)
        assert lib.save_loaded_asset(sound, only_if_is_dirty=False)
    assert pcdo.get_editor_property(prop) == sound
    assert abs(sound.get_editor_property('volume') - 1.3) < .001
    assert sound.get_editor_property('duration') > 0
if not verify:
    assert lib.save_loaded_asset(player, only_if_is_dirty=False)
explosion = load(audio_root + '/Effects/S_CSH_Explosion')
if not verify:
    path = audio_root + '/SA_CSH_Explosion'
    explosion_attenuation = lib.load_asset(path) if lib.does_asset_exist(path) else unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        'SA_CSH_Explosion', audio_root, unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
    settings = explosion_attenuation.get_editor_property('attenuation')
    props(settings, attenuate=True, spatialize=True,
          attenuation_shape_extents=unreal.Vector(500, 0, 0), falloff_distance=10000.0)
    props(explosion_attenuation, attenuation=settings)
    assert lib.save_loaded_asset(explosion_attenuation, only_if_is_dirty=False)
    concurrency = explosion.get_editor_property('concurrency_overrides')
    props(concurrency, max_count=4, voice_steal_release_time=.05)
    props(explosion, looping=False, override_concurrency=True, concurrency_overrides=concurrency,
          attenuation_settings=explosion_attenuation, volume=1.3)
    assert lib.save_loaded_asset(explosion, only_if_is_dirty=False)
assert explosion.get_editor_property('duration') > 1
assert abs(explosion.get_editor_property('volume') - 1.3) < .001
assert explosion.get_editor_property('attenuation_settings')
explosion_targets = []
for path in lib.list_assets(weapon_root, recursive=True):
    bp = unreal.load_asset(path)
    if not isinstance(bp, unreal.Blueprint):
        continue
    cdo = unreal.get_default_object(bp.generated_class())
    is_explosive = isinstance(cdo, unreal.CSHBullet) and (
        cdo.get_editor_property('explosive') or cdo.get_editor_property('spawn_impact_explosion'))
    if is_explosive or isinstance(cdo, unreal.CSHTowedCar):
        if not verify:
            props(cdo, impact_explosion_sound=explosion)
            assert lib.save_loaded_asset(bp, only_if_is_dirty=False)
        assert cdo.get_editor_property('impact_explosion_sound') == explosion
        explosion_targets.append(bp.get_name())
assert all(name in explosion_targets for name in ('BP_CSH_RPGRocket', 'BP_CSH_StrelaMissile', 'BP_CSH_SniperBullet', 'BP_CSH_TowedCar'))
unreal.log_warning('CSH_AUDIO_EXPLOSIONS: ' + ', '.join(explosion_targets))
unreal.log_warning('CSH_AUDIO_OK: 15 weapons + 2 player hit sounds + explosion; ' + ('verified from disk' if verify else 'imported and saved'))
