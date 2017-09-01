#[link(name="getdigit", kind="static")]
#[cfg(ARC64)] type uint_t = u64;
#[cfg(ARC32)] type uint_t = u32;

#[cfg(not(any(ARC64, ARC32)))]
type uint_t = u32;

extern "C" {
    pub fn intlen(__uint: uint_t) -> usize;
}
