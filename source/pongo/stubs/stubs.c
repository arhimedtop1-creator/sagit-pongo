#define PONGO_VERSION_TXT "arhimed-X2.6.3-21-gcc532fa"
#define AT_FDCWD (-100)
#define O_RDWR 2
#define PROT_RW 3
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_ANON 0x20
#define FBIOGET_VSCREENINFO 0x4600
#define FBIOGET_FSCREENINFO 0x4602
#define FBIOPAN_DISPLAY 0x4606

#include <pongo.h>
#include "logo_apple.h"

struct fb_bitfield { unsigned offset, length, msb_right; };
struct fb_var_screeninfo {
    unsigned xres, yres, xres_virtual, yres_virtual, xoffset, yoffset;
    unsigned bits_per_pixel, grayscale;
    struct fb_bitfield red, green, blue, transp;
    unsigned nonstd, activate, height, width, accel_flags;
    unsigned pixclock, left_margin, right_margin, upper_margin, lower_margin;
    unsigned hsync_len, vsync_len, sync, vmode, rotate, colorspace, reserved[4];
};
struct fb_fix_screeninfo {
    char id[16];
    unsigned long smem_start;
    unsigned smem_len, type, type_aux, visual;
    unsigned short xpanstep, ypanstep, ywrapstep;
    unsigned line_length;
    unsigned long mmio_start;
    unsigned mmio_len, accel;
    unsigned short capabilities, reserved[2];
};

static long sc(long n, long a, long b, long c, long d, long e, long f)
{
    register long x8 __asm__("x8") = n;
    register long x0 __asm__("x0") = a;
    register long x1 __asm__("x1") = b;
    register long x2 __asm__("x2") = c;
    register long x3 __asm__("x3") = d;
    register long x4 __asm__("x4") = e;
    register long x5 __asm__("x5") = f;
    __asm__ volatile("svc #0" : "+r"(x0)
                     : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
                     : "memory");
    return x0;
}

static void sleep_ms(long ms)
{
    long ts[2] = { ms / 1000, (ms % 1000) * 1000000L };
    sc(101, (long)ts, 0, 0, 0, 0, 0);
}

extern uint32_t basecolor;
static int fbfd = -1;
static struct fb_var_screeninfo fbvar;
static struct fb_fix_screeninfo fbfix;
static struct boot_args boot_storage;

struct boot_args *gBootArgs = &boot_storage;
uint32_t socnum = 0;
uint32_t *fb_base;

static void stub_present(void)
{
    fbvar.yoffset = 0;
    sc(29, fbfd, FBIOPAN_DISPLAY, (long)&fbvar, 0, 0, 0);
}

int is_16k(void) { return 0; }

void map_range(uint64_t va, uint64_t pa, uint64_t size, int type, int a, int b)
{
    (void)va; (void)pa; (void)size; (void)type; (void)a; (void)b;
}

void *alloc_contig(size_t size)
{
    long p = sc(222, 0, (long)size, PROT_RW, MAP_PRIVATE | MAP_ANON, -1, 0);
    if ((unsigned long)p >= (unsigned long)-4096) return 0;
    return (void *)p;
}

void cache_clean(void *p, size_t size)
{
    (void)p; (void)size;
    stub_present();
}

void disable_interrupts(void) {}
void enable_interrupts(void) {}

void command_register(const char *name, const char *desc, void *fn)
{
    (void)name; (void)desc; (void)fn;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n--) *d++ = (unsigned char)c;
    return dst;
}

static uint32_t ox_apple, oy_apple;

static void draw_apple(void)
{
    uint32_t ox = (fbvar.xres - LOGO_W) / 2;
    uint32_t oy = (fbvar.yres - LOGO_H) / 2;
    ox_apple = ox;
    oy_apple = oy;
    uint32_t stride = fbfix.line_length / 4;
    for (uint32_t y = 0; y < LOGO_H; y++)
        for (uint32_t x = 0; x < LOGO_W; x++) {
            uint32_t a = logo_alpha[y * LOGO_W + x];
            if (!a) continue;
            uint32_t idx = (oy + y) * stride + (ox + x);
            uint32_t bg = fb_base[idx] & 0xFF;
            uint32_t v = (255 * a + bg * (255 - a)) / 255;
            fb_base[idx] = 0xFF000000u | (v * 0x010101u);
        }
}

void draw_checkra1n_logo(uint32_t left, uint32_t bottom);

void _start(void)
{
    for (int i = 0; i < 300 && fbfd < 0; i++) {
        fbfd = (int)sc(56, AT_FDCWD, (long)"/dev/graphics/fb0", O_RDWR, 0, 0, 0);
        if (fbfd < 0) sleep_ms(100);
    }
    if (fbfd < 0) sc(94, 1, 0, 0, 0, 0, 0);

    if (sc(29, fbfd, FBIOGET_VSCREENINFO, (long)&fbvar, 0, 0, 0) < 0 ||
        sc(29, fbfd, FBIOGET_FSCREENINFO, (long)&fbfix, 0, 0, 0) < 0)
        sc(94, 2, 0, 0, 0, 0, 0);

    if (fbvar.bits_per_pixel != 32) sc(94, 3, 0, 0, 0, 0, 0);

    long addr = sc(222, 0, (long)fbfix.smem_len, PROT_RW, MAP_SHARED, fbfd, 0);
    if ((unsigned long)addr >= (unsigned long)-4096) sc(94, 4, 0, 0, 0, 0, 0);
    fb_base = (uint32_t *)addr;

    gBootArgs->Video.v_baseAddr = 0;
    gBootArgs->Video.v_rowBytes = fbfix.line_length;
    gBootArgs->Video.v_width = fbvar.xres;
    gBootArgs->Video.v_height = fbvar.yres;
    gBootArgs->Video.v_depth = 32;

    for (unsigned long i = 0; i < fbfix.smem_len / 4; i++)
        fb_base[i] = 0xFF000000u;
    stub_present();

    y_cursor = 0;
    x_cursor = LEFT_MARGIN;

    screen_init();

    draw_apple();
    draw_checkra1n_logo(ox_apple, oy_apple + LOGO_H);

    /* шапка мелким шрифтом: basecolor чёрный даёт белый текст на чёрном фоне */
    basecolor = 0xFF000000u;
    scale_factor = 1;
    y_cursor = 0;
    x_cursor = LEFT_MARGIN;
    scale_factor = 2;
    screen_puts("");
    screen_puts("#==================");
    screen_puts("#");
    screen_puts("# artOS " PONGO_VERSION_TXT);
    screen_puts("#");
    screen_puts("# arhimed-TEAM");
    screen_puts("#");
    screen_puts("#==================");
    screen_mark_banner();

    screen_write("pongoOS> ");

    for (;;) sleep_ms(1000);
}
