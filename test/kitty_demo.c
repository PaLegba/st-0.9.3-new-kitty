/*
 * Kitty graphics protocol demo for st with kitty graphics support.
 *
 * This program talks directly to the terminal using the kitty graphics
 * protocol. It does not depend on st's source code. Run it inside st, not as
 * `st -e ./kitty_demo`, otherwise the terminal closes when the demo exits.
 *
 * Modes:
 *   ./kitty_demo                     animated primitives (terminal driven loop)
 *   ./kitty_demo --live              animated primitives, generated in real time
 *   ./kitty_demo --images file ...   display images using file transmission
 *
 * Options:
 *   --width N    animation width in pixels (default 320)
 *   --height N   animation height in pixels (default 180)
 *   --frames N   number of precomputed frames (default 120)
 *   --fps N      animation frame rate (default 20)
 *   --id N       image id (default derived from the process id)
 *   --hold SEC   keep the process alive after uploading (default 0)
 *   --full       with --live, fit proportionally in the terminal (black bars)
 *
 * The protocol requires base64 chunks of at most 4096 characters. We use
 * 3072-byte raw chunks, which encode to exactly 4096 base64 characters.
 */

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#define B64_RAW_CHUNK 3072u
#define PI 3.14159265358979323846

typedef struct {
	int width, height;
	uint32_t *pixels; /* 0xRRGGBBAA */
} Canvas;

typedef struct {
	int width, height, frames, fps;
	uint32_t image_id;
	int live;
	int full;
	double hold;
	const char **images;
	int image_count;
} Options;

static const char b64_table[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint32_t rgba(unsigned r, unsigned g, unsigned b, unsigned a) {
	return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) |
	       (a & 0xff);
}

static uint32_t hsv(double h, double s, double v, double a) {
	h = fmod(h, 1.0);
	if (h < 0)
		h += 1.0;
	double c = v * s;
	double x = c * (1 - fabs(fmod(h * 6.0, 2.0) - 1));
	double m = v - c;
	double r = 0, g = 0, b = 0;
	switch ((int)(h * 6.0) % 6) {
	case 0: r = c; g = x; break;
	case 1: r = x; g = c; break;
	case 2: g = c; b = x; break;
	case 3: g = x; b = c; break;
	case 4: r = x; b = c; break;
	default: r = c; b = x; break;
	}
	return rgba((unsigned)((r + m) * 255), (unsigned)((g + m) * 255),
		    (unsigned)((b + m) * 255), (unsigned)(a * 255));
}

static Canvas *canvas_create(int width, int height) {
	Canvas *c = calloc(1, sizeof(*c));
	if (!c)
		return NULL;
	c->width = width;
	c->height = height;
	c->pixels = calloc((size_t)width * height, sizeof(*c->pixels));
	if (!c->pixels) {
		free(c);
		return NULL;
	}
	return c;
}

static void canvas_free(Canvas *c) {
	if (!c)
		return;
	free(c->pixels);
	free(c);
}

/* Straight alpha source-over blending. */
static void blend_pixel(Canvas *c, int x, int y, uint32_t color) {
	if (x < 0 || x >= c->width || y < 0 || y >= c->height)
		return;
	unsigned sa = color & 0xff;
	if (sa == 0)
		return;
	uint32_t *dst = &c->pixels[(size_t)y * c->width + x];
	if (sa == 255) {
		*dst = color;
		return;
	}
	unsigned da = *dst & 0xff;
	unsigned outa = sa + da * (255 - sa) / 255;
	if (outa == 0) {
		*dst = 0;
		return;
	}
	unsigned sr = (color >> 24) & 0xff, sg = (color >> 16) & 0xff,
		 sb = (color >> 8) & 0xff;
	unsigned dr = (*dst >> 24) & 0xff, dg = (*dst >> 16) & 0xff,
		 db = (*dst >> 8) & 0xff;
	unsigned r = (sr * sa + dr * da * (255 - sa) / 255) / outa;
	unsigned g = (sg * sa + dg * da * (255 - sa) / 255) / outa;
	unsigned b = (sb * sa + db * da * (255 - sa) / 255) / outa;
	*dst = rgba(r, g, b, outa);
}

