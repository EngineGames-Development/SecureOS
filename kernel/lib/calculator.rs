use core::ffi::c_char;
use core::ptr;

#[no_mangle]
pub extern "C" fn calc(input: *const c_char, output: *mut c_char, output_size: u32) -> i32 {
    if input.is_null() || output.is_null() || output_size == 0 {
        return -1;
    }

    let mut parser = Parser {
        ptr: input as *const u8,
    };

    if !parser.skip_calc() {
        return write(output, output_size, b"ERR CMD");
    }

    let mut result = match parser.number() {
        Some(v) => v,
        None => return write(output, output_size, b"ERR NUM"),
    };

    loop {
        parser.skip_space();

        let op = match parser.next() {
            Some(v) => v,
            None => break,
        };

        let value = match parser.number() {
            Some(v) => v,
            None => return write(output, output_size, b"ERR NUM"),
        };

        match op {
            b'+' => result += value,
            b'-' => result -= value,
            b'*' => result *= value,
            b'/' => {
                if value == 0.0 {
                    return write(output, output_size, b"ERR DIV");
                }
                result /= value;
            }
            _ => return write(output, output_size, b"ERR OP"),
        }
    }

    write_number(output, output_size, result)
}

struct Parser {
    ptr: *const u8,
}

impl Parser {
    fn next(&mut self) -> Option<u8> {
        unsafe {
            let c = *self.ptr;

            if c == 0 {
                None
            } else {
                self.ptr = self.ptr.add(1);
                Some(c)
            }
        }
    }

    fn skip_space(&mut self) {
        unsafe {
            while *self.ptr == b' ' {
                self.ptr = self.ptr.add(1);
            }
        }
    }

    fn skip_calc(&mut self) -> bool {
        let cmd = [b'c', b'a', b'l', b'c', b' '];

        for c in cmd {
            if self.next() != Some(c) {
                return false;
            }
        }

        true
    }

    fn number(&mut self) -> Option<f64> {
        self.skip_space();

        let mut value = 0.0;
        let mut divisor = 1.0;
        let mut decimal = false;
        let mut negative = false;
        let mut found = false;

        unsafe {
            if *self.ptr == b'-' {
                negative = true;
                self.ptr = self.ptr.add(1);
            }

            while (*self.ptr >= b'0' && *self.ptr <= b'9') || *self.ptr == b'.' {
                let c = *self.ptr;

                if c == b'.' {
                    if decimal {
                        break;
                    }

                    decimal = true;
                } else {
                    found = true;

                    let digit = (c - b'0') as f64;

                    if decimal {
                        divisor *= 10.0;
                        value += digit / divisor;
                    } else {
                        value = value * 10.0 + digit;
                    }
                }

                self.ptr = self.ptr.add(1);
            }
        }

        if found {
            if negative {
                Some(-value)
            } else {
                Some(value)
            }
        } else {
            None
        }
    }
}

fn write(output: *mut c_char, size: u32, text: &[u8]) -> i32 {
    if text.len() as u32 + 1 > size {
        return -1;
    }

    unsafe {
        ptr::copy_nonoverlapping(text.as_ptr(), output as *mut u8, text.len());
        *output.add(text.len()) = 0;
    }

    -1
}

fn write_number(output: *mut c_char, size: u32, value: f64) -> i32 {
    let mut buf = [0u8; 64];
    let len = float_to_str(value, &mut buf);

    if len as u32 + 1 > size {
        return -1;
    }

    unsafe {
        ptr::copy_nonoverlapping(buf.as_ptr(), output as *mut u8, len);
        *output.add(len) = 0;
    }

    0
}

fn float_to_str(mut value: f64, buf: &mut [u8; 64]) -> usize {
    let mut len = 0;

    if value < 0.0 {
        buf[len] = b'-';
        len += 1;
        value = -value;
    }

    let integer = value as u64;

    len += uint_to_str(integer, &mut buf[len..]);

    let fractional = ((value - integer as f64) * 1_000_000.0) as u64;

    if fractional != 0 {
        buf[len] = b'.';
        len += 1;

        let mut div = 100000;

        while div > 0 {
            buf[len] = b'0' + ((fractional / div) % 10) as u8;
            len += 1;
            div /= 10;
        }

        while len > 0 && buf[len - 1] == b'0' {
            len -= 1;
        }
    }

    len
}

fn uint_to_str(mut value: u64, buf: &mut [u8]) -> usize {
    if value == 0 {
        buf[0] = b'0';
        return 1;
    }

    let mut tmp = [0u8; 20];
    let mut len = 0;

    while value > 0 {
        tmp[len] = b'0' + (value % 10) as u8;
        value /= 10;
        len += 1;
    }

    for i in 0..len {
        buf[i] = tmp[len - i - 1];
    }

    len
}
