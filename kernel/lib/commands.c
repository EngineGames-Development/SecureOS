#include "include/kernel.h"
#include "include/string.h"

extern char input_buffer[];
extern int input_length;
extern void trigger_kernel_panic_gui(const char *error_msg);

void process_command() {
  print_string("\n");

  if (strcmp(input_buffer, "clear") == 0) {
    clear_screen();
  } else if (strcmp(input_buffer, "help") == 0) {
    print_string("Commands:\n");
    print_string(" - help     : Show this help menu\n");
    print_string(" - clear    : Clear the terminal window\n");
    print_string(" - panic    : Provoke a system crash\n");
    print_string(" - shutdown : Turn off the OS and QEMU\n");
  } else if (strcmp(input_buffer, "shutdown") == 0) {
    print_string("Shutting down SecureOS...\n");
    sleep_ms(500);
    asm volatile("outw %0, %1"
                 :
                 : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));
    asm volatile("outw %0, %1"
                 :
                 : "a"((unsigned short)0x2000), "Nd"((unsigned short)0xB004));
  } else if (strcmp(input_buffer, "panic") == 0) {
    trigger_kernel_panic_gui("USER-TRIGGERED PANIC COMMAND");
  } else if (input_length > 0) {
    print_string("Command not found: ");
    print_string(input_buffer);
    print_string("\n");
  }

  for (int i = 0; i < 64; i++) {
    input_buffer[i] = '\0';
  }
  input_length = 0;
  print_string("> ");
}
