#include "renderer.h"
#include "fill.h"

BSCodepointSize bsrenderer_measure_codepoint(BSRenderer* renderer, BSFont* font, int codepoint, float fontSize, float spacing) {
    return renderer->measure_codepoint(renderer, font, codepoint, fontSize, spacing);
}
void bsrenderer_draw_codepoint(BSRenderer* renderer, BSFont* font, int codepoint, float x, float y, float fontSize, BSColor color) {
    renderer->draw_codepoint(renderer, font, codepoint, x, y, fontSize, color);
}
void bsrenderer_draw_rectangle(BSRenderer* renderer, float x, float y, float width, float height, const BSFill* fill) {
    renderer->draw_rectangle(renderer, x, y, width, height, fill);
}
void bsrenderer_draw_rectangle_color(BSRenderer* renderer, float x, float y, float width, float height, BSColor color) {
    BSFill fill = {
        .kind = BSFILL_SOLID_COLOR,
        .as.color = color
    };
    renderer->draw_rectangle(renderer, x, y, width, height, &fill);
}
bool bsrenderer_load_font(BSRenderer* renderer, const char* path, BSFont* result) {
    return renderer->load_font(renderer, path, result);
}
void bsrenderer_clear_background(BSRenderer* renderer, const BSFill* fill) {
    renderer->clear_background(renderer, fill);
}
void bsrenderer_clear_background_color(BSRenderer* renderer, BSColor color) {
    BSFill fill = {
        .kind = BSFILL_SOLID_COLOR,
        .as.color = color
    };
    renderer->clear_background(renderer, &fill);
}
