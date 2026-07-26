#include <efi.h>
#include <efilib.h>

extern void start_graphics_terminal(EFI_SYSTEM_TABLE *SystemTable, unsigned int* fb_addr, unsigned int width, unsigned int height);

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status;

    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if (EFI_ERROR(status)) {
        return status;
    }

    unsigned int* fb_addr = (unsigned int*)gop->Mode->FrameBufferBase;
    unsigned int width = gop->Mode->Info->HorizontalResolution;
    unsigned int height = gop->Mode->Info->VerticalResolution;

    // Sofortiger Sprung in deinen Kernel, ohne zu warten!
    start_graphics_terminal(SystemTable, fb_addr, width, height);

    return EFI_SUCCESS;
}
