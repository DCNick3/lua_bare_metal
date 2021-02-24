export TOOLCHAIN=i686-elf

export CC=$(TOOLCHAIN)-gcc
export LD=$(TOOLCHAIN)-ld
export AS=$(TOOLCHAIN)-as
export AR=$(TOOLCHAIN)-ar
export OBJCOPY=$(TOOLCHAIN)-objcopy
export MAKE=make

export ARCH=i386
export ARDIR=arch/$(ARCH)

export CFLAGS=-g -O0 -std=gnu99 -ffreestanding -Wall -Wextra --pedantic -D ARCH=$(ARCH) -D __$(ARCH)__ -I$(PWD)/include
export LFLAGS=
export SFLAGS=

LUA_VERSION=5.3.4
LUA_DIR=src/lua-$(LUA_VERSION)
LUA_A=$(LUA_DIR)/src/liblua.a
LIBGCC=`$(CC) -print-libgcc-file-name`

objects=main.o boot.o tty.o peripheral.o memory.o abort.o setjmp.o \
 printf.o stdlib.o ctype.o errno.o string.o time.o locale.o math.o stdio.o \
 lua_files.o

ifeq ($(ARCH), i386)
objects+=vga.o gdt.o gdt_asm.o pic.o irq.o keyboard.o serial.o timer.o floppy.o isa_dma.o
endif

elf_output=main.elf
elf_link_script=main.ld

QEMU-SYSTEM-I386=qemu-system-i386
#QEMU-SYSTEM-I386=/home/dcnick3/git.cloned/qemu/build/i386-softmmu/qemu-system-i386
QEMU_FLAGS=-drive file=fda.img,if=floppy,index=0,format=raw 


$(elf_output): $(objects) $(elf_link_script) $(LUA_A)
	$(LD) -T $(elf_link_script)$(LFLAGS) $(objects) $(LUA_A) $(LIBGCC) -o $@

all: $(elf_output)


%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@



%.o: src/$(ARDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: src/$(ARDIR)/%.s
	$(AS) $(SFLAGS) $< -o $@
	
%.o: src/$(ARDIR)/%.S
	$(CC) $(SFLAGS) -c $< -o $@

$(LUA_A):
	$(MAKE) -C $(LUA_DIR) bare

lua_files.o: src/lua_files.s \
src/lua/main.lua
	$(AS) $(SFLAGS) src/lua_files.s -o $@

clean-lua:
	$(MAKE) -C $(LUA_DIR) clean
	
clean:
	rm -f $(objects) $(elf_output)

qemu: $(elf_output)
	$(QEMU-SYSTEM-I386) -nographic $(QEMU_FLAGS) -kernel $(elf_output) 
	

debug: $(elf_output)
	$(QEMU-SYSTEM-I386) -s -S -nographic $(QEMU_FLAGS) -kernel $(elf_output)

floppy: $(elf_output) boot/grub.cfg
	cp -f $(elf_output) boot/main.elf
	grub-mkimage -p boot/ -O i386-pc -o $@ multiboot fat

