CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -Wall -Wextra -ffreestanding -O2 -nostdlib -nostdinc -fno-builtin -fno-pie -fno-pic -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -Ikernel
LDFLAGS = -m elf_i386 -no-pie -T boot/linker.ld

KERNEL_SOURCES = $(wildcard kernel/*.c) $(wildcard kernel/**/*.c)
KERNEL_OBJS = boot/boot.o $(KERNEL_SOURCES:.c=.o)

all: sysroot/boot/kernel.bin

boot/boot.o: boot/boot.asm
	$(AS) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

sysroot/boot/kernel.bin: $(KERNEL_OBJS)
	mkdir -p sysroot/boot
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o $@

clean:
	rm -f boot/*.o
	find kernel -name "*.o" -type f -delete
	rm -f sysroot/boot/kernel.bin

run: all
	qemu-system-i386 -M q35 -kernel sysroot/boot/kernel.bin -no-reboot
