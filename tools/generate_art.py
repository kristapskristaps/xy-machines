"""Generate the native-compatible retained artwork for Weave's flux loom."""
import math
from pathlib import Path

root = Path(__file__).resolve().parents[1]
lines = ['<g id="weave_panel">']
lines.append('<path d="M98 54 L98 46 L106 46 M374 46 L382 46 L382 54 M98 172 L98 180 L106 180 M374 180 L382 180 L382 172 M232 113 L248 113 M240 105 L240 121" fill="none" stroke="#303034" stroke-width="0.6"/>')
lines.append('<path d="M24 190 L456 190" fill="none" stroke="#2a2a2d" stroke-width="0.5"/>')
lines.append('<text transform="matrix(1 0 0 1 24 207)" fill="#f2f1ed" font-family="UniversNextPro-Light" font-size="14px">weave</text>')
lines.append('<text transform="matrix(1 0 0 1 413 40)" fill="#56565d" font-family="UniversNextPro-Light" font-size="8px">gate</text>')
lines.append('<circle cx="452" cy="37" r="3" fill="none" stroke="#303034" stroke-width="0.6"/>')
ticks = ' '.join(f'M{238 + i * 14} 204 L{244 + i * 14} 204' for i in range(15))
lines.append(f'<path d="{ticks}" fill="none" stroke="#56565d" stroke-width="1.5"/>')
for strand in range(4):
    points = [(50 * math.cos(2 * math.pi * i / 64), 24 * math.sin(2 * math.pi * i / 64)) for i in range(64)]
    path = ' '.join(f'{"M" if i == 0 else "L"}{x:.4f} {y:.4f}' for i, (x, y) in enumerate(points)) + ' Z'
    lines.append(f'<path id="weave_node_{strand}" d="{path}" transform="matrix(1 0 0 1 240 113)" fill="none" stroke="#cecdd2" stroke-width="0.6" stroke-linejoin="round"/>')
for node in range(4, 16):
    lines.append(f'<circle id="weave_node_{node}" cx="0" cy="0" r="1.6" transform="matrix(1 0 0 1 640 320)" fill="#ff8e83" stroke="#ff8e83" stroke-width="0.6"/>')
lines.append('<circle id="weave_node_16" cx="0" cy="0" r="42" transform="matrix(1 0 0 1 640 320)" fill="none" stroke="#ff3b38" stroke-width="0.6"/>')
lines.append('<path id="weave_node_17" d="M-4 -1 L4 -1 L4 1 L-4 1 Z" fill="#ff3b38" stroke="#ff3b38" stroke-width="0.6"/>')
lines.append('<circle id="weave_node_18" cx="0" cy="0" r="1.5" transform="matrix(1 0 0 1 640 320)" fill="#ff3b38" stroke="#ff3b38" stroke-width="0.6"/>')
colors = ['#000000', '#2a2a2d', '#56565d', '#919199', '#cecdd2', '#f2f1ed', '#702a2e', '#ff3b38', '#ff8e83']
for index, color in enumerate(colors):
    lines.append(f'<circle id="weave_palette_{index}" cx="640" cy="320" r="1" fill="{color}" stroke="{color}" stroke-width="0.6"/>')
lines.append('<text id="weave_text_0" fill="#919199" font-family="UniversNextPro-Light" font-size="10px">01 / 15</text>')
lines.append('<text id="weave_text_1" fill="#ff8e83" font-family="UniversNextPro-Light" font-size="12px">shaper</text>')
lines.append('</g>')
(root / 'panel.svg.fragment').write_text('\n'.join(lines) + '\n')
