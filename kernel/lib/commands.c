#include "include/kernel.h"
#include "include/memory.h"
#include "include/string.h"

extern char input_buffer[];
extern int input_length;

void process_command() {
  print_string("\n");

  if (strcmp(input_buffer, "clear") == 0) {
    clear_screen();
  } else if (strcmp(input_buffer, "help") == 0) {
    print_string("Commands:\n");
    print_string(" - help     : Show this help menu\n");
    print_string(" - clear    : Clear the terminal window\n");
    print_string(" - malloc   : Test dynamic kernel allocation\n");
    print_string(" - crash    : Provoke a real hardware division-by-zero\n");
    print_string(" - panic    : Trigger a direct manual panic\n");
    print_string(" - shutdown : Turn off the OS and QEMU\n");
  } else if (strcmp(input_buffer, "malloc") == 0) {
    void *ptr = kmalloc(1024);
    if (ptr != 0) {
      print_string("Memory successfully allocated at heap!\n");
      kfree(ptr);
      print_string("Memory successfully freed!\n");
    } else {
      print_string("Allocation failed!\n");
    }
  } else if (strcmp(input_buffer, "crash") == 0) {
    print_string("Provoking hardware exception...\n");
    sleep_ms(300);
    volatile int a = 5;
    volatile int b = 0;
    volatile int c = a / b;
    (void)c;
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
    trigger_kernel_panic_gui("MANUAL_PANIC_COMMAND_INVOKED");
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
