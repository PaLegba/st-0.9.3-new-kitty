#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "../../stb/stb_image.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PET_PAD 64
#define PATH_BUF (PATH_MAX + 128)
#define MAX_FRAMES 1024
#define TICK_MS 33
#define FRAME_SECONDS 0.2
#define PET_SPEED 90.0
#define HAPPY_SECONDS 3.0
#define SLEEP_SECONDS 3.0
#define PET_MARGIN 4.0
#define MOUSE_TOLERANCE 4

enum state {
	SLEEPING,
	IDLE,
	N,
	S,
	E,
	W,
	NW,
	NE,
	SW,
	SE,
	DRAGGED,
	HAPPY,
	STATE_LAST
};

static const char *state_names[STATE_LAST] = {
	"sleeping", "idle", "walk_north", "walk_south", "walk_east",
	"walk_west", "walk_northwest", "walk_northeast", "walk_southwest",
	"walk_southeast", "dragged", "happy"
};

struct animation {
	int first;
	int count;
};

struct frame {
	char path[PATH_BUF];
};

struct options {
	const char *pet;
	const char *assets;
	bool screensaver;
};

struct term {
	int cols, rows;
	int xpix, ypix;
	int cw, ch;
	bool raw;
	bool active;
	bool mouse;
};

struct pet {
	double x, y;
	double target_x, target_y;
	double state_time;
	double anim_time;
	double wait_time;
	int anim_frame;
	enum state state;
	bool chasing;
	bool frozen;
	bool dragging;
	double drag_off_x, drag_off_y;
	double mouse_x, mouse_y;
	bool mouse_valid;
};

struct input_parser {
	int state;
	char csi[64];
	size_t csi_len;
};

static struct options opt = { .pet = "neko" };
static struct term term;
static struct pet pet;
static struct animation animations[STATE_LAST];
static struct frame frames[MAX_FRAMES];
static int nframes;
static int sprite_w, sprite_h;
static int canvas_w, canvas_h;
static const char *self_name;
static uint32_t image_id;
static uint32_t placement_id = 1;
static int last_frame = -1;
static int last_col = -1, last_row = -1, last_ox = -1, last_oy = -1;
static int last_cw = -1, last_ch = -1;
static volatile sig_atomic_t quit_requested;
static volatile sig_atomic_t resize_requested;
static struct input_parser parser;

static void term_leave(void);

static void die(const char *fmt, ...)
{
	va_list ap;
	term_leave();
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(1);
}

static double now_seconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

static void write_all(const void *data, size_t len)
{
	const unsigned char *p = data;
	while (len) {
		ssize_t n = write(STDOUT_FILENO, p, len);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			die("write failed: %s", strerror(errno));
		}
		p += n;
		len -= (size_t)n;
	}
}

static void write_str(const char *s)
{
	write_all(s, strlen(s));
}

static void write_quiet(const char *s, size_t len)
{
	while (len) {
		ssize_t n = write(STDOUT_FILENO, s, len);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			return;
		}
		s += n;
		len -= (size_t)n;
	}
}

static void on_signal(int sig)
{
	if (sig == SIGWINCH)
		resize_requested = 1;
	else
		quit_requested = 1;
}

static void install_signals(void)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGWINCH, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
}

static struct termios saved_termios;

static void term_enter(void)
{
	struct termios raw;

	if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
		die("kitty_pet must be run on a terminal");
	if (tcgetattr(STDIN_FILENO, &saved_termios) < 0)
		die("tcgetattr: %s", strerror(errno));
	raw = saved_termios;
	cfmakeraw(&raw);
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &raw) < 0)
		die("tcsetattr: %s", strerror(errno));
	term.raw = true;

	write_str("\033[?1049h\033[?25l\033[2J\033[H");
	term.active = true;

	if (opt.screensaver)
		write_str("\033[?1000h\033[?1003h\033[?1006h");
	else
		write_str("\033[?1000h\033[?1002h\033[?1006h");
	term.mouse = true;
}

