bits 32

section .multiboot
align 8
multiboot_start:
    dd 0xE85250D6               
    dd 0                        
    dd multiboot_end - multiboot_start
    dd - (0xE85250D6 + 0 + (multiboot_end - multiboot_start))

align 8
    dw 5
    dw 0
    dd 20
    dd 1024
    dd 768
    dd 32

align 8
    dw 0
    dw 0
    dd 8
multiboot_end:

section .text
global _start
extern start_graphics_terminal

_start:
    cli
    mov esp, stack_top
    
    push ebx
    call start_graphics_terminal

.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
