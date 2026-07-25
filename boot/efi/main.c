#include <efi.h>
#include <efilib.h>

extern void start_terminal(EFI_SYSTEM_TABLE *SystemTable);

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  start_terminal(SystemTable);
  return EFI_SUCCESS;
}