static void fill_rect(Canvas *c, int x, int y, int w, int h, uint32_t color) {
	int x0 = x < 0 ? 0 : x;
	int y0 = y < 0 ? 0 : y;
	int x1 = x + w > c->width ? c->width : x + w;
	int y1 = y + h > c->height ? c->height : y + h;
	for (int yy = y0; yy < y1; ++yy)
		for (int xx = x0; xx < x1; ++xx)
			blend_pixel(c, xx, yy, color);
}

static void fill_circle(Canvas *c, double cx, double cy, double r,
			uint32_t color) {
	int x0 = (int)floor(cx - r - 1), x1 = (int)ceil(cx + r + 1);
	int y0 = (int)floor(cy - r - 1), y1 = (int)ceil(cy + r + 1);
	for (int y = y0; y <= y1; ++y) {
		for (int x = x0; x <= x1; ++x) {
			double dx = x + 0.5 - cx, dy = y + 0.5 - cy;
			double d = sqrt(dx * dx + dy * dy) - r;
			double coverage = 0.5 - d;
			if (coverage <= 0)
				continue;
			if (coverage > 1)
				coverage = 1;
			unsigned a = (unsigned)(((color & 0xff) / 255.0) *
						coverage * 255 + 0.5);
			uint32_t cc = (color & 0xffffff00u) | a;
			blend_pixel(c, x, y, cc);
		}
	}
}

static void draw_line(Canvas *c, double x0, double y0, double x1, double y1,
		      double thickness, uint32_t color) {
	double dx = x1 - x0, dy = y1 - y0;
	double len2 = dx * dx + dy * dy;
	double half = thickness / 2.0;
	int minx = (int)floor(fmin(x0, x1) - half - 1);
	int maxx = (int)ceil(fmax(x0, x1) + half + 1);
	int miny = (int)floor(fmin(y0, y1) - half - 1);
	int maxy = (int)ceil(fmax(y0, y1) + half + 1);
	if (len2 == 0) {
		fill_circle(c, x0, y0, half, color);
		return;
	}
	for (int y = miny; y <= maxy; ++y) {
		for (int x = minx; x <= maxx; ++x) {
			double px = x + 0.5 - x0, py = y + 0.5 - y0;
			double t = (px * dx + py * dy) / len2;
			if (t < 0)
				t = 0;
			if (t > 1)
				t = 1;
			double qx = x0 + t * dx, qy = y0 + t * dy;
			double ddx = x + 0.5 - qx, ddy = y + 0.5 - qy;
			double d = sqrt(ddx * ddx + ddy * ddy) - half;
			double coverage = 0.5 - d;
			if (coverage <= 0)
				continue;
			if (coverage > 1)
				coverage = 1;
			unsigned a = (unsigned)(((color & 0xff) / 255.0) *
						coverage * 255 + 0.5);
			uint32_t cc = (color & 0xffffff00u) | a;
			blend_pixel(c, x, y, cc);
		}
	}
}

