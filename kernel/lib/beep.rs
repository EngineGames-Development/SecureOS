use core::arch::asm;

#[inline(always)]
pub unsafe fn inb(port: u16) -> u8 {
    let value: u8;
    asm!(
        "in al, dx",
        in("dx") port,
        out("al") value,
        options(nomem, nostack, preserves_flags)
    );
    value
}

#[inline(always)]
pub unsafe fn outb(port: u16, val: u8) {
    asm!(
        "out dx, al",
        in("dx") port,
        in("al") val,
        options(nomem, nostack, preserves_flags)
    );
}

extern "C" {
    fn sleep_ms(duration: i32);
}

#[no_mangle]
pub extern "C" fn play_beep(frequency: i32, duration: i32) {
    let divisor = 1193180 / (frequency as u32);

    unsafe {
        outb(0x43, 0xB6);

        outb(0x42, (divisor & 0xFF) as u8);
        outb(0x42, ((divisor >> 8) & 0xFF) as u8);

        let mut tmp = inb(0x61);
        outb(0x61, tmp | 3);

        sleep_ms(duration);

        tmp = inb(0x61);
        outb(0x61, tmp & !3);
    }
}
