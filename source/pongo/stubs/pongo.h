#ifndef PONGO_H
#define PONGO_H
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long size_t;

struct video_args {
    uint64_t v_baseAddr;
    uint32_t v_rowBytes;
    uint32_t v_width;
    uint32_t v_height;
    uint32_t v_depth;
};
struct boot_args { struct video_args Video; };

extern struct boot_args *gBootArgs;
extern uint32_t socnum;
extern uint8_t scale_factor;
extern uint32_t *fb_base;
extern uint32_t y_cursor, x_cursor, bannerHeight;

#define SCALE_FACTOR scale_factor
#define LEFT_MARGIN 16

void screen_init(void);
void screen_write(const char *str);
void screen_puts(const char *str);
void screen_mark_banner(void);
void screen_fill(uint32_t color);
void screen_invert(void);
void screen_clear_all(void);
int is_16k(void);
void map_range(uint64_t va, uint64_t pa, uint64_t size, int type, int a, int b);
void *alloc_contig(size_t size);
void cache_clean(void *p, size_t size);
void disable_interrupts(void);
void enable_interrupts(void);
void command_register(const char *name, const char *desc, void *fn);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
#endif
