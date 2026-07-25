#include <efi.h>
#include <efilib.h>

char input_buffer[64];
int input_length = 0;

extern int strcmp(const char *str1, const char *str2);
extern void process_command(void);

void clear_screen(void) {
  if (ST)
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
}

void print_char(char c) {
  if (!ST)
    return;
  CHAR16 wstr[2] = {0};
  if (c == '\n') {
    wstr[0] = L'\r';
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, wstr);
    wstr[0] = L'\n';
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, wstr);
  } else {
    wstr[0] = (CHAR16)c;
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, wstr);
  }
}

void print_string(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    print_char(str[i]);
  }
}

void sleep_ms(int milliseconds) {
  if (ST)
    uefi_call_wrapper(ST->BootServices->Stall, 1, milliseconds * 1000);
}

void start_terminal(EFI_SYSTEM_TABLE *SystemTable) {
  ST = SystemTable;
  clear_screen();

  uefi_call_wrapper(ST->ConOut->EnableCursor, 2, ST->ConOut, TRUE);

  print_string("\n\n\n");
  print_string("                     ####################################\n");
  print_string("                     #       S E C U R E   O S          #\n");
  print_string("                     #         - NEXT GEN -             #\n");
  print_string("                     ####################################\n\n");
  print_string("                              Booting Kernel...          \n\n");
  print_string(
      "                     [                                    ] 0%");

  sleep_ms(100);

  for (int i = 1; i <= 10; i++) {
    sleep_ms(100);
    print_string("===");
    if (i == 10)
      print_string(" 100%");
  }

  sleep_ms(200);
  clear_screen();

  print_string("SecureOS Terminal v0.7 (Modern 64-Bit UEFI)\n");
  print_string("Gib 'help' oder 'clear' ein.\n\n> ");

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
            CHAR16 bs[4] = {L'\b', L' ', L'\b', 0};
            uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, bs);
          }
        } else if (input_length < 63) {
          print_char(c);
          input_buffer[input_length] = c;
          input_length++;
        }
      }
    }
  }
}
