CC = gcc
AS = nasm
LD = ld
RUSTC = rustc

CFLAGS = -m32 -Wall -Wextra -ffreestanding -O2 -nostdlib -fno-builtin -fno-pie -fno-pic -mno-mmx -mno-sse -mno-sse2 -Ikernel
RUSTFLAGS = --target i686-unknown-linux-gnu -O --crate-type=staticlib -C panic=abort -C relocation-model=static
LDFLAGS = -m elf_i386 -no-pie -T kernel/linker.ld

C_SOURCES = $(wildcard kernel/*.c) $(wildcard kernel/lib/*.c)
ASM_SOURCES = kernel/entry.asm kernel/lib/logo.asm

C_OBJS = $(C_SOURCES:.c=.o)
KERNEL_OBJS = kernel/entry.o kernel/lib/logo.o $(C_OBJS) kernel/lib/librust_kernel.a

all: secureos.iso

kernel/entry.o: kernel/entry.asm
	$(AS) -f elf32 $< -o $@

kernel/lib/logo.o: kernel/lib/logo.asm
	$(AS) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

RUST_SRCS := $(wildcard kernel/lib/*.rs)

kernel/lib/librust_kernel.a: $(RUST_SRCS)
	cd kernel/lib && $(RUSTC) $(RUSTFLAGS) --crate-type=staticlib lib.rs -o ../../$@

kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o $@

secureos.iso: kernel.bin
	mkdir -p iso_root/boot/grub
	cp kernel.bin iso_root/boot/kernel.bin
	echo 'set timeout=5' > iso_root/boot/grub/grub.cfg
	echo 'set default=0' >> iso_root/boot/grub/grub.cfg
	echo 'menuentry "SecureOS (Next-Gen)" {' >> iso_root/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel.bin' >> iso_root/boot/grub/grub.cfg
	echo '    boot' >> iso_root/boot/grub/grub.cfg
	echo '}' >> iso_root/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso_root

clean:
	rm -f kernel/*.o kernel/lib/*.o kernel/lib/*.a kernel.bin secureos.iso
	rm -rf iso_root

run: all
	qemu-img create -f raw fat32_disk.img 64M

	qemu-system-x86_64 \
    -cdrom secureos.iso \
    -drive id=disk,file=fat32_disk.img,if=none,format=raw \
    -device ich9-ahci,id=ahci \
    -device ide-hd,drive=disk,bus=ahci.0 \
    -audiodev driver=sdl,id=speaker \
    -machine pc,pcspk-audiodev=speaker \
    -no-reboot
