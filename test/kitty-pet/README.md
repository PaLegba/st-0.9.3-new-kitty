# kitty_pet

`kitty_pet` is a small animated terminal pet for terminals that support the
kitty graphics protocol. It is derived from [xpet](https://github.com/uint/xpet),
but the X11/XPM implementation has been removed and replaced with a
terminal-native kitty graphics renderer.

The pet runs as a normal full-screen terminal application. It does not modify
or extend the terminal itself.

## Features

- independent C program with no X11 dependency;
- kitty graphics rendering with transparent sprites;
- smooth sub-cell movement using kitty placement `X`/`Y` pixel offsets;
- preloaded animation frames, so no image data is re-uploaded while walking;
- `neko`, `dog`, and `bsd` pets;
- idle, sleeping, walking, happy, and dragged states;
- mouse interaction: click to make the pet happy, drag it with the left button;
- optional screensaver mode that exits on the first input;
- low CPU and memory usage.

## Requirements

For building and running:

- a C99 compiler;
- a terminal with kitty graphics support, such as st built from this tree or
  kitty itself;
- `stb/stb_image.h` from the parent tree;
- `libm`.

No Xlib, Xpm, SDL, or external image library is needed.

For regenerating the PNG assets:

- Python 3;
- Pillow (`python3 -m pip install pillow`).

## Building

From this directory:

```sh
make
```

From the repository root:

```sh
make test/kitty-pet/kitty_pet
```

The build produces `./kitty_pet`.

## Running

Run the program inside an already running kitty-graphics-capable terminal:

```sh
./kitty_pet
./kitty_pet --pet dog
./kitty_pet --pet bsd
./kitty_pet --pet neko --screensaver
```

Do not normally launch it as `st -e ./kitty_pet`: when the pet exits, the
terminal window closes with it. If you want a dedicated full-screen instance,
use something like:

```sh
st -e sh -c './kitty_pet; exec sh'
```

## Options

| Option | Description |
| --- | --- |
| `--pet neko\|dog\|bsd` | Select the pet. Default: `neko`. |
| `--assets DIR` | Override the `pets-png` asset directory. |
| `--screensaver` | Exit on the first key or mouse event. |
| `-h`, `--help` | Show usage. |

## Controls

Normal mode:

| Input | Action |
| --- | --- |
| `q` | Quit. |
| `f` | Toggle chasing the mouse. |
| `s` | Freeze/unfreeze. |
| `h` or `Space` | Make the pet happy. |
| Left click on the pet | Make it happy. |
| Left-click and drag the pet | Drag it. |

In `--screensaver` mode any key or mouse event exits.

## How rendering works

At startup the program:

1. collects all PNG frames for the selected pet and state;
2. uploads them once as frames of a single kitty image (`a=t` and `a=f`);
3. enters the alternate screen and starts a 30 Hz update loop.

On each update:

- if the animation frame changed, it sends `a=a,c=<frame>`;
- if the pixel position changed, it sends `a=p` with the new cell position and
  `X`/`Y` offsets inside the first cell.

Because each frame is already stored in the terminal, moving the pet does not
retransmit image data. Only a small placement command is sent per movement step.

The PNG frames are padded with transparent space on the right and bottom. This
keeps the visible sprite inside the placement's cell box while the `X`/`Y`
offsets move it by up to one cell, avoiding visual trails and overhang
artifacts. A translucent outline is baked into the assets so mostly-black pets
remain readable on dark terminal backgrounds.

## Assets

Source sprites are kept as XPM files:

```
pets/<pet>/<state>/<frame>.xpm
```

Runtime assets are generated as padded PNG files:

```
pets-png/<pet>/<state>/<frame>.png
```

By default the program looks for `pets-png` next to its executable. Use
`--assets DIR` if the assets are stored elsewhere.

Regenerate them with:

```sh
make assets
```

The converter requires Python 3 and Pillow. It pads each image by 64 pixels on
the right and bottom and adds a subtle light outline. If you change the padding,
update `PET_PAD` in `kitty_pet.c` to match.

Animation states are:

```
idle, sleeping, happy, dragged,
walk_north, walk_south, walk_east, walk_west,
walk_northwest, walk_northeast, walk_southwest, walk_southeast
```

## Terminal size and offsets

The program queries the terminal cell size with `TIOCGWINSZ`. If the terminal
reports zero pixel dimensions, it falls back to the `CSI 16 t` query. Smooth
movement requires a terminal implementation that supports kitty placement
`X`/`Y` offsets. Current st with kitty graphics support implements them; older
st builds ignore them and movement will snap to the terminal cell grid.

## Notes

- The program is a full-screen terminal application, not an operating-system
  screensaver. It cannot draw over other applications.
- There is currently no speech bubble.
- Mouse coordinates reported by terminals are cell-based, so dragging is less
  precise than pixel-level sprite movement.
- The original xpet X11 implementation is not included in this directory.

## License

This program is derived from xpet and is distributed under the GNU General
Public License v3. See [`LICENSE`](LICENSE).