static double edge(double ax, double ay, double bx, double by, double px,
		   double py) {
	return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

static void fill_triangle(Canvas *c, double x0, double y0, double x1, double y1,
			  double x2, double y2, uint32_t color) {
	int minx = (int)floor(fmin(x0, fmin(x1, x2)));
	int maxx = (int)ceil(fmax(x0, fmax(x1, x2)));
	int miny = (int)floor(fmin(y0, fmin(y1, y2)));
	int maxy = (int)ceil(fmax(y0, fmax(y1, y2)));
	for (int y = miny; y <= maxy; ++y) {
		for (int x = minx; x <= maxx; ++x) {
			double px = x + 0.5, py = y + 0.5;
			double e0 = edge(x0, y0, x1, y1, px, py);
			double e1 = edge(x1, y1, x2, y2, px, py);
			double e2 = edge(x2, y2, x0, y0, px, py);
			if ((e0 >= 0 && e1 >= 0 && e2 >= 0) ||
			    (e0 <= 0 && e1 <= 0 && e2 <= 0))
				blend_pixel(c, x, y, color);
		}
	}
}

static void draw_ring(Canvas *c, double cx, double cy, double r,
		      double thickness, uint32_t color) {
	int steps = (int)(r * 8) + 24;
	double prevx = cx + r, prevy = cy;
	for (int i = 1; i <= steps; ++i) {
		double a = i * 2.0 * PI / steps;
		double x = cx + cos(a) * r, y = cy + sin(a) * r;
		draw_line(c, prevx, prevy, x, y, thickness, color);
		prevx = x;
		prevy = y;
	}
}

static void background_gradient(Canvas *c, double t) {
	for (int y = 0; y < c->height; ++y) {
		for (int x = 0; x < c->width; ++x) {
			double u = (double)x / c->width;
			double v = (double)y / c->height;
			double r = 0.03 + 0.10 * v + 0.04 * sin((t + u) * 6.0);
			double g = 0.04 + 0.12 * u + 0.05 * sin((t + v) * 5.0);
			double b = 0.08 + 0.18 * (1.0 - v) +
				   0.05 * cos((t + u + v) * 4.0);
			c->pixels[(size_t)y * c->width + x] = rgba(
				(unsigned)(r * 255), (unsigned)(g * 255),
				(unsigned)(b * 255), 255);
		}
	}
}

static void scene_bouncing(Canvas *c, double t) {
	background_gradient(c, t);
	int balls = 7;
	for (int i = 0; i < balls; ++i) {
		double phase = t * 2.0 * PI + i * 2.0 * PI / balls;
		double fx = 0.5 + 0.36 * sin(phase * (1.0 + i * 0.07));
		double fy = 0.5 + 0.34 * cos(phase * (1.3 + i * 0.05));
		double x = fx * c->width, y = fy * c->height;
		double r = 8 + 5 * (0.5 + 0.5 * sin(phase * 2.0));
		uint32_t col = hsv(t * 0.2 + i * 0.11, 0.85, 0.95, 0.75);
		for (int glow = 3; glow >= 1; --glow)
			fill_circle(c, x, y, r * (1.0 + glow * 0.35),
				    hsv(t * 0.2 + i * 0.11, 0.8, 0.9,
					0.08 / glow));
		fill_circle(c, x, y, r, col);
		fill_circle(c, x - r * 0.3, y - r * 0.3, r * 0.25,
			    rgba(255, 255, 255, 140));
	}
	/* A couple of outlined rings for contrast. */
	draw_ring(c, c->width * 0.5, c->height * 0.5,
		  c->height * (0.30 + 0.04 * sin(t * 2.0 * PI)), 2.0,
		  hsv(t * 0.3, 0.5, 0.9, 0.35));
}

static void rotate_point(double x, double y, double cx, double cy, double a,
			 double *ox, double *oy) {
	double dx = x - cx, dy = y - cy;
	*ox = cx + dx * cos(a) - dy * sin(a);
	*oy = cy + dx * sin(a) + dy * cos(a);
}

static void scene_geometry(Canvas *c, double t) {
	background_gradient(c, t);
	double cx = c->width * 0.5, cy = c->height * 0.5;
	double r = c->height * 0.28;
	double a = t * 2.0 * PI;
	double x0, y0, x1, y1, x2, y2;
	rotate_point(cx, cy - r, cx, cy, a, &x0, &y0);
	rotate_point(cx + r * 0.9, cy + r * 0.5, cx, cy, a, &x1, &y1);
	rotate_point(cx - r * 0.9, cy + r * 0.5, cx, cy, a, &x2, &y2);
	fill_triangle(c, x0, y0, x1, y1, x2, y2,
		      hsv(t + 0.1, 0.8, 0.95, 0.78));
	draw_line(c, x0, y0, x1, y1, 2.0, rgba(255, 255, 255, 140));
	draw_line(c, x1, y1, x2, y2, 2.0, rgba(255, 255, 255, 140));
	draw_line(c, x2, y2, x0, y0, 2.0, rgba(255, 255, 255, 140));

	double s = c->height * 0.18;
	double qx[4] = {cx - s, cx + s, cx + s, cx - s};
	double qy[4] = {cy - s, cy - s, cy + s, cy + s};
	double rx[4], ry[4];
	for (int i = 0; i < 4; ++i)
		rotate_point(qx[i], qy[i], cx, cy, -a * 1.4, &rx[i], &ry[i]);
	for (int i = 0; i < 4; ++i)
		draw_line(c, rx[i], ry[i], rx[(i + 1) % 4],
			  ry[(i + 1) % 4], 3.0,
			  hsv(t * 1.3 + i * 0.08, 0.7, 1.0, 0.8));
}

static void scene_waves(Canvas *c, double t) {
	background_gradient(c, t);
	for (int k = 0; k < 5; ++k) {
		uint32_t col = hsv(t * 0.25 + k * 0.08, 0.75, 0.95, 0.75);
		double prevx = 0, prevy = c->height * 0.5;
		for (int x = 0; x <= c->width; x += 2) {
			double u = (double)x / c->width;
			double y = c->height * (0.5 +
					       0.20 * sin(u * 8.0 * PI +
							 t * 2.0 * PI +
							 k * 0.7) *
						       sin(u * PI));
			if (x)
				draw_line(c, prevx, prevy, x, y, 2.0, col);
			prevx = x;
			prevy = y;
		}
	}
	/* Moving vertical bars at the bottom. */
	for (int i = 0; i < 24; ++i) {
		double phase = t * 2.0 * PI + i * 0.5;
		double h = 8 + 22 * (0.5 + 0.5 * sin(phase));
		fill_rect(c, (int)(i * c->width / 24.0), c->height - (int)h,
			  c->width / 32, (int)h,
			  hsv(t * 0.4 + i * 0.03, 0.7, 0.9, 0.55));
	}
}

static void scene_particles(Canvas *c, double t) {
	background_gradient(c, t);
	double cx = c->width * 0.5, cy = c->height * 0.5;
	for (int i = 0; i < 180; ++i) {
		double base = (double)i / 180.0;
		double a = base * 10.0 * PI + t * 2.0 * PI * 0.8;
		double radius = (0.08 + 0.38 * base) * c->height *
				(0.8 + 0.2 * sin(t * 2.0 * PI + base * 9.0));
		double x = cx + cos(a) * radius;
		double y = cy + sin(a) * radius * 0.8;
		double r = 1.5 + 3.0 * (0.5 + 0.5 * sin(a * 2.0 + t * 8.0));
		fill_circle(c, x, y, r, hsv(base + t * 0.25, 0.8, 1.0, 0.9));
	}
	double pulse = 0.5 + 0.5 * sin(t * 2.0 * PI * 3.0);
	fill_circle(c, cx, cy, c->height * (0.03 + 0.04 * pulse),
		    rgba(255, 255, 255, 220));
	fill_circle(c, cx, cy, c->height * (0.10 + 0.05 * pulse),
		    rgba(255, 255, 255, 35));
}

static void render_frame(Canvas *c, double seconds) {
	double t = fmod(seconds, 6.0) / 6.0;
	int scene = (int)(t * 4.0);
	double local = t * 4.0 - scene;
	switch (scene) {
	case 0: scene_bouncing(c, local); break;
	case 1: scene_geometry(c, local); break;
	case 2: scene_waves(c, local); break;
	default: scene_particles(c, local); break;
	}
}

static int b64_encode(const unsigned char *src, size_t len, char *dst) {
	size_t i, o = 0;
	for (i = 0; i + 2 < len; i += 3) {
		unsigned v = (src[i] << 16) | (src[i + 1] << 8) | src[i + 2];
		dst[o++] = b64_table[(v >> 18) & 63];
		dst[o++] = b64_table[(v >> 12) & 63];
		dst[o++] = b64_table[(v >> 6) & 63];
		dst[o++] = b64_table[v & 63];
	}
	if (i < len) {
		unsigned v = src[i] << 16;
		if (i + 1 < len)
			v |= src[i + 1] << 8;
		dst[o++] = b64_table[(v >> 18) & 63];
		dst[o++] = b64_table[(v >> 12) & 63];
		dst[o++] = i + 1 < len ? b64_table[(v >> 6) & 63] : '=';
		dst[o++] = '=';
	}
	dst[o] = '\0';
	return (int)o;
}

static void write_bytes(const void *data, size_t len) {
	const unsigned char *p = data;
	while (len) {
		ssize_t n = write(STDOUT_FILENO, p, len);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "write failed: %s\n", strerror(errno));
			exit(1);
		}
		p += n;
		len -= (size_t)n;
	}
}

