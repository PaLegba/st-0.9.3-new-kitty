/* See LICENSE file for copyright and license details. */

/*
 * appearance
 *
 * font: see http://freedesktop.org/software/fontconfig/fontconfig-user.html
 */

static char *fonts[] = {
	"UbuntuMono:pixelsize=11:antialias=false:autohint=false",
	"Liberation Mono:pixelsize=12:antialias=true:autohint=true",
};
};

static size_t currentfont = 0;
static int borderpx = 2;

/* How to align the content in the window when the size of the terminal
 * doesn't match the size of the window. Percentages: 50 means center. */
static int anysize_halign = 50;
static int anysize_valign = 50;



/*
 * What program is execed by st depends of these precedence rules:
 * 1: program passed with -e
 * 2: scroll and/or utmp
 * 3: SHELL environment variable
 * 4: value of shell in /etc/passwd
 * 5: value of shell in config.h
 */
static char *shell = "/bin/bash";
char *utmp = NULL;
/* scroll program: to enable use a string like "scroll" */
char *scroll = NULL;
char *stty_args = "stty raw pass8 nl -echo -iexten -cstopb 38400";

/* identification sequence returned in DA and DECID */
/* By default, use the same one as kitty. */
char *vtiden = "\033[?62c";

/* Kerning / character bounding-box multipliers */
static float cwscale = 1.0;
static float chscale = 1.0;

/*
 * word delimiter string
 *
 * More advanced example: L" `'\"()[]{}"
 */
wchar_t *worddelimiters = L" ";

/* selection timeouts (in milliseconds) */
static unsigned int doubleclicktimeout = 300;
static unsigned int tripleclicktimeout = 600;

/* alt screens */
int allowaltscreen = 1;

/* allow certain non-interactive (insecure) window operations such as:
   setting the clipboard text */
int allowwindowops = 0;

/*
 * draw latency range in ms - from new content/keypress/etc until drawing.
 * within this range, st draws when content stops arriving (idle). mostly it's
 * near minlatency, but it waits longer for slow updates to avoid partial draw.
 * low minlatency will tear/flicker more, as it can "detect" idle too early.
 */
static double minlatency = 2;
static double maxlatency = 33;

/*
 * blinking timeout (set to 0 to disable blinking) for the terminal blinking
 * attribute.
 */
static unsigned int blinktimeout = 800;

/*
 * thickness of underline and bar cursors
 */
static unsigned int cursorthickness = 2;

/*
 * 1: render most of the lines/blocks characters without using the font for
 *    perfect alignment between cells (U2500 - U259F except dashes/diagonals).
 *    Bold affects lines thickness if boxdraw_bold is not 0. Italic is ignored.
 * 0: disable (render all U25XX glyphs normally from the font).
 */
const int boxdraw = 0;
const int boxdraw_bold = 0;

/* braille (U28XX):  1: render as adjacent "pixels",  0: use font */
const int boxdraw_braille = 0;

/*
 * bell volume. It must be a value between -100 and 100. Use 0 for disabling
 * it
 */
static int bellvolume = 0;

/* visual bell duration (in milliseconds) */
static unsigned int bellduration = 150;

/*
 * visual bell colors
 *
 * Formulas from normal colors to bell colors.
 * Bell colors are clipped between 0x0000 and 0xffff.
 */
#define BELLR(color)	(color.red   * 0.9 + 0xffff * 0.1)
#define BELLG(color)	(color.green * 0.9 + 0xffff * 0.1)
#define BELLB(color)	(color.blue  * 0.9 + 0xffff * 0.1)
#define BELLA(color)	(color.alpha - 0x0500)

/* default TERM value */
char *termname = "st-256color";

/*
 * spaces per tab
 *
 * When you are changing this value, don't forget to adapt the »it« value in
 * the st.info and appropriately install the st.info in the environment where
 * you use this st version.
 *
 *	it#$tabspaces,
 *
 * Secondly make sure your kernel is not expanding tabs. When running `stty
 * -a` »tab0« should appear. You can tell the terminal to not expand tabs by
 *  running following command:
 *
 *	stty tabs
 */
unsigned int tabspaces = 8;

/* loaded if couldn't find scheme, has to be present */
static const OptionScheme optionscheme_default = {
	.name = "default",
	.colors = {
		/* 8 normal colors */
		[0] = "#000000", [1] = "#FF0000", [2] = "#00FF00", [3] = "#FFFF00",
		[4] = "#0000FF", [5] = "#FF00FF", [6] = "#00FFFF", [7] = "#FFFFFF",
		/* 8 bright colors */
		[8] = "#808080", [9] = "#FF0000", [10] = "#00FF00", [11] = "#FFFF00",
		[12] = "#0000FF", [13] = "#FF00FF", [14] = "#00FFFF", [15] = "#FFFFFF",

		[255] = 0,

		/* DefaultXX colors */
		[256] = "#FFFFFF", [257] = "#808080",
		[258] = "#FFFFFF", [259] = "#000000"
	},
	.defaultfg  = 258,
	.defaultbg  = 259,
	.defaultcs  = 258,
	.defaultrcs = 257,
};

