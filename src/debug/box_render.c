#include <debug/box_render.h>
#include <bsrenderer/color.h>
#include <bsrenderer/renderer.h>
#include <bsrenderer/fill.h>
#include <html.h>
#include <stdio.h>

size_t render_box_color_n = 0;
void render_box_html_tag(BSRenderer* renderer, HTMLTag* tag, float scroll_y) {
    static BSColor colors[] = {
        0x828282ff, // GRAY
        0xffcb00ff, // GOLD
        0xff6dc2ff, // PINK
        0x66bfffff, // SKYBLUE
        0xe62937ff, // RED
        0xbe2137ff, // MAROON
        0x00e430ff, // GREEN
        0xffa100ff, // ORANGE
        0x009e2fff, // LIME
        0x00752cff, // DARKGREEN
        0x0079f1ff, // BLUE
        0x0052acff, // DARKBLUE
        0xc87affff, // PURPLE
        0x873cbeff, // VIOLET
        0x701f7eff, // DARKPURPLE
        0xd3b083ff, // BEIGE
        0x7f6a4fff, // BROWN
        0x4c3f2fff, // DARKBROWN
        0xffffffff, // WHITE
        0x00000000, // BLANK
        0xff00ffff, // MAGENTA
        0xf5f5f5ff, // RAYWHITE
    };
    BSFill fill = {
        .kind = BSFILL_SOLID_COLOR,
        .as.color = colors[render_box_color_n++]
    };
    bsrenderer_draw_rectangle(renderer, tag->x, tag->y + scroll_y, tag->width, tag->height, &fill);
    for(size_t i = 0; i < tag->children.len; ++i) {
        render_box_html_tag(renderer, tag->children.items[i], scroll_y);
    }
}
