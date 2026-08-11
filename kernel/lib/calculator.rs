use core::ffi::c_char;
use core::slice;

#[no_mangle]
pub extern "C" fn calc(input: *const c_char, output: *mut c_char, output_size: u32) -> i32 {
    if input.is_null() || output.is_null() || output_size < 12 {
        return -1;
    }

    let mut len = 0;
    unsafe {
        while *input.add(len) != 0 {
            len += 1;
            if len >= 1024 {
                return raw_write(output, output_size, b"ERR TOOLONG");
            }
        }
    }

    let s = unsafe { slice::from_raw_parts(input as *const u8, len) };

    let mut i = 0;
    while i < s.len() && s[i] == b' ' {
        i += 1;
    }
    if !s[i..].starts_with(b"calc") {
        return raw_write(output, output_size, b"ERR CMD");
    }
    i += 4;

    let mut total = 0.0;
    let mut term = match parse_f64(s, &mut i) {
        Some(v) => v,
        None => return raw_write(output, output_size, b"ERR NUM"),
    };

    loop {
        while i < s.len() && s[i] == b' ' {
            i += 1;
        }
        if i >= s.len() {
            break;
        }

        let op = s[i];
        i += 1;

        let next_val = match parse_f64(s, &mut i) {
            Some(v) => v,
            None => return raw_write(output, output_size, b"ERR NUM"),
        };

        if op == b'*' {
            term *= next_val;
        } else if op == b'/' {
            if next_val == 0.0 {
                return raw_write(output, output_size, b"ERR DIV");
            }
            term /= next_val;
        } else if op == b'+' || op == b'-' {
            total += term;
            term = if op == b'+' { next_val } else { -next_val };
        } else {
            return raw_write(output, output_size, b"ERR OP");
        }
    }
    total += term;

    let mut buf = [0u8; 24];
    let mut p = 0;
    let mut val = total;
    if val < 0.0 {
        buf[p] = b'-';
        p += 1;
        val = -val;
    }
    let int_part = val as u64;
    let mut temp = int_part;
    let start = p;
    if temp == 0 {
        buf[p] = b'0';
        p += 1;
    } else {
        while temp > 0 {
            buf[p] = b'0' + (temp % 10) as u8;
            temp /= 10;
            p += 1;
        }
        buf[start..p].reverse();
    }
    let frac = ((val - int_part as f64) * 100.0 + 0.5) as u64;
    if frac > 0 && frac < 100 {
        buf[p] = b'.';
        p += 1;
        buf[p] = b'0' + (frac / 10) as u8;
        buf[p + 1] = b'0' + (frac % 10) as u8;
        p += 2;
        if buf[p - 1] == b'0' {
            p -= 1;
        }
    }

    raw_write(output, output_size, &buf[..p])
}

fn parse_f64(s: &[u8], i: &mut usize) -> Option<f64> {
    while *i < s.len() && s[*i] == b' ' {
        *i += 1;
    }
    if *i >= s.len() {
        return None;
    }

    let neg = if s[*i] == b'-' {
        *i += 1;
        true
    } else {
        false
    };
    let mut val = 0.0;
    let mut has_digits = false;

    while *i < s.len() && s[*i].is_ascii_digit() {
        has_digits = true;
        val = val * 10.0 + (s[*i] - b'0') as f64;
        *i += 1;
    }
    if *i < s.len() && s[*i] == b'.' {
        *i += 1;
        let mut div = 10.0;
        while *i < s.len() && s[*i].is_ascii_digit() {
            has_digits = true;
            val += (s[*i] - b'0') as f64 / div;
            div *= 10.0;
            *i += 1;
        }
    }

    if !has_digits {
        return None;
    }
    Some(if neg { -val } else { val })
}

fn raw_write(out: *mut c_char, size: u32, src: &[u8]) -> i32 {
    if src.len() >= size as usize {
        return -1;
    }
    unsafe {
        core::ptr::copy_nonoverlapping(src.as_ptr(), out as *mut u8, src.len());
        *out.add(src.len()) = 0;
    }
    if src.starts_with(b"ERR") {
        -1
    } else {
        0
    }
}
