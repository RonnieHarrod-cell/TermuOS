CC   = clang
CXX  = clang++
LD   = ld.lld
NASM = nasm

BUILD_DIR = kbuild

V ?= $(VERBOSE)
ifeq ($(V),1)
	Q =
else
	Q = @
endif

# Colours
ifeq ($(shell [ -t 1 ] && echo 1),1)
	C_C = \033[1;32m
	C_CXX = \033[1;36m
	C_ASM = \033[1;33m
	C_LD = \033[1;35m
	C_RESET = \033[0m
endif

quiet_C   = $(Q)printf "  ${C_C}[C]${C_RESET}     %s\n" "$<";
quiet_CXX = $(Q)printf "  ${C_CXX}[C++]${C_RESET}   %s\n" "$<";
quiet_ASM = $(Q)printf "  ${C_ASM}[ASM]${C_RESET}   %s\n" "$<";
quiet_LD  = $(Q)printf "  ${C_LD}[LD]${C_RESET}   %s\n" "$@";

SRCS :=
CPPSRCS :=

CFLAGS = -target x86_64-elf -ffreestanding -fno-stack-protector -fno-pic \
         -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel \
         -O2 -Wall -Wextra -Ikernel -Ilimine

CXXFLAGS = -target x86_64-elf -ffreestanding -fno-stack-protector -fno-pic \
           -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel \
           -O2 -Wall -Wextra -Ikernel -Ilimine \
           -fno-exceptions -fno-rtti -fno-use-cxa-atexit \
           -nostdinc++ -std=c++20

SRCS += \
       kernel/main.c \
       kernel/arch/x86_64/gdt.c \
       kernel/arch/x86_64/idt.c \
       kernel/arch/x86_64/pic.c \
       kernel/arch/x86_64/pit.c \
	   kernel/arch/x86_64/fpu.c \
       kernel/drivers/rtc/rtc.c

SRCS += \
       kernel/mm/pmm.c \
       kernel/mm/vmm.c \
       kernel/mm/heap.c

SRCS += \
       kernel/lib/printf.c \
       kernel/lib/string.c

CPPSRCS += kernel/lib/cxxabi.cpp
CPPSRCS += kernel/luna/widgets/gfx.cpp
CPPSRCS += kernel/luna/widgets/button.cpp
CPPSRCS += kernel/luna/widgets/window.cpp
CPPSRCS += kernel/luna/luna.cpp
CPPSRCS += kernel/luna/focus.cpp
CPPSRCS += kernel/luna/widgets/textfield.cpp
CPPSRCS += kernel/luna/desktop/startmenu/startmenu.cpp
CPPSRCS += kernel/luna/apps/registry.cpp
CPPSRCS += kernel/luna/apps/about.cpp
CPPSRCS += kernel/luna/apps/terminal.cpp
CPPSRCS += kernel/luna/apps/widgets.cpp

SRCS += \
       kernel/drivers/input/keyboard.c \
	   kernel/drivers/input/mouse.c \
       kernel/sched/scheduler.c

SRCS += kernel/proc/process.c

SRCS += kernel/luna/icon.c

SRCS += kernel/ob/object.c

SRCS += kernel/io/ioman.c
SRCS += kernel/drivers/storage/ata_ioman.c
SRCS += kernel/drivers/input/keyboard_ioman.c
SRCS += kernel/ipc/port.c
SRCS += \
       kernel/proc/launch.c \
       kernel/proc/exec.c

SRCS += \
       kernel/drivers/video/fb.c \
       kernel/drivers/video/terminal.c \
       kernel/drivers/serial/serial.c

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
       $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CPPSRCS)) \
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
	$(quiet_C) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp $(CONFIG_HEADER)
	@mkdir -p $(dir $@)
	$(quiet_CXX) $(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(quiet_ASM) $(NASM) -f elf64 $< -o $@

$(KERNEL): $(OBJS)
	$(quiet_LD) $(LD) -T kernel/arch/x86_64/linker.ld -nostdlib -m elf_x86_64 -o $@ $(OBJS)

iso: $(KERNEL)
	@rm -rf iso
	@mkdir -p iso/boot
	@cp $(KERNEL) iso/boot/kernel.elf
	@cp limine/limine-bios.sys iso/boot/
	@cp limine/limine-bios-cd.bin iso/boot/
	@cp limine/limine-uefi-cd.bin iso/boot/
	@cp limine/BOOTX64.EFI iso/boot/
	@cp limine.conf iso/limine.conf
	@mkdir -p iso/boot/icons
	@cp assets/icons/*.rgba iso/boot/icons/ 2>/dev/null || true
	@cp assets/logo.png iso/boot/
	@xorriso -as mkisofs \
		-b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		-graft-points \
		boot/=iso/boot \
		limine.conf=iso/limine.conf \
		-o termuos.iso

run: iso disk.img tools/tfs_write
	@./tools/tfs_write disk.img apps/hello/hello /bin/hello
	@./tools/tfs_write disk.img apps/hello_libc/hello_libc /bin/libc
	@qemu-system-x86_64 -cdrom termuos.iso -cpu qemu64,+syscall \
		-netdev user,id=net0 \
              -device virtio-net-pci,netdev=net0 \
		-drive file=disk.img,format=raw,if=ide \
		-serial stdio

disk.img: tools/mkfs_tfs
	@./tools/mkfs_tfs disk.img 64

tools/mkfs_tfs: tools/mkfs_tfs.c
	@cc -O2 -o tools/mkfs_tfs tools/mkfs_tfs.c

tools/tfs_write:
	@cc -O2 -o tools/tfs_write tools/tfs_write.c

limine:
	git clone https://github.com/limine-bootloader/limine.git \
		--branch=v8.x-binary --depth=1

clean:
	@rm -rf $(BUILD_DIR) $(KERNEL) termuos.iso iso/ disk.img tools/mkfs_tfs tools/tfs_write