/* Sends `data` as a kitty graphics command split into protocol-sized chunks.
 * `params` is the first command's parameter list (without the leading "a="
 * action if it is already included). It must not contain the m key. */
static void send_chunked(const char *params, const unsigned char *data,
			 size_t len) {
	char b64[4096 + 1];
	size_t offset = 0;
	int first = 1;
	if (len == 0) {
		char cmd[1024];
		int n = snprintf(cmd, sizeof(cmd), "\033_G%s,m=0;\033\\", params);
		write_bytes(cmd, (size_t)n);
		return;
	}
	while (offset < len) {
		size_t n = len - offset;
		if (n > B64_RAW_CHUNK)
			n = B64_RAW_CHUNK;
		int more = offset + n < len;
		b64_encode(data + offset, n, b64);
		if (first) {
			char head[1024];
			int h = snprintf(head, sizeof(head), "\033_G%s,m=%d;",
					 params, more ? 1 : 0);
			write_bytes(head, (size_t)h);
			first = 0;
		} else {
			char head[32];
			int h = snprintf(head, sizeof(head), "\033_Gm=%d;",
					 more ? 1 : 0);
			write_bytes(head, (size_t)h);
		}
		write_bytes(b64, strlen(b64));
		write_bytes("\033\\", 2);
		offset += n;
	}
}

