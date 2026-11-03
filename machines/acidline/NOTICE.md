Acidline's 44.1 kHz ladder update equations and coefficient approximation are
adapted from Acid Rain, pinned revision 7f9c8dcf4483db06239cc0cb0ad715f0e82c2180:
https://github.com/frankoooooooo/acid-rain

Acid Rain attributes its TB_303 ladder to Robin Schmidt's Open303:
https://github.com/RobinSchmidt/Open303

Only that filter/coefficient design is reused. Acidline retains its own saved
controls, presets, band-limited oscillators, envelope, sub, distortion, damping,
stock pitch handling and display. This adaptation runs at 44.1 kHz without
Acidline's previous 2x filter/FIR path, and intentionally changes its response.

MIT License

Copyright (c) 2009 Robin Schmidt
Copyright (c) 2026 Open-XY contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
