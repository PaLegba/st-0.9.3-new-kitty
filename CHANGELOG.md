# Changelog

All notable changes to this modified st tree are documented here.

This is not an upstream st release. The upstream version remains `0.9.3`; the
changes below are local additions to that base.

## [0.9.3-kitty] - 2026-10-23

### Added

- Kitty graphics protocol support:
  - direct, file, temporary-file, and shared-memory image transmission;
  - raw `f=24` and `f=32` image data with optional zlib compression (`o=z`);
  - file decoding with `f=100`;
  - classic and Unicode placeholder placements;
  - placement `X`/`Y` pixel offsets within the first cell for sub-cell sprite
    positioning;
  - image cropping, scaling, alpha compositing, and XRender display;
  - animation frames, frame composition parameters, and terminal-driven
    animation control;
  - an on-disk image cache with configurable limits and automatic eviction;
  - restoring evicted frames from their original files when possible;
  - graphics debug mode, state dumping, image unloading, and image display
    toggling.
- Kitty keyboard protocol:
  - `CSI ? u` capability query;
  - flag setting, push, and pop (`CSI = ... u`, `CSI > ... u`, `CSI < ... u`);
  - press/release reporting when requested, modifier encoding, shifted
    alternate key codes, and the functional-key table.
- Box-drawing renderer:
  - U+2500..U+259F lines, blocks, shades, quadrants, and braille;
  - Unicode Symbols for Legacy Computing U+1FB00..U+1FB8B (sextants, eighth
    stripes, and mixed edge stripes).
- ANSI underline style and underline color support (`SGR 4:x`, `SGR 58`,
  `SGR 59`).
- A pure-color `default` palette and monochrome LED color schemes: `ledg`,
  `ledb`, `ledr`, `ledw`, `ledy`, `leda`, `ledc`, and `ledmg`.
- An optional `-R on|off` retro filter for kitty graphics images. It maps image
  pixels to the active color scheme using nearest RGB matching and luminance
  scaling. The default is `-R off`.
- XTVERSION and XTWINOPS support, including text-area pixel size and cell size
  queries.
- Window content alignment for windows whose size does not exactly match the
  terminal grid (`anysize_halign`, `anysize_valign`).
- Mouse and keyboard shortcuts for graphics operations.
- `test/kitty_demo.c`, a standalone kitty graphics protocol demo with:
  - a precomputed looping animation of primitives;
  - a real-time `--live` animation mode;
  - an `--images` mode for displaying image files;
  - cursor handling that leaves the shell prompt below the displayed image.
- `test/kitty-pet/kitty_pet.c`, a separate kitty-only animated pet adapted from
  xpet, with preloaded animation frames and smooth sub-cell sprite movement via
  kitty placement offsets.
- `README.md`, `CHANGELOG.md`, and `test/README.md`.

### Changed

- Made the build system portable between OpenBSD bmake and GNU make:
  - `config.mk` now uses `pkg-config` for zlib, fontconfig and freetype2 on
    both systems and no longer requires a Linux-only `-lrt`;
  - `CC = cc` is set explicitly so GNU make does not fall back to the
    non-existent `c99` on OpenBSD;
  - demo and pet rules no longer use non-portable `$<` in explicit targets.
- `test/kitty-pet` build rules are portable between bmake and GNU make:
  - C99/pedantic/warning flags live in `PETCFLAGS` so a command-line `CFLAGS`
    override does not drop them;
  - the top-level kitty-pet target has proper prerequisites and therefore
    rebuilds when `kitty_pet.c` or its `Makefile` changes.
- The `dist` tarball now includes `test/kitty-pet/` and its assets.
- Replaced Imlib2 with the vendored `stb/stb_image.h` for image decoding.
- Added an internal image representation and composition/scaling path for
  frames and placements.
- Image pixmaps are uploaded to X11 and scaled with XRender instead of through
  Imlib2.
- Added XRender and zlib to the link line and removed Imlib2 from the build.
- Extended `config.def.h` and the working `config.h` with graphics limits,
  window alignment, box drawing, and graphics shortcuts.
- Changed the default terminal identification response to the kitty-compatible
  `\033[?62c`.
- Updated `Makefile` dependencies and `dist` packaging for the new sources and
  headers.
- Improved large tty stream handling for graphics uploads.
- Renamed the `dark` color scheme to `ork`.

### Removed

- The Imlib2 build dependency and all Imlib2 API usage.
- The tracked generated `st.info.cdb` terminfo database, which the build and
  `dist` targets do not use.

### Fixed

- OpenBSD build fixes:
  - declare `pledge(2)` correctly and expose BSD interfaces such as
    `mkdtemp(3)` and `cfmakeraw(3)`;
  - fix sign-compare, unused-variable, unused-function and format-string
    warnings in `st.c`, `x.c` and `graphics.c`;
  - use `arc4random(3)` instead of `rand(3)` on OpenBSD to avoid the linker
    warning about deterministic values;
  - replace `strcat(3)` with bounded `snprintf(3)` in the image cache path;
  - fix `kitty-pet` signal/termios declarations;
  - fix an uninitialized `Arg` in `zoomreset()`.
- OpenBSD runtime graphics fixes:
  - extend the parent pledge to `stdio rpath wpath cpath tty proc exec`;
    without `wpath`/`cpath` the kitty graphics cache aborts `st` with
    `SIGABRT`, and `exec` is needed by the external preview and image-info
    helpers;
  - copy transmitted image files into the cache with a direct
    `open`/`read`/`write` loop instead of a symlink plus `system("cp ...")`.
- Ignore legacy DEC special graphics (`SO`, `ESC ( 0`, `ESC ) 0`) while
  synchronized output mode (`CSI ? 2026 h`) is active. This prevents a leaked
  character-set switch in application output from turning subsequent ASCII text
  into pseudo-graphics.
- Render bold Legacy Computing box-drawing characters from the correct table.
- Added cleanup paths for image buffers, composed frames, and scaled pixmaps in
  the stb_image-based implementation.
- Avoided color fringing when scaling transparent images by interpolating
  premultiplied alpha.

### Notes

- Box drawing is disabled by default. Set `boxdraw`, `boxdraw_bold`, and
  `boxdraw_braille` in `config.h` to enable it.
- Existing `config.h` files are not overwritten by `make`; merge new options
  from `config.def.h` after updating.
- Image format support is limited to what stb_image can decode. Formats that
  require an external loader, such as SVG, are not supported.
- Raw image formats are limited to `f=24` and `f=32`.
- Animation frame composition (`a=c`) and editing an existing frame in place
  (`r=` with `a=f`) are not implemented.
- The graphics cache is temporary and is placed under `/tmp` by default.
