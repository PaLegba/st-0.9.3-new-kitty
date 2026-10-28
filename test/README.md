# kitty_demo

`kitty_demo.c` is a standalone C program that demonstrates the kitty graphics
protocol supported by this st tree. It talks to the terminal by writing kitty
graphics escape sequences to stdout, so it does not depend on st's source code
or configuration.

The demo can:

- generate an animated scene of geometric primitives and upload it as a
  terminal-driven kitty animation;
- generate and transmit animation frames in real time;
- display image files using the kitty file-transmission mode.

## Requirements

- a C compiler and `make` (or compile the file directly);
- `libm`;
- zlib (`libz`).

The demo uses no Imlib2, X11, or stb_image APIs directly. It generates raw RGBA
frames itself and lets the terminal decode/display them.

## Building

From the repository root:

```sh
make test/kitty_demo
```

Or directly:

```sh
cc -O2 -o test/kitty_demo test/kitty_demo.c -lm -lz
```

## Running

Run the demo **inside an already running st window**:

```sh
./test/kitty_demo
```

Do not normally run it as `st -e ./test/kitty_demo`: when the demo exits, the
terminal window closes with it, so the animation disappears immediately.

## Modes

### Default: precomputed animation

With no arguments, the demo renders a six-second loop of animated primitives,
compresses every frame with zlib, uploads the frames as kitty animation frames,
and starts terminal-driven looping. The process exits after the upload; the
terminal keeps playing the animation.

```sh
./test/kitty_demo
```

The generated scene includes circles, lines, triangles, rings, gradients, sine
waves, and particle effects.

### Live animation

`--live` generates a frame, uploads it, sleeps for one frame interval, and
repeats. This mode exercises kitty's animation loading state and continuous
frame transmission.

```sh
./test/kitty_demo --live
```

The demo restarts the image automatically after 200 frames to avoid unbounded
growth of the terminal-side frame cache. By default the image is anchored at the
cursor position where the demo was started, clamped so that it fits on screen;
every restart returns to that anchor. With `--full` the placement is scaled
proportionally to fit the terminal, centered, with the remaining area left
black. Press `Ctrl+C` to stop it.

### Image display

`--images FILE...` displays one or more image files using the kitty file
transmission mode (`t=f`, `f=100`). The terminal copies the file into its image
cache and decodes it with stb_image.

```sh
./test/kitty_demo --images test/test1.jpg test/test2.png
./test/kitty_demo --images test/cozy_coffin.jpg --hold 10
```

Images are arranged in a horizontal strip. Each image is scaled to fit the
available terminal width (up to 32 cells per image) with a two-cell gap between
images. The cursor is hidden inside the image placements and is moved to the
bottom row after all images have been displayed.

## Options

| Option | Description |
| --- | --- |
| `--live` | Generate and upload frames continuously instead of precomputing a loop. |
| `--full` | With `--live`, fit the animation proportionally to the terminal area, centered, with black bars. |
| `--images FILE...` | Display the listed image files. |
| `--width N` | Animation width in pixels (default `320`). |
| `--height N` | Animation height in pixels (default `180`). |
| `--frames N` | Number of precomputed animation frames (default `120`). |
| `--fps N` | Animation frame rate (default `20`). |
| `--id N` | Base image id (default derived from the process id). |
| `--hold SEC` | Keep the process alive for `SEC` seconds after uploading. |
| `-h`, `--help` | Show usage information. |

Example:

```sh
./test/kitty_demo --width 480 --height 270 --frames 180 --fps 30
```

## How it works

For animation frames the demo:

1. renders a frame into a 32-bit RGBA buffer;
2. compresses it with zlib;
3. base64-encodes it in 3072-byte chunks, producing 4096-character protocol
   chunks;
4. sends the first frame with `a=T` and subsequent frames with `a=f`;
5. sets the root frame gap with `a=a,r=1,z=...`;
6. starts looping with `a=a,s=3`.

Animation frames use `f=32` and `o=z`. The first frame uses `a=T`; by default
the terminal moves the cursor after the placed image. The demo emits a newline
after the upload so the shell prompt appears below the image instead of over
it. All commands use `q=2` to suppress terminal responses, since the demo does
not read responses from the terminal.

For images the demo sends `a=T,f=100,t=f` with the base64-encoded absolute path
as the payload.

`--full` does not render frames at the terminal's pixel resolution. It keeps the
normal frame size and specifies only one placement dimension (`c` or `r`), so the
terminal infers the other dimension from the aspect ratio and does the final
scaling. The animation is centered on a cleared black background. This keeps CPU
and upload bandwidth low.

## Notes and troubleshooting

- The demo requires a terminal with kitty graphics support. In this tree that is
  st with `graphics.c` built in.
- The image is placed at the current cursor position. Run the demo from a clear
  area of the terminal if you want predictable placement.
- Uploading many large frames consumes terminal-side cache. The demo uses small
  frames (320x180 by default) to stay within the default graphics limits.
- If images do not appear, verify that st was built with the graphics module and
  that `test/kitty_demo` was run inside st rather than as the argument to
  `st -e`.
- In `--live` mode, the terminal may briefly show the previous frame while the
  next frame is being uploaded. This is expected for a loading-state animation.
