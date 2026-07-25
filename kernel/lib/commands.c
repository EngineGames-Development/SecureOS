#include "include/io.h"
#include "include/kernel.h"
#include "include/string.h"

extern char input_buffer[];
extern int input_length;

void process_command() {
  print_string("\n");

  if (strcmp(input_buffer, "clear") == 0) {
    clear_screen();
  } else if (strcmp(input_buffer, "help") == 0) {
    print_string("Befehle:\n");
    print_string(" - help     : Zeigt diese Hilfe\n");
    print_string(" - clear    : Leert den Bildschirm\n");
    print_string(" - shutdown : Schaltet das OS und QEMU aus\n");
    print_string(" - panic    : Provoziert einen CPU-Absturz\n");
  } else if (strcmp(input_buffer, "shutdown") == 0) {
    print_string("Fahre SecureOS herunter...\n");
    sleep_ms(3000);
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x604, 0x3400);
    asm volatile("cli; hlt");
  } else if (strcmp(input_buffer, "panic") == 0) {
    int volatile a = 5;
    int volatile b = 0;
    int volatile c = a / b;
    (void)c;
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
