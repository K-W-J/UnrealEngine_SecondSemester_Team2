"""Unreal Python: replace only TemplarSword/EtherealBow audio. --verify is read-only."""
from pathlib import Path
import sys
import wave
import unreal

root = Path(__file__).resolve().parents[1] / 'SourceAssets/Audio/SwordBow'
verify = '--verify' in sys.argv
dest = '/Game/CSH/Audio/External'
items = [
    ('TemplarSword/BP_CSH_TemplarSword', 'S_SwordSwish_Recorded', root/'swishes/swish-7.wav'),
    ('EtherealBow/BP_CSH_EtherealBow', 'S_BowRelease_Recorded', root/'BowRelease_Single.wav'),
]
lib = unreal.EditorAssetLibrary
if not verify:
    # The original contains two releases; extract the first without synthesizing new audio.
    with wave.open(str(root/'Archery_0549.wav'), 'rb') as source:
        rate = source.getframerate()
        source.setpos(int(.098*rate))
        frames = source.readframes(int(.245*rate))
        params = source.getparams()
    with wave.open(str(root/'BowRelease_Single.wav'), 'wb') as output:
        output.setparams(params)
        output.writeframes(frames)
    tasks = []
    for _, name, filename in items:
        task = unreal.AssetImportTask()
        for key, value in dict(filename=str(filename), destination_path=dest,
            destination_name=name, automated=True, replace_existing=True,
            save=True, factory=unreal.SoundFactory()).items():
            task.set_editor_property(key, value)
        tasks.append(task)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for weapon, name, _ in items:
    sound = unreal.load_asset(dest+'/'+name)
    bp = unreal.load_asset('/Game/CSH/Buleprint/Weapons/'+weapon)
    assert sound and bp
    cdo = unreal.get_default_object(bp.generated_class())
    if not verify:
        sound.set_editor_property('looping', False)
        sound.set_editor_property('attenuation_settings', unreal.load_asset('/Game/CSH/Audio/SA_CSH_Weapon'))
        cdo.set_editor_property('fire_sound', sound)
        cdo.set_editor_property('loop_fire_sound', False)
        assert lib.save_loaded_asset(sound, only_if_is_dirty=False)
        assert lib.save_loaded_asset(bp, only_if_is_dirty=False)
    assert cdo.get_editor_property('fire_sound') == sound
    assert not cdo.get_editor_property('loop_fire_sound')
    assert 0 < sound.get_editor_property('duration') < .5
    unreal.log_warning('SWORD_BOW_OK: '+weapon+' -> '+sound.get_path_name())
