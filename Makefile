ARCH            = x86_64
EFIINC          = /usr/include/efi
EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol -Ikernel
EFILIB          = /usr/lib
EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = $(EFILIB)/elf_$(ARCH)_efi.lds

CFLAGS          = -fno-stack-protector -fpic -fshort-wchar -mno-red-zone \
                  -Wall -Wextra -DEFI_FUNCTION_WRAPPER $(EFIINCS) -O2

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
                  -Bsymbolic $(EFI_CRT_OBJS) -L$(EFILIB) -lefi -lgnuefi

KERNEL_SOURCES  = $(wildcard kernel/*.c) $(wildcard kernel/**/*.c)
KERNEL_OBJS     = $(KERNEL_SOURCES:.c=.o)

all: secureos.iso

boot/efi/main.o: boot/efi/main.c
	gcc $(CFLAGS) -c $< -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

sysroot/EFI/BOOT/BOOTX64.EFI: boot/efi/main.o $(KERNEL_OBJS)
	mkdir -p sysroot/EFI/BOOT
	ld boot/efi/main.o $(KERNEL_OBJS) $(LDFLAGS) -o boot/efi/main.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel \
		-j .rela -j .reloc --target=efi-app-$(ARCH) boot/efi/main.so $@
	rm boot/efi/main.so

secureos.img: sysroot/EFI/BOOT/BOOTX64.EFI
	dd if=/dev/zero of=$@ bs=1M count=48
	mkfs.vfat -F 32 $@
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ sysroot/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

secureos.iso: secureos.img
	mkdir -p iso_root
	xorriso -as mkisofs -R -f -e secureos.img -no-emul-boot -o $@ iso_root
	rm -rf iso_root

clean:
	rm -f boot/efi/*.o kernel/*.o kernel/lib/*.o sysroot/EFI/BOOT/BOOTX64.EFI secureos.img secureos.iso
	rm -rf iso_root

run: all
	qemu-system-x86_64 -bios secure_bios.fd -net none -drive file=secureos.img,format=raw -no-reboot
