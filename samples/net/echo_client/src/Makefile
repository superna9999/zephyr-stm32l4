ccflags-$(CONFIG_NETWORKING_WITH_BT) +=-I${ZEPHYR_BASE}/samples/bluetooth/

obj-y = echo-client.o

ifeq ($(CONFIG_BOARD_ARDUINO_101),y)
ccflags-y +=-I${ZEPHYR_BASE}/include/drivers/
obj-y += ../../common/cc2520_a101.o
endif

ifeq ($(CONFIG_NET_TESTING), y)
ccflags-y +=-I${ZEPHYR_BASE}/samples/net/common/
ccflags-y +=-DNET_TESTING_SERVER=0
endif

ifeq ($(CONFIG_NETWORKING_WITH_BT), y)
	obj-y += ../../../bluetooth/gatt/ipss.o
endif