static void send_animation_frame(const char *action, uint32_t image_id,
				 int width, int height, int gap_ms,
				 int full_cols, int full_rows, int no_move,
				 const unsigned char *rgba_data) {
	uLongf compressed_size = compressBound((uLong)width * height * 4);
	unsigned char *compressed = malloc(compressed_size);
	if (!compressed) {
		perror("malloc");
		exit(1);
	}
	int ret = compress2(compressed, &compressed_size, rgba_data,
			    (uLong)width * height * 4, Z_BEST_SPEED);
	if (ret != Z_OK) {
		fprintf(stderr, "zlib compression failed: %d\n", ret);
		exit(1);
	}
	char params[512];
	if (!strcmp(action, "T")) {
		int off = snprintf(params, sizeof(params),
			"a=T,i=%u,f=32,s=%d,v=%d,o=z,z=%d,q=2",
			image_id, width, height, gap_ms);
		if (full_cols > 0)
			off += snprintf(params + off, sizeof(params) - off,
					",c=%d", full_cols);
		if (full_rows > 0)
			off += snprintf(params + off, sizeof(params) - off,
					",r=%d", full_rows);
		if (no_move)
			snprintf(params + off, sizeof(params) - off, ",C=1");
	} else {
		snprintf(params, sizeof(params),
			 "a=f,i=%u,f=32,s=%d,v=%d,o=z,z=%d,q=2",
			 image_id, width, height, gap_ms);
	}
	send_chunked(params, compressed, compressed_size);
	free(compressed);
}

static void send_animation_control(uint32_t image_id, int state, int edit_frame,
				   int gap_ms) {
	char keys[128];
	char cmd[256];
	size_t o = 0;
	if (state)
		o += (size_t)snprintf(keys + o, sizeof(keys) - o, ",s=%d", state);
	if (edit_frame)
		o += (size_t)snprintf(keys + o, sizeof(keys) - o, ",r=%d",
				      edit_frame);
	if (gap_ms)
		o += (size_t)snprintf(keys + o, sizeof(keys) - o, ",z=%d", gap_ms);
	keys[o] = '\0';
	int n = snprintf(cmd, sizeof(cmd), "\033_Ga=a,i=%u,q=2%s\033\\",
			 image_id, keys);
	write_bytes(cmd, (size_t)n);
}

static void send_image_file(const char *path, uint32_t image_id, int cols,
			    int rows) {
	char *absolute = realpath(path, NULL);
	if (!absolute) {
		fprintf(stderr, "cannot resolve %s: %s\n", path,
			strerror(errno));
		return;
	}
	char params[512];
	if (rows > 0) {
		snprintf(params, sizeof(params),
			 "a=T,i=%u,f=100,t=f,q=2,C=1,c=%d,r=%d",
			 image_id, cols, rows);
	} else {
		snprintf(params, sizeof(params),
			 "a=T,i=%u,f=100,t=f,q=2,C=1,c=%d", image_id, cols);
	}
	send_chunked(params, (const unsigned char *)absolute, strlen(absolute));
	free(absolute);
}

