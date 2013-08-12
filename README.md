zynq_linux_drivers_elab
=======================

zedboard / xilinx zynq linux drivers for AXI DMA and char devices that are memory-mapped

We share 2 drivers for the Zedboard / Xilinx Zynq 7020:

1- An AXI-DMA linux driver for simple mode. This useful example driver was not provided by xilinx at time of release. 
This is used to transfer a large amount of data to/from devices. We tested it to about ~380MB/s

2- A memory-mapped IO for AXI ports, used to send smaller amounts of data, for example to configure hardware devices.

Enjoy
E + eLab
