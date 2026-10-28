#!/usr/bin/env python3
"""Render the actual C++ sample callback to stereo WAVs, without stock firmware."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]


def write_wav(path, audio, rate=44100):
    with wave.open(str(path), 'wb') as wav:
        wav.setnchannels(2)
        wav.setsampwidth(2)
        wav.setframerate(rate)
        wav.writeframes((np.clip(audio, -1, 1) * 32767).astype('<i2').tobytes())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--sdk', type=Path, default=ROOT.parent / 'open-xy')
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--sample', type=Path, help='Optional 16-bit PCM mono/stereo WAV, at most 882000 frames')
    args = parser.parse_args()
    out = args.out.resolve()
    if out == ROOT or ROOT in out.parents:
        parser.error('Keep generated output outside the machine repository')
    out.mkdir(parents=True, exist_ok=False)
    if args.sample:
        with wave.open(str(args.sample), 'rb') as wav:
            if wav.getsampwidth() != 2 or wav.getnchannels() not in (1, 2):
                parser.error('The audition input must be a 16-bit PCM mono/stereo WAV')
            rate, channels, frames = wav.getframerate(), wav.getnchannels(), wav.getnframes()
            if not 2 <= frames <= 882000 or not 8000 <= rate <= 192000:
                parser.error('Unsupported sample length or rate')
            source = np.frombuffer(wav.readframes(frames), dtype='<i2').reshape(-1, channels).astype(np.float64) / 32768
            if channels == 1:
                source = np.repeat(source, 2, axis=1)
    else:
        rate = 44100
        t = np.arange(rate * 3) / rate
        beat = t % 0.375
        root = np.array([130.8128, 155.5635, 174.6141, 146.8324])[np.minimum((t / 0.75).astype(int), 3)]
        source = np.empty((len(t), 2))
        for channel, detune in enumerate((1, 1.002)):
            tone = sum(np.sin(2 * np.pi * root * harmonic * detune * t) * amplitude
                       for harmonic, amplitude in ((1, 1), (1.5, .45), (2, .25), (2.378414, .2)))
            source[:, channel] = tone * .22 * np.exp(-5 * beat) * np.minimum(1, beat / .004)
    write_wav(out / 'source.wav', source, rate)
    source_pcm = (np.clip(source, -1, 1) * 32767).astype('<i2')
    (out / 'source.pcm').write_bytes(source_pcm.tobytes())
    binary = out / 'render'
    subprocess.run([args.cxx, '-std=c++11', '-O2', '-Wall', '-Werror', '-I' + str(args.sdk / 'sdk/include'),
                    str(ROOT / 'tools/render.cpp'), '-o', str(binary)], check=True)
    definition = json.loads((ROOT / 'definition.json').read_text())
    presets = dict(definition['presets'])
    presets['Octave'] = presets['Scan']
    metrics = {}
    for name, values in presets.items():
        controls = [round(values[p['id']] * 2147483647) for p in definition['parameters']]
        raw = subprocess.check_output([str(binary), str(out / 'source.pcm'), str(len(source)), str(rate),
            str(8 * 44100), str(50960740 if name == 'Octave' else 25480370), *map(str, controls)])
        audio = np.frombuffer(raw, dtype=np.float32).reshape(-1, 2)
        peak, rms = float(np.abs(audio).max()), float(np.sqrt(np.mean(audio * audio)))
        if not np.isfinite(audio).all() or rms < 1e-6 or peak > .50001:
            raise ValueError(f'{name}: silent, nonfinite or out-of-range output: peak={peak}, rms={rms}')
        write_wav(out / (name.lower() + '.wav'), audio)
        metrics[name] = dict(peak=peak, rms=rms, stereo_rms=float(np.sqrt(np.mean((audio[:, 0] - audio[:, 1]) ** 2))), controls=values)
        print(f'{name}: peak {peak:.4f}, RMS {rms:.4f}')
    hashes = {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(ROOT.rglob('*')) if path.is_file() and '.git' not in path.parts}
    (out / 'metrics.json').write_text(json.dumps(dict(sample_rate=rate, metrics=metrics, sources=hashes), indent=2) + '\n')
    print(out)


if __name__ == '__main__':
    main()
