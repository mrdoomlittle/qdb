small script to count the digits in a int. NOTE: this might be a inefficient way of doing this.

make build ARC=-DARC64 R_ARC=ARC64 RUST_LIBS=true # 64 uint
or
make ARC64 RUST_LIBS=true

make build ARC=-DARC32 R_ARC=ARC64 RUST_LIBS=true # 32 uint
or
make ARC32 RUST_LIBS=true
