
TARGET=busybox-1.25.0

all:
	@if [ ! -d $(BUILD_DIR)/$(TARGET) ]; then \
		tar xf $(TARGET).tar.* -C $(BUILD_DIR); \
	fi
	@if [ ! -f $(BUILD_DIR)/$(TARGET)/.config ]; then \
		cp config $(BUILD_DIR)/$(TARGET)/.config; \
	fi
	@make -j$(CPUS) -C $(BUILD_DIR)/$(TARGET)
	@make -j$(CPUS) -C $(BUILD_DIR)/$(TARGET) install
	@cp -a $(BUILD_DIR)/$(TARGET)/_install/* $(ROOTDIR)

