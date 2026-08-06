#ifndef FONT_H
#define FONT_H

void draw_char(int x, int y, char c, unsigned int color);
void draw_char_opaque(int x, int y, char c, unsigned int color);
void draw_string(int x, int y, const char *str, unsigned int color);

#endif
