CFLAGS="-static -nostdlib -no-pie -ffreestanding \
  -fno-stack-protector -fno-asynchronous-unwind-tables \
  -mno-mmx -mno-sse -mno-sse2 -fcf-protection=none -O0"

gcc $CFLAGS -o apps/hello/hello apps/hello/hello.c