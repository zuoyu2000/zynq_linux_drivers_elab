#include "zynqflow.h"
#include <linux/delay.h>
#include <linux/wait.h>

static bool xdma_filter(struct dma_chan *chan, void *param){
   if (*((int *)chan->private) == *(int *)param)
      return true;
   
   return false;
}


static void dmatest_slave_tx_callback(void *completion)
{
	complete(completion);
}

static void dmatest_slave_rx_callback(void *completion)
{
	complete(completion);
}

int dma_init(zynqflow_t *main_c) {
   int i;
   dma_cap_mask_t mask;
   struct dma_chan *tx_chan,*rx_chan;
   struct xilinx_dma_config config;
   unsigned long tx_tmo = msecs_to_jiffies(1000);
   unsigned long rx_tmo = msecs_to_jiffies(1000);
	dma_cookie_t tx_cookie,rx_cookie;
	enum dma_status status;
	struct completion tx_cmp,rx_cmp;
	struct dma_async_tx_descriptor *txd = NULL, *rxd = NULL;
	enum dma_ctrl_flags flags;

   //enum dma_data_direction direction;
   u32 tx_match,rx_match;

   dma_cap_zero(mask);
   dma_cap_set(DMA_SLAVE, mask);
   dma_cap_set(DMA_PRIVATE, mask);

   tx_match = (DMA_MEM_TO_DEV & 0xFF) | XILINX_DMA_IP_DMA;
   rx_match = (DMA_DEV_TO_MEM & 0xFF) | XILINX_DMA_IP_DMA;
   printk("zfdriver: tx match is %x\n", tx_match);
   printk("zfdriver: rx match is %x\n", rx_match);

   tx_chan = dma_request_channel(mask, xdma_filter, (void *)&tx_match);
   rx_chan = dma_request_channel(mask, xdma_filter, (void *)&rx_match);

   if (tx_chan) printk("zfdriver: Found tx device\n");
   else      printk("zfdriver: Did not find tx device\n");
   if (rx_chan) printk("zfdriver: Found rx device\n");
   else      printk("zfdriver: Did not find rx device\n");
   
   
   // alloc memory in kernel space
   struct device *tx_dev = tx_chan->device->dev;
   struct device *rx_dev = rx_chan->device->dev;
   dma_addr_t tx_phy,rx_phy;
   u8 *tx_vir,*rx_vir;
   
   tx_vir = dma_alloc_coherent(tx_dev, LENGTH, &tx_phy, GFP_ATOMIC); //GFP_KERNEL
   if (tx_vir == NULL) {
      printk("zfdriver: DMA tx alloc error\n");
      return -1;
   }
   memset(tx_vir, 3, LENGTH);          // fill tx with 0s
   printk("zfdriver: tx vir address = %x \n", (u_int)tx_vir);
   printk("zfdriver: tx phy address = %x \n", (u_int)tx_phy);
   
   rx_vir = dma_alloc_coherent(rx_dev, LENGTH, &rx_phy, GFP_ATOMIC); //GFP_KERNEL
   if (rx_vir == NULL) {
      printk("zfdriver: DMA rx alloc error\n");
      return -1;
   }
   memset(rx_vir, 1, LENGTH);          // fill rx with a value
   printk("zfdriver: rx vir address = %x \n", (u_int)rx_vir);
   printk("zfdriver: rx phy address = %x \n", (u_int)rx_phy);
   
   // test before transfer:
   printk("zfdriver: rx buffer before transmit:\n");
   for (i = 0; i < 10; i++)   {
      printk("%d\t", rx_vir[i]);
   }
   printk("\n");
	
	// setup interrupts:
	config.coalesc = 1;
	config.delay = 0;
	rx_chan->device->device_control(rx_chan, DMA_SLAVE_CONFIG, (unsigned long)&config);
	tx_chan->device->device_control(tx_chan, DMA_SLAVE_CONFIG, (unsigned long)&config);
	
	//transfer:
	printk("zfdriver: dmaengine_prep_slave_single\n");
	flags = DMA_CTRL_ACK | DMA_COMPL_SKIP_DEST_UNMAP | DMA_PREP_INTERRUPT;
	rxd = dmaengine_prep_slave_single(rx_chan, rx_phy, LENGTH, DMA_DEV_TO_MEM, flags);
	txd = dmaengine_prep_slave_single(tx_chan, tx_phy, LENGTH, DMA_MEM_TO_DEV, flags);
	if (!rxd || !txd) {
		printk("zfdriver: dmaengine_prep_slave_single error");
	}
   
   printk("zfdriver: init completion\n");
   init_completion(&rx_cmp);
	rxd->callback = dmatest_slave_rx_callback;
	rxd->callback_param = &rx_cmp;
	rx_cookie = rxd->tx_submit(rxd);
   init_completion(&tx_cmp);
	txd->callback = dmatest_slave_tx_callback;
	txd->callback_param = &tx_cmp;
	tx_cookie = txd->tx_submit(txd);
   if (dma_submit_error(tx_cookie) || dma_submit_error(tx_cookie)) {
		printk("zfdriver: submit error");
	}

	printk("zfdriver: dma_issue_pending\n");
	dma_async_issue_pending(rx_chan);
	dma_async_issue_pending(tx_chan);
		
   printk("zfdriver: wait_for_completion\n");
	rx_tmo = wait_for_completion_timeout(&rx_cmp, rx_tmo);
   status = dma_async_is_tx_complete(rx_chan, rx_cookie, NULL, NULL);
   if (rx_tmo == 0) {
		printk("zfdriver: rx test timed out\n");
	} 
	else if (status != DMA_SUCCESS) {
		printk("zfdriver: rx got completion callback. Status: \'%s\'\n", 
		   status == DMA_ERROR ? "error" : "in progress");
	}
	
	tx_tmo = wait_for_completion_timeout(&tx_cmp, tx_tmo);
   status = dma_async_is_tx_complete(tx_chan, tx_cookie, NULL, NULL);
   if (tx_tmo == 0) {
		printk("zfdriver: tx test timed out\n");
	} 
	else if (status != DMA_SUCCESS) {
		printk("zfdriver: tx got completion callback. Status is \'%s\'\n", 
		   status == DMA_ERROR ? "error" : "in progress");
	}
   
   // test after transfer:
   printk("zfdriver: rx buffer after transfer:\n");
   for (i = 0; i < 10; i++)   {
      printk("%d\t", rx_vir[i]);
   }
   printk("\n");
   
   // release/free buffers:
   dma_free_coherent(tx_dev, LENGTH, tx_vir, tx_phy);
   dma_free_coherent(rx_dev, LENGTH, rx_vir, rx_phy);

   // release channels after test:
   if (tx_chan) dma_release_channel(tx_chan);
   if (rx_chan) dma_release_channel(rx_chan);

   return 0;
}

int dma_open(void) {
   return 0;
}

int dma_start(void) {
   return 0;
}

int dma_release(void) {
   return 0;
}

int dma_exit(zynqflow_t *main_c) {
   return 0;
}

MODULE_AUTHOR("e-Lab");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(MODULE_NAME "zflow");
MODULE_ALIAS(MODULE_NAME);
