#include "include/kernel.h"
#include "include/string.h"

extern char input_buffer[];
extern int input_length;
extern void clear_screen_gui(unsigned int color);

void process_command() {
  print_string("\n");

  if (strcmp(input_buffer, "clear") == 0) {
    clear_screen();
  } else if (strcmp(input_buffer, "help") == 0) {
    print_string("Befehle:\n");
    print_string(" - help     : Zeigt diese Hilfe\n");
    print_string(" - clear    : Leert den Bildschirm\n");
    print_string(" - panic    : Provoziert CPU-Absturz\n");
    print_string(" - shutdown : Schaltet das OS aus\n");
  } else if (strcmp(input_buffer, "shutdown") == 0) {
    print_string("Fahre SecureOS herunter...\n");
    sleep_ms(500);
    asm volatile("outw %0, %1"
                 :
                 : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));
    asm volatile("outw %0, %1"
                 :
                 : "a"((unsigned short)0x2000), "Nd"((unsigned short)0xB004));
  } else if (strcmp(input_buffer, "panic") == 0) {
    clear_screen_gui(0x00AA0000);
    while (1) {
      asm volatile("hlt");
    }
  } else if (input_length > 0) {
    print_string("Befehl nicht gefunden: ");
    print_string(input_buffer);
    print_string("\n");
  }

  for (int i = 0; i < 64; i++) {
    input_buffer[i] = '\0';
  }
  input_length = 0;
  print_string("> ");
}
