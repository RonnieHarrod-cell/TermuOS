CFLAGS="-static -nostdlib -no-pie -ffreestanding \
  -fno-stack-protector -fno-asynchronous-unwind-tables \
  -mno-mmx -mno-sse -mno-sse2 -fcf-protection=none -O0 \
  -I libc/include"

mkdir -p libc/build apps/hello_libc

# libc objects
gcc $CFLAGS -c libc/src/syscall.c -o libc/build/syscall.o
gcc $CFLAGS -c libc/src/write.c   -o libc/build/write.o
gcc $CFLAGS -c libc/src/exit.c    -o libc/build/exit.o
gcc $CFLAGS -c libc/crt0.S        -o libc/build/crt0.o

# app
gcc $CFLAGS -c apps/hello_libc/main.c -o apps/hello_libc/main.o

# link: crt0 first, then main, then libc bits
ld -static -nostdlib -o apps/hello_libc/hello_libc \
  libc/build/crt0.o apps/hello_libc/main.o \
  libc/build/syscall.o libc/build/write.o libc/build/exit.o
