"""SFX synthesis + processing of bundled NW_MuzzleFX samples (not model-authentic recordings).
Run export_csh_audio_sources.py in Unreal first. No externally downloaded recordings.
"""
from pathlib import Path
import math
import random
import struct
import wave

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'SourceAudio' / 'CSH'
RATE = 48000
EXTRA_SOUNDS = ['Hurt', 'HurtHeavy', 'Explosion']
WEAPONS = {
    'AK47/BP_CSH_AK47': 'AK47',
    'SciFiPistol/BP_CSH_SciFiPistol': 'SciFiPistol',
    'DoubleBarrelShotgun/BP_CSH_DoubleBarrelShotgun': 'Shotgun',
    'WaterGun/BP_CSH_WaterGun': 'WaterGun',
    'RPGLauncher/BP_CSH_RPGLauncher': 'RPG',
    'StrelaLauncher/BP_CSH_StrelaLauncher': 'Strela',
    'SciFiSniper/BP_CSH_SciFiSniper': 'Sniper',
    'RotaryCannon/BP_CSH_RotaryCannon': 'Rotary',
    'EtherealBow/BP_CSH_EtherealBow': 'Bow',
    'TemplarSword/BP_CSH_TemplarSword': 'Sword',
    'BulletLauncher/BP_CSH_BulletLauncher': 'BulletLauncher',
    'FeignDeath/BP_CSH_FeignDeath': 'FeignDeath',
    'TowSword/BP_CSH_TowSword': 'TowSword',
    'Fists/BP_CSH_Fists': 'Fists',
    'BananaPistol/BP_CSH_BananaPistol': 'Banana',
}


