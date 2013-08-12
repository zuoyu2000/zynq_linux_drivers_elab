#include <linux/slab.h>         // kalloc 
#include <linux/version.h>
#include <linux/module.h>
#include <linux/mm.h>           // memory re-mapping
#include <linux/fs.h>           // struct file
#include <linux/ioctl.h>        // ioctl
#include <linux/uaccess.h>      // copy_to_user or from_user
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>  // dma access
#include <linux/amba/xilinx_dma.h>  // xilinx AXI DMA

#define LENGTH 1024

typedef struct zynqflow{
   struct zf_dma *dma;
   struct zf_file *r_f;
   struct zf_ioctl *r_io;
}zynqflow_t;

typedef struct zf_dma{
   dma_addr_t phy_addr;
   u_int *vir_addr;
   u32 idx;
}zf_dma_t;

typedef struct zf_file{
   int f_size;
}zf_file_t;

typedef struct zf_ioctl{
   u_int cmd;
   u_int size;
   u_int addr;
}zf_ioctl_t;

extern int dma_init(zynqflow_t *main_c);
extern int dma_exit(zynqflow_t *main_c);