static const OptionScheme optionscheme_dracula = {
    .name = "dracula",
    .colors = {
        /* 8 normal */
        [0] = "#282a36", [1] = "#ff5555", [2] = "#50fa7b", [3] = "#f1fa8c",
        [4] = "#bd93f9", [5] = "#ff79c6", [6] = "#8be9fd", [7] = "#bbbbbb",
        /* 8 bright */
        [8] = "#6272a4", [9] = "#ff6e6e", [10] = "#69ff94", [11] = "#ffffa5",
        [12] = "#d6acff", [13] = "#ff92df", [14] = "#a4ffff", [15] = "#ffffff",

        [255] = 0,

        /* DefaultXX */
        [256] = "#f8f8f2", /* cursor */
        [257] = "#44475a", /* reverse cursor */
        [258] = "#f8f8f2", /* fg */
        [259] = "#282a36"  /* bg */
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_solarized = {
    .name = "solarized",
    .colors = {
        /* 8 normal colors */
        [0] = "#002b36", [1] = "#dc322f", [2] = "#859900", [3] = "#b58900",
        [4] = "#268bd2", [5] = "#6c71c4", [6] = "#2aa198", [7] = "#93a1a1",
        /* 8 bright colors */
        [8] = "#657b83", [9] = "#dc322f", [10] = "#859900", [11] = "#b58900",
        [12] = "#268bd2", [13] = "#6c71c4", [14] = "#2aa198", [15] = "#fdf6e3",

        [255] = 0,

        /* DefaultXX */
        [256] = "#839496", [257] = "#002b36", [258] = "#839496", [259] = "#002b36"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_nord = {
    .name = "nord",
    .colors = {
        /* 8 normal */
        [0] = "#3b4252", [1] = "#bf616a", [2] = "#a3be8c", [3] = "#ebcb8b",
        [4] = "#81a1c1", [5] = "#b48ead", [6] = "#88c0d0", [7] = "#e5e9f0",
        /* 8 bright */
        [8] = "#4c566a", [9] = "#bf616a", [10] = "#a3be8c", [11] = "#ebcb8b",
        [12] = "#81a1c1", [13] = "#b48ead", [14] = "#8fbcbb", [15] = "#eceff4",

        [255] = 0,

        /* DefaultXX */
        [256] = "#d8dee9", [257] = "#2e3440", [258] = "#d8dee9", [259] = "#2e3440"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_gruvbox = {
    .name = "gruvbox",
    .colors = {
        /* 8 normal */
        [0] = "#282828", [1] = "#cc241d", [2] = "#98971a", [3] = "#d79921",
        [4] = "#458588", [5] = "#b16286", [6] = "#689d6a", [7] = "#a89984",
        /* 8 bright */
        [8] = "#928374", [9] = "#fb4934", [10] = "#b8bb26", [11] = "#fabd2f",
        [12] = "#83a598", [13] = "#d3869b", [14] = "#8ec07c", [15] = "#ebdbb2",

        [255] = 0,

        /* DefaultXX */
        [256] = "#ebdbb2", [257] = "#282828", [258] = "#ebdbb2", [259] = "#282828"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_plan9 = {
    .name = "plan9",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#9c0000", [2] = "#008000", [3] = "#999900",
        [4] = "#00009c", [5] = "#990099", [6] = "#009999", [7] = "#7f7f7f",
        /* 8 bright colors */
        [8] = "#c0c0c0", [9] = "#ff0000", [10] = "#00ff00", [11] = "#ffff00",
        [12] = "#5c5cff", [13] = "#ff00ff", [14] = "#00ffff", [15] = "#ffffff",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#000000", [257] = "#9ecfff", [258] = "#000000", [259] = "#ffffea"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_cyberpunk = {
    .name = "cyberpunk",
    .colors = {
        /* 8 normal */
        [0] = "#0d0221", [1] = "#ff004d", [2] = "#00ff9f", [3] = "#fffa72",
        [4] = "#00caff", [5] = "#ff00ff", [6] = "#0affef", [7] = "#a0a0a0",
        /* 8 bright */
        [8] = "#3a0f5c", [9] = "#ff4d79", [10] = "#4dffb8", [11] = "#fffaa0",
        [12] = "#4dd2ff", [13] = "#ff4dff", [14] = "#4df9ff", [15] = "#ffffff",

        [255] = 0,

        /* DefaultXX */
        [256] = "#00ffcc", [257] = "#0d0221", [258] = "#00ffcc", [259] = "#0d0221"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_light = {
    .name = "light",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#d70000", [2] = "#008700", [3] = "#af8700",
        [4] = "#005faf", [5] = "#8700af", [6] = "#00afaf", [7] = "#d0d0d0",
        /* 8 bright colors */
        [8] = "#808080", [9] = "#ff0000", [10] = "#00ff00", [11] = "#ffff00",
        [12] = "#5c5cff", [13] = "#ff00ff", [14] = "#00ffff", [15] = "#ffffff",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#000000", [257] = "#4a90e2", [258] = "#000000", [259] = "#eeeeee"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};


static const OptionScheme optionscheme_ork = {
    .name = "ork",
    .colors = {
        /* 8 normal colors */
        [0] = "#0B1311", [1] = "#D64032", [2] = "#80D7A3", [3] = "#C4A84B",
        [4] = "#6f7672", [5] = "#D64032", [6] = "#80D7A3", [7] = "#F5F5DC",
        /* 8 bright colors */
        [8] = "#1C1F1F", [9] = "#D64032", [10] = "#80D7A3", [11] = "#D4AF37",
        [12] = "#8B9383", [13] = "#D64032", [14] = "#80D7A3", [15] = "#FFFFFF",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#C4A84B", [257] = "#D64032",
        [258] = "#80D7A3", [259] = "#0B1311"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};









/*
 * you can move the schemes to a different header and do #include "optionschemes.h"
 * or you can move each scheme to its own file
 * and do #include optionscheme_light.h, #include optionscheme_plan9.h, ...
 * */

static const OptionScheme optionscheme_ledg = {
    .name = "ledg",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#001100", [2] = "#002200", [3] = "#003300", [4] = "#004400", [5] = "#005500", [6] = "#006600", [7] = "#007700",
        /* 8 bright colors */
        [8] = "#008800", [9] = "#009900", [10] = "#00AA00", [11] = "#00BB00", [12] = "#00CC00", [13] = "#00DD00", [14] = "#00EE00", [15] = "#00FF00",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#00FF00", [257] = "#007700",
        [258] = "#00FF00", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_ledb = {
    .name = "ledb",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#000011", [2] = "#000022", [3] = "#000033", [4] = "#000044", [5] = "#000055", [6] = "#000066", [7] = "#000077",
        /* 8 bright colors */
        [8] = "#000088", [9] = "#000099", [10] = "#0000AA", [11] = "#0000BB", [12] = "#0000CC", [13] = "#0000DD", [14] = "#0000EE", [15] = "#0000FF",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#0000FF", [257] = "#000077",
        [258] = "#0000FF", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_ledr = {
    .name = "ledr",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#110000", [2] = "#220000", [3] = "#330000", [4] = "#440000", [5] = "#550000", [6] = "#660000", [7] = "#770000",
        /* 8 bright colors */
        [8] = "#880000", [9] = "#990000", [10] = "#AA0000", [11] = "#BB0000", [12] = "#CC0000", [13] = "#DD0000", [14] = "#EE0000", [15] = "#FF0000",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#FF0000", [257] = "#770000",
        [258] = "#FF0000", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_ledw = {
    .name = "ledw",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#111111", [2] = "#222222", [3] = "#333333", [4] = "#444444", [5] = "#555555", [6] = "#666666", [7] = "#777777",
        /* 8 bright colors */
        [8] = "#888888", [9] = "#999999", [10] = "#AAAAAA", [11] = "#BBBBBB", [12] = "#CCCCCC", [13] = "#DDDDDD", [14] = "#EEEEEE", [15] = "#FFFFFF",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#FFFFFF", [257] = "#777777",
        [258] = "#FFFFFF", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_ledy = {
    .name = "ledy",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#111100", [2] = "#222200", [3] = "#333300", [4] = "#444400", [5] = "#555500", [6] = "#666600", [7] = "#777700",
        /* 8 bright colors */
        [8] = "#888800", [9] = "#999900", [10] = "#AAAA00", [11] = "#BBBB00", [12] = "#CCCC00", [13] = "#DDDD00", [14] = "#EEEE00", [15] = "#FFFF00",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#FFFF00", [257] = "#777700",
        [258] = "#FFFF00", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_leda = {
    .name = "leda",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#110D00", [2] = "#221900", [3] = "#332600", [4] = "#443300", [5] = "#554000", [6] = "#664C00", [7] = "#775900",
        /* 8 bright colors */
        [8] = "#886600", [9] = "#997300", [10] = "#AA7F00", [11] = "#BB8C00", [12] = "#CC9900", [13] = "#DDA600", [14] = "#EEB200", [15] = "#FFBF00",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#FFBF00", [257] = "#775900",
        [258] = "#FFBF00", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_ledc = {
    .name = "ledc",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#001111", [2] = "#002222", [3] = "#003333", [4] = "#004444", [5] = "#005555", [6] = "#006666", [7] = "#007777",
        /* 8 bright colors */
        [8] = "#008888", [9] = "#009999", [10] = "#00AAAA", [11] = "#00BBBB", [12] = "#00CCCC", [13] = "#00DDDD", [14] = "#00EEEE", [15] = "#00FFFF",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#00FFFF", [257] = "#007777",
        [258] = "#00FFFF", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme optionscheme_ledmg = {
    .name = "ledmg",
    .colors = {
        /* 8 normal colors */
        [0] = "#000000", [1] = "#110011", [2] = "#220022", [3] = "#330033", [4] = "#440044", [5] = "#550055", [6] = "#660066", [7] = "#770077",
        /* 8 bright colors */
        [8] = "#880088", [9] = "#990099", [10] = "#AA00AA", [11] = "#BB00BB", [12] = "#CC00CC", [13] = "#DD00DD", [14] = "#EE00EE", [15] = "#FF00FF",

        [255] = 0,

        /* DefaultXX colors */
        [256] = "#FF00FF", [257] = "#770077",
        [258] = "#FF00FF", [259] = "#000000"
    },
    .defaultfg = 258,
    .defaultbg = 259,
    .defaultcs = 256,
    .defaultrcs = 257
};

static const OptionScheme *optionschemes[] = {
    &optionscheme_default,
    &optionscheme_light,
    &optionscheme_plan9,
    &optionscheme_dracula,
    &optionscheme_solarized,
    &optionscheme_nord,
    &optionscheme_gruvbox,
    &optionscheme_cyberpunk,
    &optionscheme_ork,
    &optionscheme_ledg,
    &optionscheme_ledb,
    &optionscheme_ledr,
    &optionscheme_ledw,
    &optionscheme_ledy,
    &optionscheme_leda,
    &optionscheme_ledc,
    &optionscheme_ledmg,
    NULL /* has to be NULL terminated */
};

/*
 * Default shape of cursor
 * 2: Block ("█")
 * 4: Underline ("_")
 * 6: Bar ("|")
 * 7: Snowman ("☃")
 */
static unsigned int cursorshape = 2;

/*
 * Default columns and rows numbers
 */

static unsigned int cols = 80;
static unsigned int rows = 24;

/*
 * Default colour and shape of the mouse cursor
 */
static unsigned int mouseshape = XC_xterm;
static unsigned int mousefg = 7;
static unsigned int mousebg = 0;

/*
 * Color used to display font attributes when fontconfig selected a font which
 * doesn't match the ones requested.
 */
static unsigned int defaultattr = 11;

/*
 * Graphics configuration
 */

/// The template for the cache directory.
const char graphics_cache_dir_template[] = "/tmp/st-images-XXXXXX";
/// The max size of a single image file, in bytes.
unsigned graphics_max_single_image_file_size = 20 * 1024 * 1024;
/// The max size of the cache, in bytes.
unsigned graphics_total_file_cache_size = 300 * 1024 * 1024;
/// The max ram size of an image or placement, in bytes.
unsigned graphics_max_single_image_ram_size = 100 * 1024 * 1024;
/// The max total size of all images loaded into RAM.
unsigned graphics_max_total_ram_size = 300 * 1024 * 1024;
/// The max total number of image placements and images.
unsigned graphics_max_total_placements = 4096;
/// The ratio by which limits can be exceeded.
double graphics_excess_tolerance_ratio = 0.05;
/// The minimum delay between redraws caused by animations, in milliseconds.
unsigned graphics_animation_min_delay = 20;

/*
 * Force mouse select/shortcuts while mask is active (when MODE_MOUSE is set).
 * Note that if you want to use ShiftMask with selmasks, set this to an other
 * modifier, set to 0 to not use it.
 */
static uint forcemousemod = ShiftMask;

/* Internal keyboard shortcuts. */
#define MODKEY Mod1Mask
#define TERMMOD (ControlMask|ShiftMask)

/*
 * Internal mouse shortcuts.
 * Beware that overloading Button1 will disable the selection.
 */
static MouseShortcut mshortcuts[] = {
	/* mask                 button   function        argument       release */
	{ TERMMOD,              Button3, previewimage,   {.s = "feh"} },
	{ TERMMOD,              Button2, showimageinfo,  {},            1 },
	{ ShiftMask,            Button4, kscrollup,      {.i = 1} },
	{ ShiftMask,            Button5, kscrolldown,    {.i = 1} },
	{ XK_ANY_MOD,           Button2, selpaste,       {.i = 0},      1 },
	{ ShiftMask,            Button4, ttysend,        {.s = "\033[5;2~"} },
	{ XK_ANY_MOD,           Button4, ttysend,        {.s = "\031"} },
	{ ShiftMask,            Button5, ttysend,        {.s = "\033[6;2~"} },
	{ XK_ANY_MOD,           Button5, ttysend,        {.s = "\005"} },
};

/* Internal keyboard shortcuts. */
#define TERM2MOD (ControlMask)

static Shortcut shortcuts[] = {
	/* mask                 keysym          function        argument */
	{ XK_ANY_MOD,           XK_Break,       sendbreak,      {.i =  0} },
	{ ControlMask,          XK_Print,       toggleprinter,  {.i =  0} },
	{ ShiftMask,            XK_Print,       printscreen,    {.i =  0} },
	{ XK_ANY_MOD,           XK_Print,       printsel,       {.i =  0} },
	{ TERM2MOD,              XK_equal,       zoom,           {.f = +1} },
	{ TERMMOD,              XK_Prior,       zoom,           {.f = +1} },
	{ TERM2MOD,              XK_minus,        zoom,           {.f = -1} },
	{ TERMMOD,              XK_Next,        zoom,           {.f = -1} },
	{ TERMMOD,              XK_Home,        zoomreset,      {.f =  0} },
	{ TERMMOD,              XK_C,           clipcopy,       {.i =  0} },
	{ TERMMOD,              XK_V,           clippaste,      {.i =  0} },
	{ TERMMOD,              XK_Y,           selpaste,       {.i =  0} },
	{ ShiftMask,            XK_Insert,      selpaste,       {.i =  0} },
	{ TERMMOD,              XK_Num_Lock,    numlock,        {.i =  0} },
	{ TERMMOD,              XK_F1,          togglegrdebug,  {.i =  0} },
	{ TERMMOD,              XK_F6,          dumpgrstate,    {.i =  0} },
	{ TERMMOD,              XK_F7,          unloadimages,   {.i =  0} },
	{ TERMMOD,              XK_F8,          toggleimages,   {.i =  0} },
	{ TERM2MOD,              XK_grave,           cyclefonts,     {}        },
	{ ShiftMask,            XK_Page_Up,     kscrollup,      {.i = -1} },
    { ShiftMask,            XK_Page_Down,   kscrolldown,    {.i = -1} },
};

/*
 * Special keys (change & recompile st.info accordingly)
 *
 * Mask value:
 * * Use XK_ANY_MOD to match the key no matter modifiers state
 * * Use XK_NO_MOD to match the key alone (no modifiers)
 * appkey value:
 * * 0: no value
 * * > 0: keypad application mode enabled
 * *   = 2: term.numlock = 1
 * * < 0: keypad application mode disabled
 * appcursor value:
 * * 0: no value
 * * > 0: cursor application mode enabled
 * * < 0: cursor application mode disabled
 *
 * Be careful with the order of the definitions because st searches in
 * this table sequentially, so any XK_ANY_MOD must be in the last
 * position for a key.
 */

/*
 * If you want keys other than the X11 function keys (0xFD00 - 0xFFFF)
 * to be mapped below, add them to this array.
 */
static KeySym mappedkeys[] = { -1 };

/*
 * State bits to ignore when matching key or button events.  By default,
 * numlock (Mod2Mask) and keyboard layout (XK_SWITCH_MOD) are ignored.
 */
static uint ignoremod = Mod2Mask|XK_SWITCH_MOD;

/*
 * This is the huge key array which defines all compatibility to the Linux
 * world. Please decide about changes wisely.
 */
static Key key[] = {
	/* keysym           mask            string      appkey appcursor */
	{ XK_KP_Home,       ShiftMask,      "\033[2J",       0,   -1},
	{ XK_KP_Home,       ShiftMask,      "\033[1;2H",     0,   +1},
	{ XK_KP_Home,       XK_ANY_MOD,     "\033[H",        0,   -1},
	{ XK_KP_Home,       XK_ANY_MOD,     "\033[1~",       0,   +1},
	{ XK_KP_Up,         XK_ANY_MOD,     "\033Ox",       +1,    0},
	{ XK_KP_Up,         XK_ANY_MOD,     "\033[A",        0,   -1},
	{ XK_KP_Up,         XK_ANY_MOD,     "\033OA",        0,   +1},
	{ XK_KP_Down,       XK_ANY_MOD,     "\033Or",       +1,    0},
	{ XK_KP_Down,       XK_ANY_MOD,     "\033[B",        0,   -1},
	{ XK_KP_Down,       XK_ANY_MOD,     "\033OB",        0,   +1},
	{ XK_KP_Left,       XK_ANY_MOD,     "\033Ot",       +1,    0},
	{ XK_KP_Left,       XK_ANY_MOD,     "\033[D",        0,   -1},
	{ XK_KP_Left,       XK_ANY_MOD,     "\033OD",        0,   +1},
	{ XK_KP_Right,      XK_ANY_MOD,     "\033Ov",       +1,    0},
	{ XK_KP_Right,      XK_ANY_MOD,     "\033[C",        0,   -1},
	{ XK_KP_Right,      XK_ANY_MOD,     "\033OC",        0,   +1},
	{ XK_KP_Prior,      ShiftMask,      "\033[5;2~",     0,    0},
	{ XK_KP_Prior,      XK_ANY_MOD,     "\033[5~",       0,    0},
	{ XK_KP_Begin,      XK_ANY_MOD,     "\033[E",        0,    0},
	{ XK_KP_End,        ControlMask,    "\033[J",       -1,    0},
	{ XK_KP_End,        ControlMask,    "\033[1;5F",    +1,    0},
	{ XK_KP_End,        ShiftMask,      "\033[K",       -1,    0},
	{ XK_KP_End,        ShiftMask,      "\033[1;2F",    +1,    0},
	{ XK_KP_End,        XK_ANY_MOD,     "\033[4~",       0,    0},
	{ XK_KP_Next,       ShiftMask,      "\033[6;2~",     0,    0},
	{ XK_KP_Next,       XK_ANY_MOD,     "\033[6~",       0,    0},
	{ XK_KP_Insert,     ShiftMask,      "\033[2;2~",    +1,    0},
	{ XK_KP_Insert,     ShiftMask,      "\033[4l",      -1,    0},
	{ XK_KP_Insert,     ControlMask,    "\033[L",       -1,    0},
	{ XK_KP_Insert,     ControlMask,    "\033[2;5~",    +1,    0},
	{ XK_KP_Insert,     XK_ANY_MOD,     "\033[4h",      -1,    0},
	{ XK_KP_Insert,     XK_ANY_MOD,     "\033[2~",      +1,    0},
	{ XK_KP_Delete,     ControlMask,    "\033[M",       -1,    0},
	{ XK_KP_Delete,     ControlMask,    "\033[3;5~",    +1,    0},
	{ XK_KP_Delete,     ShiftMask,      "\033[2K",      -1,    0},
	{ XK_KP_Delete,     ShiftMask,      "\033[3;2~",    +1,    0},
	{ XK_KP_Delete,     XK_ANY_MOD,     "\033[P",       -1,    0},
	{ XK_KP_Delete,     XK_ANY_MOD,     "\033[3~",      +1,    0},
	{ XK_KP_Multiply,   XK_ANY_MOD,     "\033Oj",       +2,    0},
	{ XK_KP_Add,        XK_ANY_MOD,     "\033Ok",       +2,    0},
	{ XK_KP_Enter,      XK_ANY_MOD,     "\033OM",       +2,    0},
	{ XK_KP_Enter,      XK_ANY_MOD,     "\r",           -1,    0},
	{ XK_KP_Subtract,   XK_ANY_MOD,     "\033Om",       +2,    0},
	{ XK_KP_Decimal,    XK_ANY_MOD,     "\033On",       +2,    0},
	{ XK_KP_Divide,     XK_ANY_MOD,     "\033Oo",       +2,    0},
	{ XK_KP_0,          XK_ANY_MOD,     "\033Op",       +2,    0},
	{ XK_KP_1,          XK_ANY_MOD,     "\033Oq",       +2,    0},
	{ XK_KP_2,          XK_ANY_MOD,     "\033Or",       +2,    0},
	{ XK_KP_3,          XK_ANY_MOD,     "\033Os",       +2,    0},
	{ XK_KP_4,          XK_ANY_MOD,     "\033Ot",       +2,    0},
	{ XK_KP_5,          XK_ANY_MOD,     "\033Ou",       +2,    0},
	{ XK_KP_6,          XK_ANY_MOD,     "\033Ov",       +2,    0},
	{ XK_KP_7,          XK_ANY_MOD,     "\033Ow",       +2,    0},
	{ XK_KP_8,          XK_ANY_MOD,     "\033Ox",       +2,    0},
	{ XK_KP_9,          XK_ANY_MOD,     "\033Oy",       +2,    0},
	{ XK_Up,            ShiftMask,      "\033[1;2A",     0,    0},
	{ XK_Up,            Mod1Mask,       "\033[1;3A",     0,    0},
	{ XK_Up,         ShiftMask|Mod1Mask,"\033[1;4A",     0,    0},
	{ XK_Up,            ControlMask,    "\033[1;5A",     0,    0},
	{ XK_Up,      ShiftMask|ControlMask,"\033[1;6A",     0,    0},
	{ XK_Up,       ControlMask|Mod1Mask,"\033[1;7A",     0,    0},
	{ XK_Up,ShiftMask|ControlMask|Mod1Mask,"\033[1;8A",  0,    0},
	{ XK_Up,            XK_ANY_MOD,     "\033[A",        0,   -1},
	{ XK_Up,            XK_ANY_MOD,     "\033OA",        0,   +1},
	{ XK_Down,          ShiftMask,      "\033[1;2B",     0,    0},
	{ XK_Down,          Mod1Mask,       "\033[1;3B",     0,    0},
	{ XK_Down,       ShiftMask|Mod1Mask,"\033[1;4B",     0,    0},
	{ XK_Down,          ControlMask,    "\033[1;5B",     0,    0},
	{ XK_Down,    ShiftMask|ControlMask,"\033[1;6B",     0,    0},
	{ XK_Down,     ControlMask|Mod1Mask,"\033[1;7B",     0,    0},
	{ XK_Down,ShiftMask|ControlMask|Mod1Mask,"\033[1;8B",0,    0},
	{ XK_Down,          XK_ANY_MOD,     "\033[B",        0,   -1},
	{ XK_Down,          XK_ANY_MOD,     "\033OB",        0,   +1},
	{ XK_Left,          ShiftMask,      "\033[1;2D",     0,    0},
	{ XK_Left,          Mod1Mask,       "\033[1;3D",     0,    0},
	{ XK_Left,       ShiftMask|Mod1Mask,"\033[1;4D",     0,    0},
	{ XK_Left,          ControlMask,    "\033[1;5D",     0,    0},
	{ XK_Left,    ShiftMask|ControlMask,"\033[1;6D",     0,    0},
	{ XK_Left,     ControlMask|Mod1Mask,"\033[1;7D",     0,    0},
	{ XK_Left,ShiftMask|ControlMask|Mod1Mask,"\033[1;8D",0,    0},
	{ XK_Left,          XK_ANY_MOD,     "\033[D",        0,   -1},
	{ XK_Left,          XK_ANY_MOD,     "\033OD",        0,   +1},
	{ XK_Right,         ShiftMask,      "\033[1;2C",     0,    0},
	{ XK_Right,         Mod1Mask,       "\033[1;3C",     0,    0},
	{ XK_Right,      ShiftMask|Mod1Mask,"\033[1;4C",     0,    0},
	{ XK_Right,         ControlMask,    "\033[1;5C",     0,    0},
	{ XK_Right,   ShiftMask|ControlMask,"\033[1;6C",     0,    0},
	{ XK_Right,    ControlMask|Mod1Mask,"\033[1;7C",     0,    0},
	{ XK_Right,ShiftMask|ControlMask|Mod1Mask,"\033[1;8C",0,   0},
	{ XK_Right,         XK_ANY_MOD,     "\033[C",        0,   -1},
	{ XK_Right,         XK_ANY_MOD,     "\033OC",        0,   +1},
	{ XK_ISO_Left_Tab,  ShiftMask,      "\033[Z",        0,    0},
	{ XK_Return,        Mod1Mask,       "\033\r",        0,    0},
	{ XK_Return,        XK_ANY_MOD,     "\r",            0,    0},
	{ XK_Insert,        ShiftMask,      "\033[4l",      -1,    0},
	{ XK_Insert,        ShiftMask,      "\033[2;2~",    +1,    0},
	{ XK_Insert,        ControlMask,    "\033[L",       -1,    0},
	{ XK_Insert,        ControlMask,    "\033[2;5~",    +1,    0},
	{ XK_Insert,        XK_ANY_MOD,     "\033[4h",      -1,    0},
	{ XK_Insert,        XK_ANY_MOD,     "\033[2~",      +1,    0},
	{ XK_Delete,        ControlMask,    "\033[M",       -1,    0},
	{ XK_Delete,        ControlMask,    "\033[3;5~",    +1,    0},
	{ XK_Delete,        ShiftMask,      "\033[2K",      -1,    0},
	{ XK_Delete,        ShiftMask,      "\033[3;2~",    +1,    0},
	{ XK_Delete,        XK_ANY_MOD,     "\033[P",       -1,    0},
	{ XK_Delete,        XK_ANY_MOD,     "\033[3~",      +1,    0},
	{ XK_BackSpace,     XK_NO_MOD,      "\177",          0,    0},
	{ XK_BackSpace,     Mod1Mask,       "\033\177",      0,    0},
	{ XK_Home,          ShiftMask,      "\033[2J",       0,   -1},
	{ XK_Home,          ShiftMask,      "\033[1;2H",     0,   +1},
	{ XK_Home,          XK_ANY_MOD,     "\033[H",        0,   -1},
	{ XK_Home,          XK_ANY_MOD,     "\033[1~",       0,   +1},
	{ XK_End,           ControlMask,    "\033[J",       -1,    0},
	{ XK_End,           ControlMask,    "\033[1;5F",    +1,    0},
	{ XK_End,           ShiftMask,      "\033[K",       -1,    0},
	{ XK_End,           ShiftMask,      "\033[1;2F",    +1,    0},
	{ XK_End,           XK_ANY_MOD,     "\033[4~",       0,    0},
	{ XK_Prior,         ControlMask,    "\033[5;5~",     0,    0},
	{ XK_Prior,         ShiftMask,      "\033[5;2~",     0,    0},
	{ XK_Prior,         XK_ANY_MOD,     "\033[5~",       0,    0},
	{ XK_Next,          ControlMask,    "\033[6;5~",     0,    0},
	{ XK_Next,          ShiftMask,      "\033[6;2~",     0,    0},
	{ XK_Next,          XK_ANY_MOD,     "\033[6~",       0,    0},
	{ XK_F1,            XK_NO_MOD,      "\033OP" ,       0,    0},
	{ XK_F1, /* F13 */  ShiftMask,      "\033[1;2P",     0,    0},
	{ XK_F1, /* F25 */  ControlMask,    "\033[1;5P",     0,    0},
	{ XK_F1, /* F37 */  Mod4Mask,       "\033[1;6P",     0,    0},
	{ XK_F1, /* F49 */  Mod1Mask,       "\033[1;3P",     0,    0},
	{ XK_F1, /* F61 */  Mod3Mask,       "\033[1;4P",     0,    0},
	{ XK_F2,            XK_NO_MOD,      "\033OQ" ,       0,    0},
	{ XK_F2, /* F14 */  ShiftMask,      "\033[1;2Q",     0,    0},
	{ XK_F2, /* F26 */  ControlMask,    "\033[1;5Q",     0,    0},
	{ XK_F2, /* F38 */  Mod4Mask,       "\033[1;6Q",     0,    0},
	{ XK_F2, /* F50 */  Mod1Mask,       "\033[1;3Q",     0,    0},
	{ XK_F2, /* F62 */  Mod3Mask,       "\033[1;4Q",     0,    0},
	{ XK_F3,            XK_NO_MOD,      "\033OR" ,       0,    0},
	{ XK_F3, /* F15 */  ShiftMask,      "\033[1;2R",     0,    0},
	{ XK_F3, /* F27 */  ControlMask,    "\033[1;5R",     0,    0},
	{ XK_F3, /* F39 */  Mod4Mask,       "\033[1;6R",     0,    0},
	{ XK_F3, /* F51 */  Mod1Mask,       "\033[1;3R",     0,    0},
	{ XK_F3, /* F63 */  Mod3Mask,       "\033[1;4R",     0,    0},
	{ XK_F4,            XK_NO_MOD,      "\033OS" ,       0,    0},
	{ XK_F4, /* F16 */  ShiftMask,      "\033[1;2S",     0,    0},
	{ XK_F4, /* F28 */  ControlMask,    "\033[1;5S",     0,    0},
	{ XK_F4, /* F40 */  Mod4Mask,       "\033[1;6S",     0,    0},
	{ XK_F4, /* F52 */  Mod1Mask,       "\033[1;3S",     0,    0},
	{ XK_F5,            XK_NO_MOD,      "\033[15~",      0,    0},
	{ XK_F5, /* F17 */  ShiftMask,      "\033[15;2~",    0,    0},
	{ XK_F5, /* F29 */  ControlMask,    "\033[15;5~",    0,    0},
	{ XK_F5, /* F41 */  Mod4Mask,       "\033[15;6~",    0,    0},
	{ XK_F5, /* F53 */  Mod1Mask,       "\033[15;3~",    0,    0},
	{ XK_F6,            XK_NO_MOD,      "\033[17~",      0,    0},
	{ XK_F6, /* F18 */  ShiftMask,      "\033[17;2~",    0,    0},
	{ XK_F6, /* F30 */  ControlMask,    "\033[17;5~",    0,    0},
	{ XK_F6, /* F42 */  Mod4Mask,       "\033[17;6~",    0,    0},
	{ XK_F6, /* F54 */  Mod1Mask,       "\033[17;3~",    0,    0},
	{ XK_F7,            XK_NO_MOD,      "\033[18~",      0,    0},
	{ XK_F7, /* F19 */  ShiftMask,      "\033[18;2~",    0,    0},
	{ XK_F7, /* F31 */  ControlMask,    "\033[18;5~",    0,    0},
	{ XK_F7, /* F43 */  Mod4Mask,       "\033[18;6~",    0,    0},
	{ XK_F7, /* F55 */  Mod1Mask,       "\033[18;3~",    0,    0},
	{ XK_F8,            XK_NO_MOD,      "\033[19~",      0,    0},
	{ XK_F8, /* F20 */  ShiftMask,      "\033[19;2~",    0,    0},
	{ XK_F8, /* F32 */  ControlMask,    "\033[19;5~",    0,    0},
	{ XK_F8, /* F44 */  Mod4Mask,       "\033[19;6~",    0,    0},
	{ XK_F8, /* F56 */  Mod1Mask,       "\033[19;3~",    0,    0},
	{ XK_F9,            XK_NO_MOD,      "\033[20~",      0,    0},
	{ XK_F9, /* F21 */  ShiftMask,      "\033[20;2~",    0,    0},
	{ XK_F9, /* F33 */  ControlMask,    "\033[20;5~",    0,    0},
	{ XK_F9, /* F45 */  Mod4Mask,       "\033[20;6~",    0,    0},
	{ XK_F9, /* F57 */  Mod1Mask,       "\033[20;3~",    0,    0},
	{ XK_F10,           XK_NO_MOD,      "\033[21~",      0,    0},
	{ XK_F10, /* F22 */ ShiftMask,      "\033[21;2~",    0,    0},
	{ XK_F10, /* F34 */ ControlMask,    "\033[21;5~",    0,    0},
	{ XK_F10, /* F46 */ Mod4Mask,       "\033[21;6~",    0,    0},
	{ XK_F10, /* F58 */ Mod1Mask,       "\033[21;3~",    0,    0},
	{ XK_F11,           XK_NO_MOD,      "\033[23~",      0,    0},
	{ XK_F11, /* F23 */ ShiftMask,      "\033[23;2~",    0,    0},
	{ XK_F11, /* F35 */ ControlMask,    "\033[23;5~",    0,    0},
	{ XK_F11, /* F47 */ Mod4Mask,       "\033[23;6~",    0,    0},
	{ XK_F11, /* F59 */ Mod1Mask,       "\033[23;3~",    0,    0},
	{ XK_F12,           XK_NO_MOD,      "\033[24~",      0,    0},
	{ XK_F12, /* F24 */ ShiftMask,      "\033[24;2~",    0,    0},
	{ XK_F12, /* F36 */ ControlMask,    "\033[24;5~",    0,    0},
	{ XK_F12, /* F48 */ Mod4Mask,       "\033[24;6~",    0,    0},
	{ XK_F12, /* F60 */ Mod1Mask,       "\033[24;3~",    0,    0},
	{ XK_F13,           XK_NO_MOD,      "\033[1;2P",     0,    0},
	{ XK_F14,           XK_NO_MOD,      "\033[1;2Q",     0,    0},
	{ XK_F15,           XK_NO_MOD,      "\033[1;2R",     0,    0},
	{ XK_F16,           XK_NO_MOD,      "\033[1;2S",     0,    0},
	{ XK_F17,           XK_NO_MOD,      "\033[15;2~",    0,    0},
	{ XK_F18,           XK_NO_MOD,      "\033[17;2~",    0,    0},
	{ XK_F19,           XK_NO_MOD,      "\033[18;2~",    0,    0},
	{ XK_F20,           XK_NO_MOD,      "\033[19;2~",    0,    0},
	{ XK_F21,           XK_NO_MOD,      "\033[20;2~",    0,    0},
	{ XK_F22,           XK_NO_MOD,      "\033[21;2~",    0,    0},
	{ XK_F23,           XK_NO_MOD,      "\033[23;2~",    0,    0},
	{ XK_F24,           XK_NO_MOD,      "\033[24;2~",    0,    0},
	{ XK_F25,           XK_NO_MOD,      "\033[1;5P",     0,    0},
	{ XK_F26,           XK_NO_MOD,      "\033[1;5Q",     0,    0},
	{ XK_F27,           XK_NO_MOD,      "\033[1;5R",     0,    0},
	{ XK_F28,           XK_NO_MOD,      "\033[1;5S",     0,    0},
	{ XK_F29,           XK_NO_MOD,      "\033[15;5~",    0,    0},
	{ XK_F30,           XK_NO_MOD,      "\033[17;5~",    0,    0},
	{ XK_F31,           XK_NO_MOD,      "\033[18;5~",    0,    0},
	{ XK_F32,           XK_NO_MOD,      "\033[19;5~",    0,    0},
	{ XK_F33,           XK_NO_MOD,      "\033[20;5~",    0,    0},
	{ XK_F34,           XK_NO_MOD,      "\033[21;5~",    0,    0},
	{ XK_F35,           XK_NO_MOD,      "\033[23;5~",    0,    0},

/* xterm compatible Shift-F* mod */
/* modifier      key        function        argument */
    /* key          mask            string          appkey appcursor */
//    { XK_F1, /* F11 */  ShiftMask,      "\033[1;2N",     0,    0}, /* Shift+F1  (F11) */
//    { XK_F2, /* F12 */  ShiftMask,      "\033[1;2O",     0,    0}, /* Shift+F2  (F12) */
//    { XK_F3, /* F13 */  ShiftMask,      "\033[1;2P",     0,    0},
//    { XK_F4, /* F14 */  ShiftMask,      "\033[1;2Q",     0,    0},
//    { XK_F5, /* F15 */  ShiftMask,      "\033[1;2R",     0,    0},
//    { XK_F6, /* F16 */  ShiftMask,      "\033[1;2S",     0,    0},
//    { XK_F7, /* F17 */  ShiftMask,      "\033[15;2~",    0,    0},
//    { XK_F8, /* F18 */  ShiftMask,      "\033[17;2~",    0,    0},
//    { XK_F9, /* F19 */  ShiftMask,      "\033[18;2~",    0,    0},
//    { XK_F10,/* F20 */  ShiftMask,      "\033[19;2~",    0,    0},
//    { XK_F11,/* F21 */  ShiftMask,      "\033[20;2~",    0,    0},
//    { XK_F12,/* F22 */  ShiftMask,      "\033[21;2~",    0,    0},

};

/*
 * Selection types' masks.
 * Use the same masks as usual.
 * Button1Mask is always unset, to make masks match between ButtonPress.
 * ButtonRelease and MotionNotify.
 * If no match is found, regular selection is used.
 */
static uint selmasks[] = {
	[SEL_RECTANGULAR] = Mod1Mask,
};

/*
 * Printable characters in ASCII, used to estimate the advance width
 * of single wide characters.
 */
static char ascii_printable[] =
	" !\"#$%&'()*+,-./0123456789:;<=>?"
	"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	"`abcdefghijklmnopqrstuvwxyz{|}~";
