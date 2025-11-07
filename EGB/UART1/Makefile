obj-m += driver_serial.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
BUILD_DIR := $(PWD)/build
MODULE_NAME := driver_serial

all: | $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@mv *.ko *.mod.* *.o modules.order Module.symvers .tmp_versions $(BUILD_DIR)/ 2>/dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@rm -rf $(BUILD_DIR)

load: all
	sudo insmod $(BUILD_DIR)/$(MODULE_NAME).ko
	@echo "Módulo cargado. Verifica con: lsmod | grep $(MODULE_NAME)"

unload:
	sudo rmmod $(MODULE_NAME) 2>/dev/null || true
	@echo "Módulo descargado"

reload: unload load
	@echo "Módulo recargado"

status:
	@if lsmod | grep -q $(MODULE_NAME); then \
		echo "Módulo $(MODULE_NAME) está cargado"; \
	else \
		echo "Módulo $(MODULE_NAME) NO está cargado"; \
	fi

.PHONY: all clean load unload reload status