#ifndef KERNEL_H
#define KERNEL_H

void clear_screen(void);
void print_string(const char *str);
void print_char(char c);
void sleep_ms(int milliseconds);
void update_hardware_cursor(void);
void process_command(void);

extern char input_buffer[];
extern int input_length;
extern int cursor_x;
extern int cursor_y;
extern volatile unsigned short *video_memory;

#endif