static int parse_int(const char *s, int *out) {
	char *end = NULL;
	long v = strtol(s, &end, 10);
	if (!s[0] || *end || v < 1 || v > INT_MAX)
		return 0;
	*out = (int)v;
	return 1;
}

static void usage(const char *argv0) {
	fprintf(stderr,
		"Usage: %s [options]\n"
		"       %s --images FILE... [options]\n"
		"Options:\n"
		"  --live          generate frames continuously instead of a "
		"precomputed loop\n"
		"  --full          fit --live output proportionally (black bars)\n"
		"  --images F...   display the given image files\n"
		"  --width N       animation width in pixels (default 320)\n"
		"  --height N      animation height in pixels (default 180)\n"
		"  --frames N      number of frames (default 120)\n"
		"  --fps N         animation frame rate (default 20)\n"
		"  --id N          image id (default derived from pid)\n"
		"  --hold SEC      keep running after upload (default 0)\n",
		argv0, argv0);
}

static void sleep_ms(double ms) {
	struct timespec ts;
	ts.tv_sec = (time_t)(ms / 1000.0);
	ts.tv_nsec = (long)((ms - ts.tv_sec * 1000.0) * 1000000.0);
	while (nanosleep(&ts, &ts) < 0 && errno == EINTR)
		;
}

static unsigned char *canvas_to_rgba(const Canvas *c) {
	size_t n = (size_t)c->width * c->height;
	unsigned char *out = malloc(n * 4);
	if (!out)
		return NULL;
	for (size_t i = 0; i < n; ++i) {
		uint32_t p = c->pixels[i];
		out[i * 4 + 0] = (p >> 24) & 0xff;
		out[i * 4 + 1] = (p >> 16) & 0xff;
		out[i * 4 + 2] = (p >> 8) & 0xff;
		out[i * 4 + 3] = p & 0xff;
	}
	return out;
}

static void run_precomputed(const Options *o) {
	Canvas *c = canvas_create(o->width, o->height);
	if (!c) {
		perror("canvas");
		exit(1);
	}
	int gap = (int)(1000.0 / o->fps + 0.5);
	if (gap < 1)
		gap = 1;

	for (int i = 0; i < o->frames; ++i) {
		render_frame(c, (double)i / o->fps);
		unsigned char *rgba = canvas_to_rgba(c);
		if (!rgba) {
			perror("rgba");
			exit(1);
		}
		send_animation_frame(i == 0 ? "T" : "f", o->image_id,
				     o->width, o->height, i == 0 ? 0 : gap,
				     0, 0, 0, rgba);
		free(rgba);
		if (i == 0)
			send_animation_control(o->image_id, 0, 1, gap);
	}
	/* Loop forever. */
	send_animation_control(o->image_id, 3, 0, 0);
	canvas_free(c);
}

static void get_term_metrics(int *cols, int *rows, int *cw, int *ch) {
	struct winsize ws;
	int xpix = 0, ypix = 0;
	*cols = 80;
	*rows = 24;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
	    ws.ws_col > 0 && ws.ws_row > 0) {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		xpix = ws.ws_xpixel;
		ypix = ws.ws_ypixel;
	}
	*cw = xpix > 0 ? xpix / *cols : 8;
	*ch = ypix > 0 ? ypix / *rows : 16;
	if (*cw < 1)
		*cw = 1;
	if (*ch < 1)
		*ch = 1;
}

