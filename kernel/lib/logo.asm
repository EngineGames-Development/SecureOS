section .rodata
global logo_bmp
global logo_bmp_len

logo_bmp:
    incbin "logo.bmp"
logo_end:

logo_bmp_len:
    dd logo_end - logo_bmp
