obj-m = module/kmap.o
ccflags-y = -I$(src)/common

KVERSION = $(shell uname -r)
BLDDIR= /lib/modules/$(KVERSION)/build

.PHONY: module clean
all: module umap

module: module/kmap.c
	@echo "Building kernel module"
	make -C $(BLDDIR) M=$(PWD) modules

umap: user/umap.c
	@echo "Building user application"
	@$(CC) -g -Icommon $^ -o $@

clean:
	@make -C $(BLDDIR) M=$(PWD) clean
	@rm -f umap
