all: prepare rootfs bsp public firmware

prepare:
	make -C toolchain
	mkdir -p $(DESTDIR)

bsp:
	make -C bsp

public:
	make -C public

rootfs:
	make -C rootfs
	cp -a toolchain/sysroot/lib/*.so* $(ROOTDIR)/lib

firmware:
	mkdir -p build/firmware
	cp bsp/uEnv.txt build/firmware
	cp build/uboot/SPL build/firmware
	cp build/uboot/u-boot.img build/firmware
	cp build/linux/arch/arm/boot/zImage build/firmware
	cp build/linux/arch/arm/boot/dts/* build/firmware
	# build rootfs
	cp -a $(DESTDIR)/etc $(ROOTDIR)
	cp -a $(DESTDIR)/usr/lib/*.so* $(ROOTDIR)/usr/lib
	-find $(ROOTDIR) -type f \( ! -iname "*.ko" \) | xargs -I {} $(CROSS_COMPILE)strip {} 2> /dev/null
	mksquashfs $(ROOTDIR) build/firmware/rootfs.squashfs -b 64K -no-xattrs -root-owned -noappend -comp xz

clean:
	rm -rf build

.PHONY: bsp public rootfs