def firearm_source(speed):
    with wave.open(str(OUT / 'Reference' / 'SW_5_56_new_01_v2.wav'), 'rb') as source:
        assert source.getsampwidth() == 2
        channels, rate = source.getnchannels(), source.getframerate()
        raw = source.readframes(source.getnframes())
    samples = struct.unpack('<%dh' % (len(raw)//2), raw)
    mono = [sum(samples[i:i+channels])/(channels*32768) for i in range(0, len(samples), channels)]
    # Trim only leading silence, retain the recorded transient and environmental tail.
    onset = next((i for i, x in enumerate(mono) if abs(x) > .02), 0)
    mono = mono[max(0, onset-int(rate*.002)):]
    step = speed * rate / RATE
    result = []
    for i in range(int((len(mono)-1)/step)):
        position = i * step
        j = int(position)
        result.append(mono[j] + (mono[j+1]-mono[j]) * (position-j))
    return result


def synth(name, seed):
    rng = random.Random(seed)
    duration = {'WaterGun': 2.0, 'Rotary': .27, 'AK47': .72,
                'RPG': .72, 'Strela': .85, 'Sniper': .65,
                'Shotgun': .95, 'Explosion': 1.8, 'Hurt': .32, 'HurtHeavy': .45}.get(name, .34)
    data = [0.0] * int(RATE * duration)

    def tone(start, length, f0, f1, gain, decay=4, metallic=False):
        phase = 0.0
        for j in range(min(int(length * RATE), len(data) - int(start * RATE))):
            t = j / RATE
            u = t / length
            phase += math.tau * (f1 + (f0 - f1) * (1 - u) ** 2) / RATE
            value = math.sin(phase)
            if metallic:
                value += .25 * math.sin(phase * 2.71)
            env = min(1.0, t / .002) * math.exp(-decay * u) * min(1.0, (length-t)/.012)
            data[int(start * RATE) + j] += value * gain * env

    def noise(start, length, gain, decay=5, smooth=.3):
        low = 0.0
        for j in range(min(int(length * RATE), len(data) - int(start * RATE))):
            t = j / RATE
            low += smooth * (rng.uniform(-1, 1) - low)
            env = min(1.0, t / .001) * math.exp(-decay*t/length) * min(1.0, (length-t)/.012)
            data[int(start * RATE)+j] += low * gain * env

    def sample_layer(speed, gain, length):
        source = firearm_source(speed)
        limit = min(len(data), len(source), int(length*RATE))
        for i in range(limit):
            data[i] += source[i] * gain * min(1.0, (limit-i)/(RATE*.045))

    def swish(start, length, gain, brightness):
        low = body = 0.0
        for j in range(min(int(length*RATE), len(data)-int(start*RATE))):
            u = j/(length*RATE)
            # Blade accelerates into a sharp cutting peak, then trails off.
            envelope = (u/.32)**1.5 if u < .32 else math.exp(-8*(u-.32))
            envelope *= min(1.0, (1-u)*18)
            white = rng.uniform(-1, 1)
            low += (.08 + brightness * math.sin(math.pi*u))*(white-low)
            body += .035*(white-body)
            data[int(start*RATE)+j] += (low-body)*gain*envelope

    if name in ('AK47', 'Shotgun', 'Rotary'):
        sample_layer({'AK47': .88, 'Shotgun': .67, 'Rotary': .96}[name], 1.0, duration)
        # Reinforce the body without replacing the recorded attack with a pitched chirp.
        noise(.006, .2 if name != 'Shotgun' else .4, .32, 6, .05)
        if name == 'Shotgun':
            tone(.003, .18, 80, 55, .12, 6)
            noise(.035, .35, .25, 5, .2)
    elif name == 'BulletLauncher':
        heavy = name in ('Shotgun', 'BulletLauncher')
        noise(0, .05, 1.1, 5, .85)
        noise(.007, duration-.007, .9 if heavy else .5, 5, .14)
        tone(0, duration*.8, 155 if heavy else 240, 48 if heavy else 85, .65)
        tone(.025, .09, 1600, 1050, .10, 7, True)
        if heavy:
            noise(.07, .08, .3, 6, .7)
    elif name in ('SciFiPistol', 'Sniper', 'FeignDeath'):
        tone(0, duration*.9, 160 if name == 'Sniper' else 230, 48, .65, 4, True)
        tone(.005, duration*.75, 430, 120, .22, 5, True)
        noise(0, .08, .9, 6, .85)
        noise(.015, duration-.015, 1.0, 4, .18)
        # Saturated, irregular low-mid energy instead of the earlier high 'pew' sweep.
        for i in range(len(data)):
            t = i/RATE
            data[i] = math.tanh(data[i]*2.4) * (.83 + .17*math.sin(math.tau*73*t))
    elif name in ('RPG', 'Strela'):
        sample_layer(.7, .45, .12)
        tone(0, .18, 85, 42, .25)
        noise(.005, .16, 1.2, 4, .35)
        noise(.025, duration-.025, 1.5, 2, .1 if name == 'RPG' else .23)
    elif name == 'WaterGun':
        # Unpitched turbulent spray and irregular splash texture, no gun attack or sine droplets.
        low = slow = modulation = 0.0
        for i in range(len(data)):
            white = rng.uniform(-1, 1)
            low += .24*(white-low)
            slow += .018*(white-slow)
            modulation += .002*(rng.uniform(-1, 1)-modulation)
            data[i] = (low-.65*slow) * (.6+min(.5, abs(modulation)*8))
        overlap = int(.04*RATE)
        head, tail = data[:overlap], data[-overlap:]
        data[:overlap] = [tail[i]*(1-i/overlap)+head[i]*(i/overlap) for i in range(overlap)]
        data = data[:-overlap]
    elif name == 'Explosion':
        noise(0, .09, 1.2, 5, .85)
        noise(.005, 1.75, 2.0, 5, .04)
        noise(.01, 1.2, .9, 5, .2)
        tone(.002, .7, 65, 28, .55, 5)
        for start in (.12, .21, .33, .48, .71):
            noise(start, .15, .2, 6, .55)
    elif name == 'Banana':
        tone(0, .17, 620, 85, .8, 3)
        tone(.045, .20, 190, 70, .23, 3)
        noise(0, .045, .35, 5, .2)
    elif name == 'Bow':
        # Decaying plucked string, followed by the arrow's short air release.
        delay = [rng.uniform(-1, 1) for _ in range(int(RATE/185))]
        for i in range(len(data)):
            j = i % len(delay)
            value = delay[j]
            delay[j] = .493*(value+delay[(j+1) % len(delay)])
            data[i] += .55*value*math.exp(-i/(RATE*.13))
        swish(.008, .17, .65, .35)
    elif name == 'Sword':
        # Dry 'shhk' cutting stroke; no sci-fi oscillator or long metallic whistle.
        swish(0, .27, 1.8, .8)
        noise(.071, .045, .65, 5, .9)
        swish(.075, .13, .4, .55)
    elif name == 'TowSword':
        swish(0, .24, 1.0, .45)
        # Short chain/hook clatters instead of the regular sword's slicing sound.
        for start, frequency in ((.055, 740), (.093, 1100), (.14, 620), (.19, 950)):
            tone(start, .075, frequency, frequency, .23, 8, True)
            noise(start, .028, .4, 7, .7)
    elif name == 'Fists':
        # A missed punch is just air and fabric; do not fake a hit on every swing.
        swish(0, .19, 1.2, .18)
        noise(.015, .095, .2, 5, .05)
    else:  # Impact plus breath-like noise: intentionally not a synthetic spoken voice.
        heavy = name == 'HurtHeavy'
        tone(0, .25 if heavy else .17, 115 if heavy else 170, 42, .7, 5)
        noise(0, .07, .7, 5, .4)
        noise(.035, duration-.035, .6, 3, .055)

    peak = max(abs(x) for x in data)
    target = .60 if name in ('WaterGun', 'Rotary') else .72
    data = [x * target / max(peak, .001) for x in data]
    # Click-free boundaries and headroom for overlapping automatic fire.
    fade = int(.003 * RATE)
    if name != 'WaterGun':
        for i in range(fade):
            data[i] *= i / fade
            data[-1-i] *= i / fade
    assert all(math.isfinite(x) and abs(x) < .99 for x in data)
    return data


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for index, name in enumerate(list(WEAPONS.values()) + EXTRA_SOUNDS):
        data = synth(name, 8120 + index)
        path = OUT / ('S_CSH_' + name + '.wav')
        pcm = struct.pack('<%dh' % len(data), *(round(x*32767) for x in data))
        with wave.open(str(path), 'wb') as output:
            output.setparams((1, 2, RATE, 0, 'NONE', 'not compressed'))
            output.writeframes(pcm)
        with wave.open(str(path), 'rb') as check:
            assert check.getnframes() == len(data) and check.getframerate() == RATE
        print('%s: %.2fs, peak %.3f' % (name, len(data)/RATE, max(abs(x) for x in data)))


if __name__ == '__main__':
    main()
