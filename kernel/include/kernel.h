#ifndef KERNEL_H
#define KERNEL_H

void draw_pixel(unsigned int x, unsigned int y, unsigned int color);
void draw_rect(unsigned int start_x, unsigned int start_y, unsigned int width,
               unsigned int height, unsigned int color);
void clear_screen(void);
void print_char(char c);
void print_string(const char *str);
void sleep_ms(int milliseconds);

extern char input_buffer[];
extern int input_length;

#endif
