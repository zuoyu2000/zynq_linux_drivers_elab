zynq_linux_drivers_elab
=======================

zedboard / xilinx zynq linux drivers for AXI DMA and char devices that are memory-mapped

We share 2 drivers (kernel module) for the Zedboard / Xilinx Zynq 7020:

1- zdma: An AXI-DMA linux driver for simple mode. This useful example driver was not provided by xilinx at time of release. 
This is used to transfer a large amount of data to/from devices.
We tested it to about ~380 MB/s with 32 bit data and cloecked at 100MHz.

2- zchar: A memory-mapped IO for AXI ports, used to send smaller amounts of data, for example to configure hardware devices.


Compatibility:
These were tested with: https://github.com/Xilinx/linux-xlnx
version: https://github.com/Xilinx/linux-xlnx/commit/3f7c2d54957e950b3a36a251578185bfd374562c


Makefile:
```
obj-m  := zdma.o zchar.o

# Path to the Linux kernel, if not passed in as arg, set default.
ifeq ($(KDIR),)
  KDIR := /lib/modules/$(shell uname -r)/build
endif


all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
```


Enjoy
E + eLab