static int query_cursor(int *row, int *col) {
	struct termios saved, raw;
	char buf[64];
	size_t used = 0;
	struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
	int ok = 0;

	if (tcgetattr(STDIN_FILENO, &saved) < 0)
		return 0;
	raw = saved;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &raw) < 0)
		return 0;
	write_bytes("\033[6n", 4);
	while (used + 1 < sizeof(buf)) {
		int r = poll(&pfd, 1, 200);
		if (r <= 0)
			break;
		ssize_t n = read(STDIN_FILENO, buf + used, sizeof(buf) - used - 1);
		if (n <= 0)
			break;
		used += (size_t)n;
		buf[used] = '\0';
		if (strchr(buf, 'R'))
			break;
	}
	buf[used] = '\0';
	if (sscanf(buf, "\033[%d;%dR", row, col) == 2)
		ok = 1;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &saved);
	return ok;
}

static void run_live(const Options *o) {
	Canvas *c = canvas_create(o->width, o->height);
	if (!c) {
		perror("canvas");
		exit(1);
	}
	int gap = (int)(1000.0 / o->fps + 0.5);
	if (gap < 1)
		gap = 1;
	const int restart_after = 200;
	int frame = 0;
	uint32_t id = o->image_id;
	int term_cols = 80, term_rows = 24, cw = 8, ch = 16;
	int image_cols, image_rows;
	int anchor_row = 1, anchor_col = 1;
	int full_cols = 0, full_rows = 0;

	get_term_metrics(&term_cols, &term_rows, &cw, &ch);
	image_cols = (o->width + cw - 1) / cw;
	image_rows = (o->height + ch - 1) / ch;
	if (o->full) {
		double image_aspect = (double)o->width / o->height;
		double term_aspect =
			((double)term_cols * cw) / ((double)term_rows * ch);
		write_bytes("\033[40m\033[2J\033[0m\033[H",
			    sizeof("\033[40m\033[2J\033[0m\033[H") - 1);
		if (image_aspect >= term_aspect) {
			full_cols = term_cols;
			full_rows = 0;
			image_cols = term_cols;
			image_rows = (int)ceil((double)o->height * term_cols *
					       cw / ((double)o->width * ch));
			anchor_col = 1;
			anchor_row = (term_rows - image_rows) / 2 + 1;
		} else {
			full_rows = term_rows;
			full_cols = 0;
			image_rows = term_rows;
			image_cols = (int)ceil((double)o->width * term_rows *
					       ch / ((double)o->height * cw));
			anchor_row = 1;
			anchor_col = (term_cols - image_cols) / 2 + 1;
		}
		if (anchor_row < 1)
			anchor_row = 1;
		if (anchor_col < 1)
			anchor_col = 1;
	} else {
		int row = 1, col = 1;
		query_cursor(&row, &col);
		anchor_row = row;
		anchor_col = col;
		if (anchor_col + image_cols - 1 > term_cols) {
			anchor_col = term_cols - image_cols + 1;
			if (anchor_col < 1)
				anchor_col = 1;
		}
		if (anchor_row + image_rows - 1 >= term_rows) {
			anchor_row = term_rows - image_rows;
			if (anchor_row < 1)
				anchor_row = 1;
		}
	}
	for (;;) {
		if (frame == 0) {
			char pos[32];
			int n = snprintf(pos, sizeof(pos), "\033[%d;%dH",
					 anchor_row, anchor_col);
			write_bytes(pos, (size_t)n);
		}
		render_frame(c, (double)frame / o->fps);
		unsigned char *rgba = canvas_to_rgba(c);
		if (!rgba) {
			perror("rgba");
			exit(1);
		}
		send_animation_frame(frame == 0 ? "T" : "f", id, o->width,
				     o->height, frame == 0 ? 0 : gap,
				     full_cols, full_rows, 1, rgba);
		free(rgba);
		if (frame == 0) {
			char pos[32];
			int below_row;
			int n;
			send_animation_control(id, 0, 1, gap);
			send_animation_control(id, 2, 0, 0); /* loading mode */
			below_row = o->full ? term_rows :
				anchor_row + image_rows;
			if (below_row > term_rows)
				below_row = term_rows;
			n = snprintf(pos, sizeof(pos), "\033[%d;1H", below_row);
			write_bytes(pos, (size_t)n);
		}
		sleep_ms(gap);
		++frame;
		if (frame >= restart_after) {
			/* Delete the placement, then free the stored image. */
			char cmd[128];
			int n = snprintf(cmd, sizeof(cmd),
					 "\033_Ga=d,d=i,i=%u,q=2\033\\", id);
			write_bytes(cmd, (size_t)n);
			n = snprintf(cmd, sizeof(cmd),
				     "\033_Ga=d,d=I,i=%u,q=2\033\\", id);
			write_bytes(cmd, (size_t)n);
			id++;
			frame = 0;
		}
	}
}

