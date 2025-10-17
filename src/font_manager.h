#ifndef font_manager_h
#define font_manager_h

#include <stb/stb_truetype.h>
#include <stdint.h>
#include <stddef.h>
#include "font_define.h"

struct font_manager;

size_t font_manager_sizeof();
void font_manager_init(struct font_manager *, void *L);
void* font_manager_shutdown(struct font_manager *);
void font_manager_import(struct font_manager *F, void* fontdata, size_t sz);

int font_manager_addfont_with_family(struct font_manager *F, const char* family);
void font_manager_fontheight(struct font_manager *F, int fontid, int size, int *ascent, int *descent, int *lineGap);
int font_manager_pixelsize(struct font_manager *F, int fontid, int pointsize);
const char* font_manager_glyph(struct font_manager *F, int fontid, int codepoint, int size, struct font_glyph *g, struct font_glyph *og);
//int font_manager_touch(struct font_manager *, int font, int codepoint, struct font_glyph *glyph);
//const char * font_manager_update(struct font_manager *, int font, int codepoint, struct font_glyph *glyph, uint8_t *buffer, int stride);
int font_manager_flush(struct font_manager *);
void font_manager_scale(struct font_manager *F, struct font_glyph *glyph, int size);
int font_manager_underline(struct font_manager *F, int fontid, int size, float *underline_position, float *thickness);
float font_manager_sdf_mask(struct font_manager *F);
float font_manager_sdf_distance(struct font_manager *F, uint8_t numpixel);
void font_manager_icon_init(struct font_manager *F, int n, void *data);
int font_manager_enum_fontname(struct font_manager *F, int idx, char buffer[], int buf_sz);

// for debug
const void * font_manager_texture(struct font_manager *F, int *sz);

#endif //font_manager_h
