#[link(name="getdigit", kind="static")]
#[cfg(ARC64)] pub type uint_t = u64;
#[cfg(ARC32)] pub type uint_t = u32;
#[cfg(not(any(ARC64, ARC32)))]
pub type uint_t = u32;

extern "C" {
    pub fn getdigit(__uint: uint_t, __unit: usize) -> u8;
}
