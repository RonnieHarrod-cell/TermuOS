CC   = clang
LD   = ld.lld
NASM = nasm

BUILD_DIR = kbuild

SRCS :=
CPPSRCS :=

CFLAGS = -target x86_64-elf -ffreestanding -fno-stack-protector -fno-pic \
         -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel \
         -O2 -Wall -Wextra -Ikernel -Ilimine

SRCS += \
       kernel/main.c \
       kernel/arch/x86_64/gdt.c \
       kernel/arch/x86_64/idt.c \
       kernel/arch/x86_64/pic.c \
       kernel/arch/x86_64/pit.c \
       kernel/drivers/rtc/rtc.c

SRCS += \
       kernel/mm/pmm.c \
       kernel/mm/vmm.c \
       kernel/mm/heap.c

SRCS += \
       kernel/lib/printf.c \
       kernel/lib/string.c

SRCS += \
       kernel/drivers/input/keyboard.c \
       kernel/sched/scheduler.c

SRCS += kernel/proc/process.c

SRCS += kernel/ob/object.c

SRCS += kernel/io/ioman.c
SRCS += kernel/drivers/storage/ata_ioman.c
SRCS += kernel/drivers/input/keyboard_ioman.c
SRCS += kernel/ipc/port.c
SRCS += kernel/tlib/tlib_bundle.c
SRCS += \
       kernel/tlib/tlib_launch.c \
       kernel/tlib/exec.c

SRCS += \
       kernel/drivers/video/fb.c \
       kernel/drivers/video/terminal.c

SRCS += \
       kernel/drivers/net/pci.c \
       kernel/drivers/net/virtio_net.c \
       kernel/net/net.c

SRCS += \
       kernel/drivers/storage/ata.c \
       kernel/drivers/storage/disk.c

SRCS += \
       kernel/fs/vfs.c \
       kernel/fs/ramfs.c \
       kernel/fs/tfs.c

SRCS += kernel/shell/shell.c

SRCS += \
       kernel/user/syscall.c \
       kernel/user/userspace.c

OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS)) \
       $(BUILD_DIR)/kernel/arch/x86_64/entry.o \
       $(BUILD_DIR)/kernel/arch/x86_64/gdt_asm.o \
       $(BUILD_DIR)/kernel/arch/x86_64/isr.o \
       $(BUILD_DIR)/kernel/sched/context_switch.o

OBJS += \
       $(BUILD_DIR)/kernel/user/syscall_asm.o \
       $(BUILD_DIR)/kernel/user/userspace_asm.o

KERNEL = kernel.elf

all: iso

$(BUILD_DIR)/%.o: %.c $(CONFIG_HEADER)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@


$(KERNEL): $(OBJS)
	$(LD) -T kernel/arch/x86_64/linker.ld -nostdlib -m elf_x86_64 -o $@ $(OBJS)

iso: $(KERNEL)
	rm -rf iso
	mkdir -p iso/boot
	cp $(KERNEL) iso/boot/kernel.elf
	cp limine/limine-bios.sys iso/boot/
	cp limine/limine-bios-cd.bin iso/boot/
	cp limine/limine-uefi-cd.bin iso/boot/
	cp limine/BOOTX64.EFI iso/boot/
	cp limine.conf iso/limine.conf
	xorriso -as mkisofs \
		-b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		-graft-points \
		boot/=iso/boot \
		limine.conf=iso/limine.conf \
		-o termuos.iso

run: iso disk.img tools/tfs_write
	./tools/tfs_write disk.img apps/HelloGui /mnt/HelloGui.tapp
	qemu-system-x86_64 -cdrom termuos.iso -cpu qemu64,+syscall \
		-netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
		-drive file=disk.img,format=raw,if=ide \
		-serial stdio

disk.img: tools/mkfs_tfs
	./tools/mkfs_tfs disk.img 64

tools/mkfs_tfs: tools/mkfs_tfs.c
	cc -O2 -o tools/mkfs_tfs tools/mkfs_tfs.c

tools/tfs_write:
	cc -O2 -o tools/tfs_write tools/tfs_write.c

limine:
	git clone https://github.com/limine-bootloader/limine.git \
		--branch=v8.x-binary --depth=1

clean:
	rm -rf $(BUILD_DIR) $(KERNEL) termuos.iso iso/ disk.img tools/mkfs_tfs tools/tfs_write
