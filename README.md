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


Install:

set this variable (for example in your .cshrc):

```export KDIR=~/Desktop/linux-xlnx```

Then run ```make```

PS: this is also a full driver for AXI DMA from our group: https://github.com/culurciello/zync-xdma


Enjoy
E + eLab
