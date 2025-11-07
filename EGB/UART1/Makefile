obj-m += driver_serial.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
BUILD_DIR := $(PWD)/build

all: | $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@mv *.ko *.mod* *.o modules.order Module.symvers .*.cmd .tmp_versions $(BUILD_DIR)/ 2>/dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@rm -rf $(BUILD_DIR)

.PHONY: all clean