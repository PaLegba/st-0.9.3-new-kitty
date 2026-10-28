# st with kitty graphics and box drawing

This tree is a modified version of [st](https://st.suckless.org/) 0.9.3. It
keeps st's small footprint and configuration model, while adding:

- the kitty graphics protocol, including animations and Unicode placeholders;
- image decoding through the vendored single-header library
  [`stb/stb_image.h`](stb/stb_image.h) instead of Imlib2;
- box-drawing rendering, including Unicode Symbols for Legacy Computing;
- a standalone C demo of the graphics protocol.

The original, minimal st README is still available as [`README`](README).

## Features

### Kitty graphics protocol

The graphics implementation lives in [`graphics.c`](graphics.c) and
[`graphics.h`](graphics.h). It is based on the st-graphics patch by Sergei
Grechanik, adapted to this tree and to stb_image.

Implemented functionality includes:

- image transmission over direct data (`t=d`), files (`t=f`), temporary files
  (`t=t`), and POSIX shared memory (`t=s`);
- raw `f=24` and `f=32` pixel data, optionally zlib-compressed with `o=z`;
- PNG, JPEG, GIF, BMP, TGA, PSD, HDR, PIC, and PNM decoding through stb_image
  when `f=100` is used;
- classic and Unicode placeholder placements;
- placement `X`/`Y` pixel offsets within the first cell, for sub-cell sprite
  positioning;
- image scaling, cropping, alpha compositing, and XRender-based display;
- animation frames (`a=f`) and animation control (`a=a`), including
  terminal-driven looping;
- an optional retro palette filter for images (`-R on`), using the active
  scheme with nearest-color mapping and luminance scaling;
- an on-disk image cache with size limits and automatic eviction;
- restoring evicted images from their original files when those files are still
  available;
- image info and preview actions.

Imlib2 is not required. stb_image is compiled directly into `graphics.o`.

### Box drawing

The tree includes the boxdraw v2 patch and the Legacy Computing extension:

- U+2500..U+259F line, block, quadrant, shade, and braille glyphs;
- U+1FB00..U+1FB8B sextants, eighth-column/row stripes, and mixed edge stripes.

Box drawing is disabled by default. Enable it in `config.h`:

```c
const int boxdraw = 1;
const int boxdraw_bold = 1;
const int boxdraw_braille = 1;
```

`boxdraw_bold` makes box lines bold when the glyph has the bold attribute.
`boxdraw_braille` renders braille as adjacent pixels instead of using the font.

### Other additions from the graphics work

- ANSI underline styles and underline colors (`SGR 4:x`, `SGR 58`, `SGR 59`);
- XTVERSION and XTWINOPS queries, including text-area size in pixels and in
  cells;
- window content alignment when the window size does not exactly match the
  terminal grid (`anysize_halign`, `anysize_valign`);
- improved handling of large graphics streams on the tty input path.

## Known limitations

- Raw image formats are limited to `f=24` and `f=32`; `f=8` and `f=16` are not
  supported.
- `f=100` decoding depends on stb_image. Formats that require external loaders,
  such as SVG, are not available.
- Animation frame composition (`a=c`) and editing an existing frame in place
  (`r=` with `a=f`) are not implemented. New animation frames should be
  uploaded with `a=f`.
- The graphics cache is local to the machine running st; shared-memory and file
  transmission still require the file/data to be accessible to the terminal
  process.

## Requirements

Build requirements:

- a C compiler and `make`;
- Xlib and Xft headers;
- fontconfig and freetype2;
- XRender (`libXrender`);
- zlib;
- `pkg-config`.

No Imlib2 development package is needed. stb_image is vendored in this tree.

## Building

```sh
make
```

`make` creates `config.h` from `config.def.h` only if `config.h` does not exist
yet. If you already have a `config.h`, it is used as-is; merge new options from
`config.def.h` manually after updating.

Build the graphics demo separately:

```sh
make test/kitty_demo
```

## Installing

The default installation prefix is `/usr/local`. Edit [`config.mk`](config.mk)
if needed, then run:

```sh
make clean install
```

Compile and install the terminfo entry if you did not install with
`make install`:

```sh
tic -sx st.info
```

Run st with `TERM=st-256color` or a compatible value.

## Configuration

Most options are in `config.h`. Graphics-specific options added by this tree
are:

| Option | Meaning |
| --- | --- |
| `graphics_cache_dir_template` | Template for the temporary image cache directory. |
| `graphics_max_single_image_file_size` | Maximum size of one cached image file, in bytes. |
| `graphics_total_file_cache_size` | Maximum total size of the on-disk image cache. |
| `graphics_max_single_image_ram_size` | Maximum RAM size of one image/frame or placement. |
| `graphics_max_total_ram_size` | Maximum total RAM used by loaded images and pixmaps. |
| `graphics_max_total_placements` | Maximum number of images/placements kept by the terminal. |
| `graphics_excess_tolerance_ratio` | How far limits may be exceeded before cleanup starts. |
| `graphics_animation_min_delay` | Minimum delay between animation redraws, in milliseconds. |

The image cache is created under `/tmp` by default. The cache directory is
removed when the graphics module is deinitialized.

### Color schemes

Select a color scheme with `-S NAME`, for example:

```sh
st -S ork
```

Available schemes include `default`, `light`, `plan9`, `dracula`, `solarized`,
`nord`, `gruvbox`, `cyberpunk`, `ork`, and the monochrome LED schemes `ledg`,
`ledb`, `ledr`, `ledw`, `ledy`, `leda`, `ledc`, and `ledmg`.

`default` uses pure RGB colors. `ork` is the dark green/gold/red palette. The
LED schemes use a black background and a 16-step intensity gradient of a single
color.

The active scheme can also be applied to kitty graphics images:

```sh
st -S ork -R on
```

`-R on` maps each image pixel to the nearest color in the active palette and
scales its intensity by luminance. `-R off` (the default) leaves image colors
untouched. The filter applies only to kitty graphics images; the terminal text
palette is selected independently with `-S`.

### Graphics key and mouse bindings

`TERMMOD` is `Ctrl+Shift` in the default configuration.

| Binding | Action |
| --- | --- |
| `TERMMOD+F1` | Cycle graphics debug mode. |
| `TERMMOD+F6` | Dump graphics state to stderr. |
| `TERMMOD+F7` | Unload images to reduce RAM usage. |
| `TERMMOD+F8` | Toggle image display. |
| `TERMMOD+Button3` | Preview the image under the mouse with `feh`. |
| `TERMMOD+Button2` (release) | Show information about the image under the mouse. |

The default terminal identification response (`vtiden`) is set to the
kitty-compatible `\033[?62c`. Change `vtiden` in `config.h` if your environment
needs a different DA response.

## Kitty graphics demo

A standalone, dependency-light demonstration is provided in
[`test/kitty_demo.c`](test/kitty_demo.c). See
[`test/README.md`](test/README.md) for full documentation.

```sh
make test/kitty_demo

# Run inside st, not as `st -e ./test/kitty_demo`:
./test/kitty_demo
./test/kitty_demo --live
./test/kitty_demo --images test/test1.jpg test/test2.png test/cozy_coffin.jpg
```

The default mode generates a looping animation of geometric primitives and
uploads it as kitty animation frames. `--live` generates and transmits frames in
real time. `--images` displays image files through the kitty file-transmission
mode.

## Testing with the recorded terminal session

The `test` directory contains:

- `test/test1.jpg`, `test/test2.png`, `test/cozy_coffin.jpg` — sample images;
- `test/typescript3` and `test/timing3` — a recorded kitty graphics session and
  its timing log;
- `test/kitty` — a snapshot of the original kitty repository and its tests.

The recorded session can be replayed with `scriptreplay`:

```sh
scriptreplay -t test/timing3 test/typescript3
```

It is a relatively demanding graphics stream. If replay at full speed overwhelms
the terminal's input buffer on your system, use the options supported by your
`scriptreplay` version to adjust the replay rate.

## Project layout

| Path | Purpose |
| --- | --- |
| `graphics.c`, `graphics.h` | Kitty graphics implementation and public interface. |
| `khash.h`, `kvec.h` | Hash-table and vector helpers used by the graphics cache. |
| `rowcolumn_diacritics_helpers.c` | Diacritic-to-row/column helpers for Unicode placeholders. |
| `boxdraw.c`, `boxdraw_data.h` | Box-drawing and Legacy Computing rendering. |
| `stb/stb_image.h` | Vendored image decoder. |
| `test/kitty_demo.c` | Standalone protocol demonstration. |
| `test/README.md` | Documentation for the demo. |
| `test/kitty-pet/` | Separate kitty-only animated pet adapted from xpet. |

## Credits

- st is maintained by the suckless community.
- The kitty graphics implementation is based on
  [st-graphics](https://github.com/sergei-grechanik/st-graphics) by Sergei
  Grechanik.
- stb_image is part of the [stb](https://github.com/nothings/stb) project by
  Sean Barrett and contributors.
- The boxdraw implementation is based on the st boxdraw patch by Avi Halachmi
  (`:avih`).
- The Legacy Computing extension is based on the boxdraw-legacycomputing patch
  by Miloš Stojanović.

## License

st is distributed under the MIT/X Consortium License; see [`LICENSE`](LICENSE).
stb_image is public domain or MIT-licensed, as stated in its header. The boxdraw
code carries its own MIT/X Consortium-style header.