static void term_leave(void)
{
	if (term.active) {
		char cmd[128];
		int n;
		term.active = false;
		n = snprintf(cmd, sizeof(cmd),
			"\033_Ga=d,d=I,i=%u,q=2\033\\", image_id);
		if (n > 0)
			write_quiet(cmd, (size_t)n);
		if (term.mouse)
			write_quiet("\033[?1006l\033[?1003l\033[?1002l\033[?1000l",
				    sizeof("\033[?1006l\033[?1003l\033[?1002l\033[?1000l") - 1);
		write_quiet("\033[0m\033[?25h\033[?1049l",
			    sizeof("\033[0m\033[?25h\033[?1049l") - 1);
		term.mouse = false;
	}
	if (term.raw) {
		tcsetattr(STDIN_FILENO, TCSADRAIN, &saved_termios);
		term.raw = false;
	}
}

static void query_cell_size(void)
{
	char buf[128];
	size_t used = 0;
	double end;
	struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };

	write_str("\033[16t");
	end = now_seconds() + 0.3;
	while (now_seconds() < end && used + 1 < sizeof(buf)) {
		int r = poll(&pfd, 1, 30);
		if (r < 0 && errno != EINTR)
			break;
		if (r <= 0)
			continue;
		ssize_t n = read(STDIN_FILENO, buf + used, sizeof(buf) - used - 1);
		if (n <= 0)
			break;
		used += (size_t)n;
		buf[used] = '\0';
		if (strchr(buf, 't'))
			break;
	}
	buf[used] = '\0';
	if (used && sscanf(buf, "\033[6;%d;%d", &term.ch, &term.cw) == 2)
		return;
	term.cw = 8;
	term.ch = 16;
}

static void get_size(void)
{
	struct winsize ws;

	memset(&ws, 0, sizeof(ws));
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0)
		die("TIOCGWINSZ: %s", strerror(errno));
	term.cols = ws.ws_col;
	term.rows = ws.ws_row;
	term.xpix = ws.ws_xpixel;
	term.ypix = ws.ws_ypixel;
	if (term.cols <= 0 || term.rows <= 0)
		die("terminal reports an invalid size");
	if (term.xpix <= 0 || term.ypix <= 0) {
		query_cell_size();
		term.xpix = term.cols * term.cw;
		term.ypix = term.rows * term.ch;
	} else {
		term.cw = term.xpix / term.cols;
		term.ch = term.ypix / term.rows;
	}
	if (term.cw <= 0 || term.ch <= 0)
		die("terminal reports an invalid cell size");
	if (term.cw != last_cw || term.ch != last_ch) {
		last_cw = term.cw;
		last_ch = term.ch;
		last_col = last_row = -1;
	}
}

static void make_assets_path(char *out, size_t outsz)
{
	const char *assets = opt.assets;
	char exe[PATH_MAX];
	char *slash;

	if (assets) {
		snprintf(out, outsz, "%s", assets);
		return;
	}
	if (realpath("/proc/self/exe", exe)) {
		slash = strrchr(exe, '/');
		if (slash) {
			*slash = '\0';
			snprintf(out, outsz, "%s/pets-png", exe);
			return;
		}
	}
	if (realpath("/proc/curproc/file", exe)) {
		slash = strrchr(exe, '/');
		if (slash) {
			*slash = '\0';
			snprintf(out, outsz, "%s/pets-png", exe);
			return;
		}
	}
	if (self_name && strchr(self_name, '/') && realpath(self_name, exe)) {
		slash = strrchr(exe, '/');
		if (slash) {
			*slash = '\0';
			snprintf(out, outsz, "%s/pets-png", exe);
			return;
		}
	}
	snprintf(out, outsz, "pets-png");
}

