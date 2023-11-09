#
#       FILE NAME: Makefile
#
#       FILE DESCRIPTION: Make description file for building driver
#
#       PROJECT NAME: Linux Software (aDIO)
#
#       PROJECT VERSION: (Defined in README.TXT)
#
#       Copyright 2004 RTD Embedded Technologies, Inc.  All Rights Reserved.
#

#

#
# $(src) points to the relative path where this Makefile exists.  This must be
# used instead of getting the current working directory via pwd when telling
# the preprocessor where to find the driver header files.
#

EXTRA_CFLAGS = \
	-I$(src)/../include \
	-D$(shell $(src)/CHIPSET/CHIPSET) \

#
# Kernel build environment directory.  Supposedly it is safer to use this
# method of referring to it than using /usr/src.
#

KERNEL_DIR := /lib/modules/$(shell uname -r)/build

#
# Get Kernel Version for handling depricated SUBDIRS functionality after 
# Kernel version 5.0
#
KERNEL_VER := $(shell uname -r | cut -c1-3)


# Change how driver module library is compiled based on Kernel Version
ifeq (5.0, $(firstword $(sort $(KERNEL_VER) 5.0)))
SUBDIRS_OR_M := M
else
SUBDIRS_OR_M := SUBDIRS
endif

obj-m := rtd-aDIO.o

DRIVER_MODULE=rtd-aDIO.ko
FILES_TO_CLEAN= \
	*.o \
	rtd-aDIO.ko \
	.*.cmd \
	*.ko \
	.tmp_versions \
	*.mod.c \
    *.markers \
    *.symvers \
    *.order \
    *~ \
    *.mod

driver:		rtd-aDIO.c
	make -C $(KERNEL_DIR) $(SUBDIRS_OR_M)=`pwd` modules

clean:
	rm -rf $(FILES_TO_CLEAN)

load:
		insmod ./$(DRIVER_MODULE)
		@sleep 1
		chmod 666 /dev/rtd-aDIO-*

unload:
		rmmod rtd-aDIO
