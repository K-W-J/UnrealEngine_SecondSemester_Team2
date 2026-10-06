from pathlib import Path
import unreal

out = Path(__file__).resolve().parents[1] / 'SourceAudio' / 'CSH' / 'Reference'
out.mkdir(parents=True, exist_ok=True)
root = '/Game/Art/CSH/NW_MuzzleFX/Sound/Sfx'
for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True):
    sound = unreal.load_asset(path)
    if not isinstance(sound, unreal.SoundWave):
        continue
    task = unreal.AssetExportTask()
    task.set_editor_property('object', sound)
    task.set_editor_property('filename', str(out / (sound.get_name()+'.wav')))
    task.set_editor_property('automated', True)
    task.set_editor_property('prompt', False)
    task.set_editor_property('replace_identical', True)
    assert unreal.Exporter.run_asset_export_task(task), path
    unreal.log_warning('AUDIO_SOURCE: %s duration=%s' % (path, sound.get_editor_property('duration')))
