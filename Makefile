CC = gcc
AS = nasm
LD = ld


CFLAGS = -m32 -Wall -Wextra -ffreestanding -O2 -nostdlib -fno-builtin -fno-pie -fno-pic -mno-mmx -mno-sse -mno-sse2 -Ikernel

CFLAGS = -m32 -Wall -Wextra -ffreestanding -O2 -nostdlib -fno-builtin -fno-pie -fno-pic -mno-mmx -mno-sse -mno-sse2 -Ikernel

LDFLAGS = -m elf_i386 -no-pie -T kernel/linker.ld

KERNEL_SOURCES = $(wildcard kernel/*.c) $(wildcard kernel/lib/*.c)
ASM_SOURCES = kernel/entry.asm kernel/lib/logo.asm

KERNEL_OBJS = kernel/entry.o kernel/lib/logo.o $(KERNEL_SOURCES:.c=.o)

all: secureos.iso

kernel/entry.o: kernel/entry.asm
	$(AS) -f elf32 $< -o $@

kernel/lib/logo.o: kernel/lib/logo.asm
	$(AS) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

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
	rm -f kernel/*.o kernel/lib/*.o kernel.bin secureos.iso
	rm -rf iso_root

run: all
	qemu-system-x86_64 -cdrom secureos.iso -no-reboot
