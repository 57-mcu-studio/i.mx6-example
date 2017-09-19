export BUILD_DIR=$(BUILD_PATH)/public

subdir = busybox

all: prepare
	@for dir in $(subdir); do \
		make -C $$dir; \
		rm -f $(DESTDIR)/usr/lib/*.la; \
	done

prepare:
	@mkdir -p $(BUILD_DIR)