static void load_assets(void)
{
	char assets[PATH_BUF];
	char path[PATH_BUF + 256];
	int i, s;
	int w, h, n;

	make_assets_path(assets, sizeof(assets));
	for (s = 0; s < STATE_LAST; ++s) {
		animations[s].first = nframes;
		animations[s].count = 0;
		for (i = 0;; ++i) {
			snprintf(path, sizeof(path), "%s/%s/%s/%d.png",
				 assets, opt.pet, state_names[s], i);
			if (access(path, R_OK) < 0)
				break;
			if (nframes >= MAX_FRAMES)
				die("too many animation frames");
			if (strlen(path) >= sizeof(frames[nframes].path))
				die("asset path is too long: %s", path);
			memcpy(frames[nframes].path, path, strlen(path) + 1);
			++nframes;
			++animations[s].count;
		}
	}

	if (animations[IDLE].count == 0)
		die("no idle frames for pet '%s' in %s", opt.pet, assets);

	for (s = 0; s < STATE_LAST; ++s) {
		if (animations[s].count == 0)
			animations[s] = animations[IDLE];
	}

	if (!stbi_info(frames[0].path, &w, &h, &n))
		die("could not read %s: %s", frames[0].path,
		    stbi_failure_reason() ? stbi_failure_reason() : "unknown error");
	canvas_w = w;
	canvas_h = h;
	sprite_w = w - PET_PAD;
	sprite_h = h - PET_PAD;
	if (sprite_w <= 0 || sprite_h <= 0)
		die("invalid padded sprite size %dx%d", w, h);

	for (i = 1; i < nframes; ++i) {
		if (!stbi_info(frames[i].path, &w, &h, &n))
			die("could not read %s: %s", frames[i].path,
			    stbi_failure_reason() ? stbi_failure_reason()
						 : "unknown error");
		if (w != canvas_w || h != canvas_h)
			die("inconsistent frame size in %s", frames[i].path);
	}
}

