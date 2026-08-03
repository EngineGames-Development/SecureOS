#![no_std]

use core::panic::PanicInfo;

extern "C" {
    fn print_string(s: *const u8);
    fn trigger_kernel_panic_gui(msg: *const u8);
}

#[no_mangle]
pub extern "C" fn rust_init() {
    unsafe {
        print_string(b"RUST CORE INITIALIZED SUCCESSFULLY.\n\0".as_ptr());
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    unsafe {
        trigger_kernel_panic_gui(b"RUST_RUNTIME_PANIC\0".as_ptr());
    }
    loop {}
}
