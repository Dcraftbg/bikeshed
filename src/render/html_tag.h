#pragma once
typedef struct BSFont BSFont;
typedef struct BSRenderer BSRenderer;
typedef struct HTMLTag HTMLTag;
void render_html_tag(BSRenderer* renderer, HTMLTag* tag, BSFont* font, float fontSize, float textFontSize, float spacing, float scroll_y);
