"""Trace the supplied helmet into the SDK's path-only SVG subset.

The white badge and checkerboard are excluded. Only the original mask is used.
Regeneration requires Pillow and NumPy; builds use the checked-in SVG.
"""
from pathlib import Path
import math
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]


def contours(bitmap):
    edges = {}
    def edge(a, b):
        edges.setdefault(a, []).append(b)
    height, width = bitmap.shape
    for y, x in zip(*np.where(bitmap)):
        x, y = int(x), int(y)
        if y == 0 or not bitmap[y - 1, x]: edge((x, y), (x + 1, y))
        if x == width - 1 or not bitmap[y, x + 1]: edge((x + 1, y), (x + 1, y + 1))
        if y == height - 1 or not bitmap[y + 1, x]: edge((x + 1, y + 1), (x, y + 1))
        if x == 0 or not bitmap[y, x - 1]: edge((x, y + 1), (x, y))
    result = []
    while edges:
        start = point = next(iter(edges))
        previous, loop = None, []
        while True:
            loop.append(point)
            choices = edges[point]
            if len(choices) > 1 and previous:
                dx, dy = point[0] - previous[0], point[1] - previous[1]
                choices.sort(key=lambda q: -(dx * (q[1] - point[1]) - dy * (q[0] - point[0])))
            following = choices.pop(0)
            if not choices: del edges[point]
            previous, point = point, following
            if point == start: break
        if len(loop) > 8: result.append(loop)
    return sorted(result, key=lambda loop: -len(loop))


def simplify(points, epsilon=0.7):
    if len(points) < 3: return points
    a, b = np.array(points[0], float), np.array(points[-1], float)
    delta = b - a
    length = np.dot(delta, delta)
    p = np.array(points, float)
    projected = a + np.clip(((p - a) @ delta) / max(length, 1), 0, 1)[:, None] * delta
    distances = np.sum((p - projected) ** 2, axis=1)
    index = int(np.argmax(distances))
    if distances[index] <= epsilon * epsilon: return [points[0], points[-1]]
    return simplify(points[:index + 1], epsilon)[:-1] + simplify(points[index:], epsilon)


def closed(points):
    # Split the ring at its most distant vertex before simplifying both arcs.
    at = int(np.argmax(np.sum((np.array(points) - np.array(points[0])) ** 2, axis=1)))
    return simplify(points[:at + 1])[:-1] + simplify(points[at:] + points[:1])[:-1]


image = np.array(Image.open(ROOT / 'artwork/mask-reference.png').convert('L'))
black = image[118:404, 224:518] < 110
loops = contours(black)
parts = [(ROOT / 'artwork/header.svg.fragment').read_text()]


def path(node, points, fill='none', stroke='#777777', width=0.6):
    if isinstance(points, str):
        data = points
    else:
        data = 'M' + ' L'.join(f'{x:.4f},{y:.4f}' for x, y in points) + ' Z'
    parts.append(f'<path id="vader_node_{node}" d="{data}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"/>')


def mask_points(loop):
    return [((x + 224 - 371) / 2, (y + 118 - 261) / 2) for x, y in closed(loop)]


# Everything around the mask is drawn first, so its silhouette stays legible.
for node in (25, 26, 27, 28, *range(60, 68)):
    segments = []
    for segment in range(4):
        points = []
        for radius, indices in ((70, range(6)), (69.4, reversed(range(6)))):
            for i in indices:
                angle = math.radians(segment * 90 + 8 + i * 7)
                points.append((radius * math.cos(angle), radius * math.sin(angle) * 0.91))
        segments.append('M' + ' L'.join(f'{x:.4f},{y:.4f}' for x, y in points) + ' Z')
    path(node, ' '.join(segments), stroke='#aaaaaa', width=0.45)
for node in range(68, 80):
    path(node, 'M-3,-0.4 L3,-0.4 L3,0.4 L-3,0.4 Z', '#dc3844', '#dc3844', 0.4)
for node in range(80, 86):
    path(node, 'M-3,0 L0,-2 L3,0 L0,2 Z', '#e7e7e7', '#e7e7e7', 0.4)
for node in range(2):
    points = [(-52 + i * 104 / 31, math.sin(i * math.tau / 31) * 18 - .4) for i in range(32)]
    points += [(x, y + .8) for x, y in reversed(points)]
    path(node, points, stroke='#dddddd', width=.5)

