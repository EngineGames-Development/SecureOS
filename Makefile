CC      := gcc
AS      := nasm
LD      := ld
RUSTC   := rustc

CFLAGS  := -m32 -Wall -Wextra -ffreestanding -O2 -nostdlib \
           -fno-builtin -fno-pie -fno-pic -mno-mmx -mno-sse -mno-sse2 -Ikernel

RUSTFLAGS := --target i686-unknown-linux-gnu -O --crate-type=staticlib \
             -C panic=abort -C relocation-model=static

LDFLAGS := -m elf_i386 -no-pie -T kernel/linker.ld

C_SOURCES   := $(wildcard kernel/*.c) $(wildcard kernel/lib/*.c)
ASM_SOURCES := kernel/entry.asm kernel/lib/logo.asm
RUST_SRCS   := $(wildcard kernel/lib/*.rs)

OBJS := $(ASM_SOURCES:.asm=.o) $(C_SOURCES:.c=.o)
LIBS := kernel/lib/librust_kernel.a

.PHONY: all clean run

all: secureos.iso

%.o: %.asm
	$(AS) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(RUST_SRCS)
	$(RUSTC) $(RUSTFLAGS) kernel/lib/lib.rs -o $@

kernel.bin: $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

secureos.iso: kernel.bin
	@mkdir -p iso_root/boot/grub
	cp kernel.bin iso_root/boot/kernel.bin
	@echo 'set timeout=0' > iso_root/boot/grub/grub.cfg
	@echo 'set default=0' >> iso_root/boot/grub/grub.cfg
	@echo 'menuentry "SecureOS" {' >> iso_root/boot/grub/grub.cfg
	@echo '  multiboot2 /boot/kernel.bin' >> iso_root/boot/grub/grub.cfg
	@echo '  boot' >> iso_root/boot/grub/grub.cfg
	@echo '}' >> iso_root/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso_root

clean:
	rm -f kernel/*.o kernel/lib/*.o kernel/lib/*.a kernel.bin secureos.iso
	rm -rf iso_root fat32_disk.img

run: all
	qemu-img create -f raw fat32_disk.img 64M
	mkfs.vfat fat32_disk.img
	qemu-system-i386 \
		-boot d \
		-cdrom secureos.iso \
		-drive id=disk,file=fat32_disk.img,if=none,format=raw \
		-device ich9-ahci,id=ahci \
		-device ide-hd,drive=disk,bus=ahci.0 \
		-audiodev driver=sdl,id=speaker \
		-machine pc,pcspk-audiodev=speaker \
		-no-reboot
