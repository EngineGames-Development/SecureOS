#include "include/font.h"
#include <efi.h>
#include <efilib.h>

unsigned int *framebuffer = NULL;
unsigned int screen_width = 0;
unsigned int screen_height = 0;

char input_buffer[64];
int input_length = 0;

extern int strcmp(const char *str1, const char *str2);
extern void process_command(void);

void draw_pixel(unsigned int x, unsigned int y, unsigned int color) {
  if (x >= screen_width || y >= screen_height || framebuffer == NULL)
    return;
  framebuffer[y * screen_width + x] = color;
}

void draw_rect(unsigned int start_x, unsigned int start_y, unsigned int width,
               unsigned int height, unsigned int color) {
  for (unsigned int y = start_y; y < start_y + height; y++) {
    for (unsigned int x = start_x; x < start_x + width; x++) {
      draw_pixel(x, y, color);
    }
  }
}

void clear_screen_gui(unsigned int color) {
  for (unsigned int i = 0; i < screen_width * screen_height; i++) {
    framebuffer[i] = color;
  }
}

void draw_desktop_and_taskbar(void) {
  clear_screen_gui(0x001A1A24);

  unsigned int taskbar_height = 40;
  unsigned int taskbar_y = screen_height - taskbar_height;
  draw_rect(0, taskbar_y, screen_width, taskbar_height, 0x00101014);

  unsigned int button_width = 60;
  unsigned int button_height = 28;
  unsigned int button_x = 6;
  unsigned int button_y = taskbar_y + ((taskbar_height - button_height) / 2);
  draw_rect(button_x, button_y, button_width, button_height, 0x00005B9E);

  draw_string(button_x + 10, button_y + 6, "START", 0x00FFFFFF);
  draw_string(20, 20, "SECUREOS NEXT GEN", 0x00005B9E);
}

void start_graphics_terminal(EFI_SYSTEM_TABLE *SystemTable,
                             unsigned int *fb_addr, unsigned int width,
                             unsigned int height) {
  ST = SystemTable;
  framebuffer = (unsigned int *)fb_addr;
  screen_width = width;
  screen_height = height;

  draw_desktop_and_taskbar();

  EFI_INPUT_KEY Key;
  while (1) {
    EFI_STATUS status =
        uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key);
    if (status == EFI_SUCCESS) {
      if (Key.ScanCode == 0) {
        char c = (char)Key.UnicodeChar;
        if (c == '\r' || c == '\n') {
          input_buffer[input_length] = '\0';
          process_command();
        } else if (c == '\b') {
          if (input_length > 0) {
            input_length--;
            input_buffer[input_length] = '\0';
          }
        } else if (input_length < 63) {
          input_buffer[input_length] = c;
          input_length++;
        }
      }
    }
  }
}