int main(int argc, char **argv) {
	Options o = {
		.width = 320,
		.height = 180,
		.frames = 120,
		.fps = 20,
		.image_id = (uint32_t)((getpid() << 8) ^ 0x4b545447u),
		.live = 0,
		.hold = 0,
	};
	if (!o.image_id)
		o.image_id = 0x4b545447u;

	const char **images = calloc((size_t)argc, sizeof(*images));
	if (!images) {
		perror("calloc");
		return 1;
	}
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--live")) {
			o.live = 1;
		} else if (!strcmp(argv[i], "--full")) {
			o.full = 1;
		} else if (!strcmp(argv[i], "--images")) {
			while (i + 1 < argc && argv[i + 1][0] != '-')
				images[o.image_count++] = argv[++i];
		} else if (!strcmp(argv[i], "--width")) {
			if (i + 1 >= argc || !parse_int(argv[++i], &o.width)) {
				fprintf(stderr, "bad --width\n");
				return 1;
			}
		} else if (!strcmp(argv[i], "--height")) {
			if (i + 1 >= argc || !parse_int(argv[++i], &o.height)) {
				fprintf(stderr, "bad --height\n");
				return 1;
			}
		} else if (!strcmp(argv[i], "--frames")) {
			if (i + 1 >= argc || !parse_int(argv[++i], &o.frames)) {
				fprintf(stderr, "bad --frames\n");
				return 1;
			}
		} else if (!strcmp(argv[i], "--fps")) {
			if (i + 1 >= argc || !parse_int(argv[++i], &o.fps)) {
				fprintf(stderr, "bad --fps\n");
				return 1;
			}
		} else if (!strcmp(argv[i], "--id")) {
			int id;
			if (i + 1 >= argc || !parse_int(argv[++i], &id)) {
				fprintf(stderr, "bad --id\n");
				return 1;
			}
			o.image_id = (uint32_t)id;
		} else if (!strcmp(argv[i], "--hold")) {
			int hold;
			if (i + 1 >= argc || !parse_int(argv[++i], &hold)) {
				fprintf(stderr, "bad --hold\n");
				return 1;
			}
			o.hold = (double)hold;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage(argv[0]);
			return 0;
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "unknown option: %s\n", argv[i]);
			usage(argv[0]);
			return 1;
		} else {
			images[o.image_count++] = argv[i];
		}
	}
	o.images = images;

	if (o.width > 4096 || o.height > 4096 || o.frames > 100000 ||
	    o.fps > 240) {
		fprintf(stderr, "unreasonable dimensions or frame count\n");
		return 1;
	}
	if (o.image_count) {
		struct winsize ws;
		int term_cols = 80, term_rows = 24;
		int gap = 2;
		int cell_w;
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
		    ws.ws_col > 0 && ws.ws_row > 0) {
			term_cols = ws.ws_col;
			term_rows = ws.ws_row;
		}
		cell_w = (term_cols - gap * (o.image_count + 1)) /
			 o.image_count;
		if (cell_w > 32)
			cell_w = 32;
		if (cell_w < 1)
			cell_w = 1;
		printf("\033[H\033[2J");
		fflush(stdout);
		for (int i = 0; i < o.image_count; ++i) {
			int col = gap + i * (cell_w + gap);
			printf("\033[2;%dH", col + 1);
			fflush(stdout);
			send_image_file(o.images[i], o.image_id + (uint32_t)i,
					cell_w, 0);
		}
		printf("\033[%d;1H", term_rows);
		fflush(stdout);
	} else if (o.live) {
		run_live(&o);
	} else {
		run_precomputed(&o);
		/* The cursor was moved after the image placement. Put the following
		 * shell prompt on a fresh line below the image. */
		write_bytes("\r\n", 2);
	}
	fflush(stdout);
	if (o.hold > 0)
		sleep_ms(o.hold * 1000.0);
	free(images);
	return 0;
}
