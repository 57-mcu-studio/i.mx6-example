#!/bin/bash
if [ $# != 1 ]; then
	echo "USAGE: $0 <device>"
	exit 1
fi

dev=$1
sudo dd if=build/firmware/SPL of=${dev} bs=1K seek=1 oflag=dsync

target=`df | grep "${dev}1" | sed 's/.*% //'`

cp build/firmware/uEnv.txt ${target}
cp build/firmware/u-boot.img ${target}
cp build/firmware/zImage ${target}
cp build/firmware/*.dtb ${target}
cp build/firmware/rootfs.squashfs ${target}
