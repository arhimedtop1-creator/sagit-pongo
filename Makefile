SHELL := /bin/sh
CC := clang
PYTHON := python3
READELF := readelf

SRC := source/pongo
BUILD := build
PONGO := $(BUILD)/pongo-build/pongo_real
RAMDISK := $(BUILD)/boot/ramdisk-pongo.cpio.gz
BOOT := $(BUILD)/boot/pongo-linux.img
KERNEL := kernel/linux-pongo.gz
GEN := tools/gen_init_cpio
MKBOOT := tools/mkbootimg-sagit/mkbootimg.py

CFLAGS := --target=aarch64-linux-gnu -march=armv8-a -O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie -Wall -Wextra -I$(SRC)/stubs -I$(SRC)/include -I$(SRC)/build
LDFLAGS := --target=aarch64-linux-gnu -fuse-ld=lld -nostdlib -static -no-pie -Wl,-e,_start -Wl,-Ttext=0x400000 -Wl,-z,max-page-size=4096 -Wl,--build-id=none -Wl,--no-undefined

.PHONY: all pc_pongo pc_all termux_pongo termux_all clean
all: pc_all

pc_pongo:
	mkdir -p $(BUILD)/pongo-build
	$(CC) $(CFLAGS) $(SRC)/build/fb_linux.c $(SRC)/stubs/stubs.c $(LDFLAGS) -o $(PONGO)
	@LC_ALL=C $(READELF) -h $(PONGO) | grep -q 'Machine:.*AArch64' || { rm -f $(PONGO); echo "ERROR: not AArch64"; exit 1; }
	@if $(READELF) -l $(PONGO) | grep -q INTERP; then rm -f $(PONGO); echo "ERROR: dynamic ELF"; exit 1; fi
	@echo "ARM64 PONGO built: $(PONGO)"

pc_all: pc_pongo
	@test -f $(KERNEL) || { echo "Missing $(KERNEL)"; exit 1; }
	@test -x $(GEN) || { echo "Missing $(GEN)"; exit 1; }
	@test -f $(MKBOOT) || { echo "Missing $(MKBOOT)"; exit 1; }
	@set -e; tmp=$$(mktemp -d); trap 'rm -rf "$$tmp"' EXIT; \
	{ echo "dir /dev 0755 0 0"; \
	  echo "dir /dev/graphics 0755 0 0"; \
	  echo "dir /proc 0555 0 0"; \
	  echo "dir /sys 0555 0 0"; \
	  echo "dir /run 0755 0 0"; \
	  echo "nod /dev/console 0600 0 0 c 5 1"; \
	  echo "nod /dev/kmsg 0600 0 0 c 1 11"; \
	  echo "nod /dev/graphics/fb0 0666 0 0 c 29 0"; \
	  echo "file /init $(abspath $(PONGO)) 0755 0 0"; } > "$$tmp/nodes"; \
	$(GEN) "$$tmp/nodes" | gzip -n -9 > $(RAMDISK); gzip -t $(RAMDISK)
	$(PYTHON) $(MKBOOT) --kernel $(KERNEL) --ramdisk $(RAMDISK) --base 0x00000000 --kernel_offset 0x00008000 --ramdisk_offset 0x01000000 --tags_offset 0x00000100 --pagesize 4096 --os_version 16.0.0 --os_patch_level 2026-01 --header_version 0 --cmdline "loglevel=8 ignore_loglevel androidboot.console=ttyMSM0 androidboot.hardware=qcom androidboot.boot_devices=soc/1da4000.ufshc msm_rtb.filter=0x37 ehci-hcd.park=3 service_locator.enable=1 swiotlb=2048 loop.max_part=7 panic=0" -o $(BOOT)
	@ls -lh $(PONGO) $(RAMDISK) $(BOOT)

clean:
	rm -rf $(BUILD)/pongo-build $(BUILD)/boot

termux_pongo: pc_pongo

termux_all: pc_all
