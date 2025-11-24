SRC_DIR := src
TEST_DIR := test
BUILD_DIR := build

# When called by kernel build system
ifneq ($(KERNELRELEASE),)
	obj-m += mychardev.o
	mychardev-objs := mychardev-main.o
else

all: module test

module: $(SRC_DIR)/mychardev.c
	@echo "Building kernel module..."
	@mkdir -p $(BUILD_DIR)
	@cp $(SRC_DIR)/mychardev.c $(BUILD_DIR)/mychardev-main.c
	@echo "obj-m += mychardev.o" > $(BUILD_DIR)/Kbuild
	@echo "mychardev-objs := mychardev-main.o" >> $(BUILD_DIR)/Kbuild
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/$(BUILD_DIR) modules
	@rm -f $(BUILD_DIR)/mychardev-main.c $(BUILD_DIR)/Kbuild

test: $(TEST_DIR)/test_mychardev.c
	@echo "Building test program..."
	@mkdir -p $(BUILD_DIR)
	gcc -o $(BUILD_DIR)/test_mychardev $(TEST_DIR)/test_mychardev.c

clean:
	@if [ -d $(BUILD_DIR) ]; then \
		$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/$(BUILD_DIR) clean 2>/dev/null || true; \
	fi
	rm -rf $(BUILD_DIR)/*
	@# Clean any stray files in root
	rm -f *.o *.ko *.mod* modules.order Module.symvers
	rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*.o
	rm -f .*cmd .*.o $(SRC_DIR)/.*cmd $(TEST_DIR)/.*cmd

install: module
	sudo insmod $(BUILD_DIR)/mychardev.ko

remove:
	sudo rmmod mychardev || true

info:
	modinfo $(BUILD_DIR)/mychardev.ko

.PHONY: all module test clean install remove info

endif
