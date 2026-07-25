bits 32

section .multiboot
    align 4
    dd 0x1BADB002
    dd 0x00
    dd - (0x1BADB002 + 0x00)

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
global load_idt

extern kernel_main
extern irq_handler
extern exception_handler

_start:
    mov esp, stack_top
    call kernel_main
    
.halt:
    cli
    hlt
    jmp .halt

load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    jmp 0x08:.flush

.flush:
    ret

global exception0
global exception13
global exception14

exception0:
    push byte 0
    push byte 0
    jmp exception_common_stub

exception13:
    push byte 13 
    jmp exception_common_stub

exception14:
    push byte 14         
    jmp exception_common_stub

exception_common_stub:
    pushad               
    mov ax, ds           
    push eax
    mov ax, 0x10  
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call exception_handler
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popad
    add esp, 8
    iretd

%macro IRQ 2
global irq%1
irq%1:
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 9, 41

irq_common_stub:
    pushad
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call irq_handler
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popad
    add esp, 8
    iretd
