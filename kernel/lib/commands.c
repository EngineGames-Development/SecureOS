#include "include/kernel.h"
#include "include/memory.h"
#include "include/string.h"

extern char input_buffer[];
extern int input_length;
extern void trigger_kernel_panic_gui(const char *error_msg);
extern unsigned int get_free_memory_size(void);
extern void print_float(float num, int decimals);

void process_command() {
  print_string("\n");

  if (strcmp(input_buffer, "clear") == 0) {
    clear_screen();
  } else if (strcmp(input_buffer, "help") == 0) {
    print_string("Commands:\n");
    print_string(" - help     : Show this help menu\n");
    print_string(" - clear    : Clear the terminal window\n");
    print_string(" - sysinfo  : Display system specifications\n");
    print_string(" - malloc   : Test dynamic kernel allocation\n");
    print_string(" - crash    : Provoke a real hardware division-by-zero\n");
    print_string(" - panic    : Trigger a direct manual panic\n");
    print_string(
        " - calc ... : Calculate float chains (e.g. calc 5.5 + 4.5)\n");
    print_string(" - shutdown : Turn off the OS and QEMU\n");
  } else if (strcmp(input_buffer, "sysinfo") == 0) {
    print_string("--- SECUREOS SYSTEM INFO ---\n");
    print_string("OS Name   : SecureOS Next Gen\n");
    print_string("Year      : 2026\n");
    print_string("Free Heap : ");
    print_int(get_free_memory_size());
    print_string(" Bytes\n");
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
  } else if (strcmp(input_buffer, "panic") == 0) {
    trigger_kernel_panic_gui("MANUAL_PANIC_COMMAND_INVOKED");
  } else if (strncmp(input_buffer, "calc ", 5) == 0) {
    char *p = input_buffer + 5;
    float res = 0.0f;
    int has_num = 0;

    while (*p == ' ') {
      p++;
    }

    if ((*p >= '0' && *p <= '9') || *p == '.') {
      float factor = 1.0f;
      int is_decimal = 0;
      while ((*p >= '0' && *p <= '9') || *p == '.') {
        if (*p == '.') {
          is_decimal = 1;
          p++;
          continue;
        }
        if (!is_decimal) {
          res = res * 10.0f + (float)(*p - '0');
        } else {
          factor *= 0.1f;
          res += (float)(*p - '0') * factor;
        }
        p++;
      }
      has_num = 1;
    }

    if (!has_num) {
      print_string("Error: Number expected\n");
    } else {
      int error_occurred = 0;
      while (*p != '\0') {
        while (*p == ' ') {
          p++;
        }
        if (*p == '\0') {
          break;
        }

        char op = *p++;
        if (op != '+' && op != '-' && op != '*' && op != '/') {
          print_string("Error: Invalid operator\n");
          error_occurred = 1;
          break;
        }

        while (*p == ' ') {
          p++;
        }

        float next_num = 0.0f;
        has_num = 0;

        if ((*p >= '0' && *p <= '9') || *p == '.') {
          float factor = 1.0f;
          int is_decimal = 0;
          while ((*p >= '0' && *p <= '9') || *p == '.') {
            if (*p == '.') {
              is_decimal = 1;
              p++;
              continue;
            }
            if (!is_decimal) {
              next_num = next_num * 10.0f + (float)(*p - '0');
            } else {
              factor *= 0.1f;
              next_num += (float)(*p - '0') * factor;
            }
            p++;
          }
          has_num = 1;
        }

        if (!has_num) {
          print_string("Error: Number expected after operator\n");
          error_occurred = 1;
          break;
        }

        if (op == '+')
          res += next_num;
        else if (op == '-')
          res -= next_num;
        else if (op == '*')
          res *= next_num;
        else if (op == '/') {
          if (next_num == 0.0f) {
            print_string("Error: Division by zero\n");
            error_occurred = 1;
            break;
          }
          res /= next_num;
        }
      }

      if (!error_occurred) {
        print_string("Result: ");
        print_float(res, 4);
        print_string("\n");
      }
    }
  } else if (strcmp(input_buffer, "shutdown") == 0) {
    print_string("Shutting down SecureOS...\n");
    sleep_ms(500);
    asm volatile("outw %0, %1"
                 :
                 : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));
    asm volatile("outw %0, %1"
                 :
                 : "a"((unsigned short)0x2000), "Nd"((unsigned short)0xB004));
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