static int b64_encode(const unsigned char *src, size_t len, char *dst)
{
	static const char tab[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t i, o = 0;
	for (i = 0; i + 2 < len; i += 3) {
		unsigned v = (src[i] << 16) | (src[i + 1] << 8) | src[i + 2];
		dst[o++] = tab[(v >> 18) & 63];
		dst[o++] = tab[(v >> 12) & 63];
		dst[o++] = tab[(v >> 6) & 63];
		dst[o++] = tab[v & 63];
	}
	if (i < len) {
		unsigned v = src[i] << 16;
		if (i + 1 < len)
			v |= src[i + 1] << 8;
		dst[o++] = tab[(v >> 18) & 63];
		dst[o++] = tab[(v >> 12) & 63];
		dst[o++] = i + 1 < len ? tab[(v >> 6) & 63] : '=';
		dst[o++] = '=';
	}
	dst[o] = '\0';
	return (int)o;
}

static void upload_frame(const char *path, int first)
{
	char *absolute;
	char encoded[8192];
	char cmd[9000];
	int n;

	absolute = realpath(path, NULL);
	if (!absolute)
		die("realpath(%s): %s", path, strerror(errno));
	if (strlen(absolute) * 4 / 3 + 8 >= sizeof(encoded))
		die("asset path is too long: %s", absolute);
	b64_encode((const unsigned char *)absolute, strlen(absolute), encoded);
	free(absolute);
	n = snprintf(cmd, sizeof(cmd),
		"\033_Ga=%s,i=%u,f=100,t=f,q=2;%s\033\\",
		first ? "t" : "f", image_id, encoded);
	if (n < 0 || (size_t)n >= sizeof(cmd))
		die("kitty command is too long");
	write_all(cmd, (size_t)n);
}

static void upload_frames(void)
{
	for (int i = 0; i < nframes; ++i)
		upload_frame(frames[i].path, i == 0);
}

static void send_frame(int frame)
{
	char cmd[128];
	int n = snprintf(cmd, sizeof(cmd),
		"\033_Ga=a,i=%u,c=%d,q=2\033\\", image_id, frame);
	write_all(cmd, (size_t)n);
}

static void send_placement(int col, int row, int ox, int oy)
{
	char cmd[256];
	int n = snprintf(cmd, sizeof(cmd),
		"\033[%d;%dH\033_Ga=p,i=%u,p=%u,C=1,X=%d,Y=%d,q=2\033\\",
		row + 1, col + 1, image_id, placement_id, ox, oy);
	write_all(cmd, (size_t)n);
}

static double random_range(double lo, double hi)
{
	return lo + (hi - lo) * (rand() / (RAND_MAX + 1.0));
}

static void clamp_pet(void)
{
	double maxx = term.xpix - sprite_w - PET_MARGIN;
	double maxy = term.ypix - sprite_h - PET_MARGIN;
	if (maxx < PET_MARGIN)
		maxx = PET_MARGIN;
	if (maxy < PET_MARGIN)
		maxy = PET_MARGIN;
	if (pet.x < PET_MARGIN)
		pet.x = PET_MARGIN;
	if (pet.x > maxx)
		pet.x = maxx;
	if (pet.y < PET_MARGIN)
		pet.y = PET_MARGIN;
	if (pet.y > maxy)
		pet.y = maxy;
}

static enum state direction(double dx, double dy)
{
	double ax = fabs(dx), ay = fabs(dy);
	if (ax < 0.01 && ay < 0.01)
		return IDLE;
	if (ax > ay * 2.0)
		return dx < 0 ? W : E;
	if (ay > ax * 2.0)
		return dy < 0 ? N : S;
	if (dx < 0 && dy < 0)
		return NW;
	if (dx > 0 && dy < 0)
		return NE;
	if (dx < 0 && dy > 0)
		return SW;
	return SE;
}

static void set_state(enum state s)
{
	if (pet.state == s)
		return;
	pet.state = s;
	pet.anim_frame = 0;
	pet.anim_time = 0;
	pet.state_time = 0;
	if (s == IDLE)
		pet.wait_time = random_range(0.8, 2.5);
}

static void pick_target(void)
{
	double maxx = term.xpix - sprite_w - PET_MARGIN;
	double maxy = term.ypix - sprite_h - PET_MARGIN;
	if (maxx < PET_MARGIN)
		maxx = PET_MARGIN;
	if (maxy < PET_MARGIN)
		maxy = PET_MARGIN;
	pet.target_x = random_range(PET_MARGIN, maxx);
	pet.target_y = random_range(PET_MARGIN, maxy);
}

static void move_towards(double tx, double ty, double dt)
{
	double dx = tx - pet.x;
	double dy = ty - pet.y;
	double dist = hypot(dx, dy);
	double step = PET_SPEED * dt;
	if (dist <= 0.01)
		return;
	if (step > dist)
		step = dist;
	pet.x += dx / dist * step;
	pet.y += dy / dist * step;
	clamp_pet();
	set_state(direction(dx, dy));
}

static void update_pet(double dt)
{
	pet.state_time += dt;
	pet.anim_time += dt;

	if (pet.dragging)
		return;
	if (pet.frozen) {
		if (pet.state != SLEEPING)
			set_state(SLEEPING);
		return;
	}
	if (pet.chasing) {
		double tx = pet.mouse_valid ? pet.mouse_x : term.xpix / 2.0;
		double ty = pet.mouse_valid ? pet.mouse_y : term.ypix / 2.0;
		double dx = tx - pet.x, dy = ty - pet.y;
		if (hypot(dx, dy) < 3.0) {
			set_state(IDLE);
		} else {
			move_towards(tx, ty, dt);
		}
		return;
	}

	switch (pet.state) {
	case HAPPY:
		if (pet.state_time >= HAPPY_SECONDS)
			set_state(IDLE);
		break;
	case SLEEPING:
		if (pet.state_time >= SLEEP_SECONDS)
			set_state(IDLE);
		break;
	case IDLE:
		if (pet.state_time >= pet.wait_time) {
			pick_target();
			set_state(direction(pet.target_x - pet.x,
					    pet.target_y - pet.y));
		}
		break;
	case DRAGGED:
		break;
	default: {
		double dx = pet.target_x - pet.x;
		double dy = pet.target_y - pet.y;
		if (hypot(dx, dy) < 0.5) {
			pet.x = pet.target_x;
			pet.y = pet.target_y;
			clamp_pet();
			set_state(IDLE);
			if (random_range(0.0, 1.0) < 0.2)
				set_state(SLEEPING);
		} else {
			move_towards(pet.target_x, pet.target_y, dt);
		}
		break;
	}
	}
}

static void update_animation(void)
{
	int count = animations[pet.state].count;
	int frame;
	if (count <= 0)
		return;
	frame = (int)(pet.anim_time / FRAME_SECONDS) % count;
	pet.anim_frame = frame;
}

static int frame_number(void)
{
	int s = pet.state;
	int count = animations[s].count;
	int frame;
	if (count <= 0) {
		s = IDLE;
		count = animations[s].count;
	}
	if (count <= 0)
		return 1;
	frame = pet.anim_frame;
	if (frame < 0)
		frame = 0;
	if (frame >= count)
		frame = frame % count;
	return animations[s].first + frame + 1;
}

static void render(void)
{
	int px = (int)lround(pet.x);
	int py = (int)lround(pet.y);
	int maxx = term.xpix - sprite_w;
	int maxy = term.ypix - sprite_h;
	int col, row, ox, oy, frame;

	if (maxx < 0)
		maxx = 0;
	if (maxy < 0)
		maxy = 0;
	if (px < 0)
		px = 0;
	if (px > maxx)
		px = maxx;
	if (py < 0)
		py = 0;
	if (py > maxy)
		py = maxy;

	col = px / term.cw;
	row = py / term.ch;
	ox = px % term.cw;
	oy = py % term.ch;

	frame = frame_number();
	if (frame != last_frame) {
		send_frame(frame);
		last_frame = frame;
	}
	if (col != last_col || row != last_row || ox != last_ox ||
	    oy != last_oy) {
		send_placement(col, row, ox, oy);
		last_col = col;
		last_row = row;
		last_ox = ox;
		last_oy = oy;
	}
}

static void set_happy(void)
{
	pet.chasing = false;
	pet.frozen = false;
	pet.dragging = false;
	set_state(HAPPY);
}

static void toggle_freeze(void)
{
	if (pet.frozen) {
		pet.frozen = false;
		set_state(IDLE);
	} else {
		pet.dragging = false;
		pet.chasing = false;
		pet.frozen = true;
		set_state(SLEEPING);
	}
}

static void toggle_chase(void)
{
	pet.frozen = false;
	pet.chasing = !pet.chasing;
	if (pet.chasing) {
		if (!pet.mouse_valid) {
			pet.mouse_x = term.xpix / 2.0;
			pet.mouse_y = term.ypix / 2.0;
		}
	} else {
		set_state(IDLE);
	}
}

static void handle_key(unsigned char c)
{
	if (opt.screensaver) {
		quit_requested = 1;
		return;
	}
	switch (c) {
	case 'q':
	case 3:
	case 4:
		quit_requested = 1;
		break;
	case 'f':
		toggle_chase();
		break;
	case 's':
		toggle_freeze();
		break;
	case 'h':
	case ' ':
		set_happy();
		break;
	default:
		break;
	}
}

static void handle_mouse(int button, int x, int y, char final)
{
	int px, py;
	int inside;

	if (opt.screensaver) {
		quit_requested = 1;
		return;
	}
	px = (x - 1) * term.cw + term.cw / 2;
	py = (y - 1) * term.ch + term.ch / 2;
	if (px < 0)
		px = 0;
	if (py < 0)
		py = 0;
	pet.mouse_x = px;
	pet.mouse_y = py;
	pet.mouse_valid = true;

	inside = px >= pet.x - MOUSE_TOLERANCE &&
		 px <= pet.x + sprite_w + MOUSE_TOLERANCE &&
		 py >= pet.y - MOUSE_TOLERANCE &&
		 py <= pet.y + sprite_h + MOUSE_TOLERANCE;

	if (final == 'm') {
		if (pet.dragging) {
			pet.dragging = false;
			set_happy();
		}
		return;
	}
	if (button & 64)
		return;
	if (button & 32) {
		if (pet.dragging) {
			pet.x = px - pet.drag_off_x;
			pet.y = py - pet.drag_off_y;
			clamp_pet();
		}
		return;
	}
	if ((button & 3) == 0 && inside) {
		pet.chasing = false;
		pet.frozen = false;
		pet.dragging = true;
		pet.drag_off_x = px - pet.x;
		pet.drag_off_y = py - pet.y;
		set_state(DRAGGED);
	}
}

static void handle_csi(const char *s, char final)
{
	int button, x, y;
	if (s[0] == '<' && (final == 'M' || final == 'm') &&
	    sscanf(s + 1, "%d;%d;%d", &button, &x, &y) == 3)
		handle_mouse(button, x, y, final);
}

static void feed_input(unsigned char c)
{
	switch (parser.state) {
	case 0:
		if (c == 0x1b) {
			parser.state = 1;
		} else {
			handle_key(c);
		}
		break;
	case 1:
		if (c == '[') {
			parser.state = 2;
			parser.csi_len = 0;
		} else {
			parser.state = 0;
			if (opt.screensaver)
				quit_requested = 1;
		}
		break;
	default:
		if (c >= 0x40 && c <= 0x7e) {
			parser.csi[parser.csi_len] = '\0';
			handle_csi(parser.csi, (char)c);
			parser.state = 0;
			parser.csi_len = 0;
		} else if (parser.csi_len + 1 < sizeof(parser.csi)) {
			parser.csi[parser.csi_len++] = (char)c;
		} else {
			parser.state = 0;
			parser.csi_len = 0;
		}
		break;
	}
}

static void run(void)
{
	double last = now_seconds();
	struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };

	for (;;) {
		double now, dt;
		unsigned char buf[256];
		ssize_t n;

		if (quit_requested)
			break;
		if (resize_requested) {
			resize_requested = 0;
			get_size();
			clamp_pet();
			last_col = last_row = -1;
		}
		now = now_seconds();
		dt = now - last;
		if (dt < 0)
			dt = 0;
		if (dt > 0.1)
			dt = 0.1;
		last = now;

		update_pet(dt);
		update_animation();
		render();

		if (poll(&pfd, 1, TICK_MS) < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (!(pfd.revents & POLLIN))
			continue;
		n = read(STDIN_FILENO, buf, sizeof(buf));
		if (n <= 0)
			break;
		for (ssize_t i = 0; i < n; ++i)
			feed_input(buf[i]);
	}
}

static void usage(const char *name)
{
	fprintf(stderr,
		"usage: %s [--pet neko|dog|bsd] [--assets DIR] [--screensaver]\n"
		"\n"
		"keys: q quit, f chase mouse, s freeze, h/space happy\n",
		name);
}

static void parse_args(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--pet") && i + 1 < argc) {
			opt.pet = argv[++i];
		} else if (!strcmp(argv[i], "--assets") && i + 1 < argc) {
			opt.assets = argv[++i];
		} else if (!strcmp(argv[i], "--screensaver")) {
			opt.screensaver = true;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage(argv[0]);
			exit(0);
		} else {
			usage(argv[0]);
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	self_name = argv[0];
	parse_args(argc, argv);
	srand((unsigned)time(NULL) ^ (unsigned)getpid());
	image_id = ((uint32_t)getpid() * 2654435761u) ^ 0x4b504554u;
	if (!image_id)
		image_id = 0x4b504554u;

	atexit(term_leave);
	install_signals();
	term_enter();
	get_size();
	load_assets();
	upload_frames();

	pet.x = (term.xpix - sprite_w) / 2.0;
	pet.y = (term.ypix - sprite_h) / 2.0;
	clamp_pet();
	pet.mouse_x = pet.x + sprite_w / 2.0;
	pet.mouse_y = pet.y + sprite_h / 2.0;
	pet.mouse_valid = true;
	set_state(IDLE);
	pet.wait_time = 0.5;

	run();
	term_leave();
	return 0;
}