# Back only the open grille: filling the whole convex silhouette would turn
# the reference's empty space beneath the flared helmet into white wedges.
grille = [(325, 374), (363, 332), (363, 324), (367, 318), (375, 318), (379, 324), (379, 332), (417, 374)]
path(46, [((x - 371) / 2, (y - 261) / 2) for x, y in grille], '#c5c8ca', '#c5c8ca', .3)
for node, loop in ((47, 0), (48, 1), (49, 4), (50, 7)):
    path(node, mask_points(loops[loop]), '#050506', '#b5bfc8', .85 if node == 47 else .55)
for node, loop in ((51, 2), (52, 3), (53, 5), (54, 6), (55, 8), (56, 9), (57, 13), (12, 16)):
    path(node, mask_points(loops[loop]), '#c5c8ca', '#c5c8ca', .22)
for node, loop in zip(range(13, 18), (10, 11, 12, 14, 15)):
    path(node, mask_points(loops[loop]), '#050506', '#050506', .22)

# Eye overlays follow the reference's sloping almond shapes.
left = [(308, 274), (311, 267), (317, 255), (325, 255), (334, 259),
        (343, 268), (351, 278), (355, 283), (342, 284), (329, 282), (316, 278)]
for node, mirror in ((10, False), (11, True)):
    points = [(((742 - x if mirror else x) - 371) / 2, (y - 261) / 2) for x, y in left]
    path(node, points, '#8b9296', '#8b9296', .25)
for node in (58, 59):
    path(node, 'M-0.6,-1.5 L0.6,-1.5 L0.6,1.5 L-0.6,1.5 Z', '#ffdddd', '#ffdddd', .3)
for node in range(18, 24):
    path(node, 'M-0.45,0 L0.45,0 L0.45,-6 L-0.45,-6 Z', '#ba2937', '#ba2937', .25)
path(24, 'M-17,-0.4 L17,-0.4 L17,0.4 L-17,0.4 Z', '#9a2530', '#9a2530', .3)
for node in range(29, 33):
    path(node, 'M0,10 L0,0 L10,0 L10,1 L1,1 L1,10 Z', stroke='#444444', width=.4)
for node in range(33, 41):
    path(node, 'M-1,0 L1,0 L1,-5 L-1,-5 Z', '#555555', '#555555', .3)
for node in range(41, 45):
    path(node, 'M-0.5,0 L0.5,0 L0.5,-4 L-0.5,-4 Z', stroke='#777777', width=.4)
parts.append('<circle id="vader_node_45" cx="0" cy="0" r="1.5" fill="#990f1b" stroke="#990f1b" stroke-width="0.4"/>')
for x in (35, 341):
    parts.append(f'<path d="M{x},120 L{x + 104},120 L{x + 104},120.4 L{x},120.4 Z" fill="none" stroke="#202020" stroke-width="0.3"/>')
for text, x, y, size, fill in (('CLEAN SUB', 59, 85, 8, '#777777'), ('HARMONIC BODY', 356, 85, 8, '#777777'),
                             ('PHASE LOCKED', 54, 151, 7, '#444444'), ('BAND LIMITED', 361, 151, 7, '#444444'),
                             ('V A D E R', 202, 216, 15, '#c8c8c8'), ('808', 37, 212, 14, '#666666'),
                             ('SUB REACTOR', 361, 212, 9, '#666666')):
    # Native SVG text uses its matrix, not x/y attributes.
    parts.append(f'<text transform="matrix(1 0 0 1 {x} {y})" fill="{fill}" font-family="UniversNextPro-Light" font-size="{size}px">{text}</text>')
colors = ['#000000', '#141619', '#272b30', '#3c4248', '#525a61', '#6c757d', '#87929a', '#a1aab1',
          '#bec5ca', '#dde2e5', '#f5f5f5', '#49151e', '#81212d', '#c12b3c', '#f04052', '#ffe2e5']
for i, color in enumerate(colors):
    parts.append(f'<circle id="vader_palette_{i}" cx="640" cy="0" r="1" fill="{color}" stroke="{color}" stroke-width="0.45"/>')
parts.append('</g>')
(ROOT / 'panel.svg.fragment').write_text('\n'.join(parts) + '\n')
print(f'Traced {len(loops)} mask contours into {len("".join(parts))} bytes of native SVG.')
