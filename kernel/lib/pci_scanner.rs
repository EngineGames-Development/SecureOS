use core::arch::asm;

extern "C" {
    fn printf(format: *const u8, ...);
}

unsafe fn outl(port: u16, value: u32) {
    asm!(
        "out dx, eax",
        in("dx") port,
        in("eax") value,
        options(nomem, nostack, preserves_flags)
    );
}

unsafe fn inl(port: u16) -> u32 {
    let value: u32;
    asm!(
        "in eax, dx",
        out("eax") value,
        in("dx") port,
        options(nomem, nostack, preserves_flags)
    );
    value
}

fn u16_to_hex_str(value: u16, buffer: &mut [u8; 5]) {
    let hex_chars = b"0123456789ABCDEF";
    buffer[0] = hex_chars[((value >> 12) & 0xF) as usize];
    buffer[1] = hex_chars[((value >> 8) & 0xF) as usize];
    buffer[2] = hex_chars[((value >> 4) & 0xF) as usize];
    buffer[3] = hex_chars[(value & 0xF) as usize];
    buffer[4] = 0;
}

#[no_mangle]
pub extern "C" fn scan_pci_bus() {
    let config_address_port: u16 = 0xCF8;
    let config_data_port: u16 = 0xCFC;
    let function: u32 = 0;
    let register_offset: u32 = 0;

    unsafe {
        printf(b"Starting PCI Bus Scan...\n\0".as_ptr());
        printf(b"Bus   | Device  | Vendor ID | Device ID\n\0".as_ptr());
        printf(b"-------------------------------------------\n\0".as_ptr());
    }

    for bus in 0..32 {
        for device in 0..32 {
            let address: u32 = (1 << 31)
                | (bus << 16)
                | (device << 11)
                | (function << 8)
                | (register_offset & 0xFC);

            unsafe {
                outl(config_address_port, address);
                let data: u32 = inl(config_data_port);

                let vendor_id = (data & 0xFFFF) as u16;
                if vendor_id != 0xFFFF {
                    let device_id = ((data >> 16) & 0xFFFF) as u16;

                    let mut vendor_buffer = [0u8; 5];
                    let mut device_buffer = [0u8; 5];

                    u16_to_hex_str(vendor_id, &mut vendor_buffer);
                    u16_to_hex_str(device_id, &mut device_buffer);

                    printf(
                        b"%d     | %d       | 0x%s    | 0x%s\n\0".as_ptr(),
                        bus,
                        device,
                        vendor_buffer.as_ptr(),
                        device_buffer.as_ptr(),
                    );
                }
            }
        }
    }
}
