"""Connect the recorded sword swish to TowSword only, preserving its volume."""
import unreal

bp = unreal.load_asset('/Game/CSH/Buleprint/Weapons/TowSword/BP_CSH_TowSword')
sound = unreal.load_asset('/Game/CSH/Audio/External/S_SwordSwish_Recorded')
assert bp and sound
cdo = unreal.get_default_object(bp.generated_class())
volume = cdo.get_editor_property('fire_sound_volume')
cdo.set_editor_property('fire_sound', sound)
cdo.set_editor_property('loop_fire_sound', False)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
assert cdo.get_editor_property('fire_sound') == sound
assert cdo.get_editor_property('fire_sound_volume') == volume
unreal.log_warning('TOW_SWORD_AUDIO_OK: {} volume={}'.format(sound.get_path_name(), volume))
