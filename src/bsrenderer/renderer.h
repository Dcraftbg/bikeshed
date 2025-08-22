#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "color.h"

typedef struct BSFill BSFill;
typedef struct BSFont BSFont;
typedef struct {
    float width, height;
} BSCodepointSize;
typedef struct BSRenderer BSRenderer;
struct BSRenderer {
    void (*clear_background)(BSRenderer*, const BSFill* fill);
    BSCodepointSize (*measure_codepoint)(BSRenderer*, BSFont* font, int codepoint, float fontSize, float spacing);
    void (*draw_codepoint)(BSRenderer*, BSFont* font, int codepoint, float x, float y, float fontSize, BSColor color);
    void (*draw_rectangle)(BSRenderer*, float x, float y, float width, float height, const BSFill* fill);
    // TODO: accept generic allocator for allocating state
    bool (*load_font)(BSRenderer*, const char* path, BSFont* result);
    void *private_data;
};

// Convenient wrappers around those calls^
void bsrenderer_clear_background(BSRenderer* renderer, const BSFill* fill);
void bsrenderer_clear_background_color(BSRenderer* renderer, BSColor color);
BSCodepointSize bsrenderer_measure_codepoint(BSRenderer*, BSFont* font, int codepoint, float fontSize, float spacing);
void bsrenderer_draw_codepoint(BSRenderer*, BSFont* font, int codepoint, float x, float y, float fontSize, BSColor color);
void bsrenderer_draw_rectangle(BSRenderer*, float x, float y, float width, float height, const BSFill* fill);
void bsrenderer_draw_rectangle_color(BSRenderer* renderer, float x, float y, float width, float height, BSColor color);
bool bsrenderer_load_font(BSRenderer*, const char* path, BSFont* result);
