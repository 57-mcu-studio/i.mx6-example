subdir = aufs bin dev etc home lib mnt opt proc root run sbin sys tmp usr var
subdir += usr/bin usr/lib usr/sbin usr/share
subdir += var/cache var/lock var/log var/run var/spool var/tmp

all:
	@mkdir -p $(ROOTDIR)
	@for dir in $(subdir); do \
		mkdir -p $(ROOTDIR)/$$dir; \
	done
	@find . -maxdepth 1 \( ! -iname "Makefile" ! -iname "\." \) | xargs -I {} cp -afr {} $(ROOTDIR)

