# linux kernel
KERNEL_DIR=linux
KERNEL_CONFIG=config/linux_config
KERNEL_BUILD_DIR=$(BUILD_PATH)/linux

# u-boot
UBOOT_DIR=u-boot
UBOOT_CONFIG=config/uboot_config
UBOOT_BUILD_DIR=$(BUILD_PATH)/uboot

all: kernel uboot

kernel:
	@if [ ! -d $(KERNEL_BUILD_DIR) ]; then \
		mkdir -p $(KERNEL_BUILD_DIR); \
	fi
	@if [ ! -f $(KERNEL_BUILD_DIR)/.config ]; then \
		cp $(KERNEL_CONFIG) $(KERNEL_BUILD_DIR)/.config; \
	fi
	@make -j$(CPUS) ARCH=arm -C $(KERNEL_DIR) O=$(KERNEL_BUILD_DIR) zImage
	@make -j$(CPUS) ARCH=arm -C $(KERNEL_DIR) O=$(KERNEL_BUILD_DIR) modules
	@make -j$(CPUS) ARCH=arm -C $(KERNEL_DIR) O=$(KERNEL_BUILD_DIR) imx6dl-edm1-cf.dtb imx6q-edm1-cf.dtb
	@make -j$(CPUS) ARCH=arm -C $(KERNEL_DIR) O=$(KERNEL_BUILD_DIR) modules_install INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=$(ROOTDIR)

kernel_config:
	@if [ ! -d $(KERNEL_BUILD_DIR) ]; then \
		mkdir -p $(KERNEL_BUILD_DIR); \
	fi
	@if [ ! -f $(KERNEL_BUILD_DIR)/.config ]; then \
		cp $(KERNEL_CONFIG) $(KERNEL_BUILD_DIR)/.config; \
	fi
	@make -j$(CPUS) ARCH=arm -C $(KERNEL_DIR) O=$(KERNEL_BUILD_DIR) menuconfig

uboot:
	@if [ ! -d $(UBOOT_BUILD_DIR) ]; then \
		mkdir -p $(UBOOT_BUILD_DIR); \
	fi
	@if [ ! -f $(UBOOT_BUILD_DIR)/.config ]; then \
		cp $(UBOOT_CONFIG) $(UBOOT_BUILD_DIR)/.config; \
	fi
	@make -j$(CPUS) -C $(UBOOT_DIR) O=$(UBOOT_BUILD_DIR)

