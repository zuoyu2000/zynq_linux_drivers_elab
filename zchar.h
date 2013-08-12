#ifndef ZCHAR_H
#define ZCHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/types.h>
#include <asm/ioctl.h>

#define MODULE_NAME	"zchar"
#define MEM_LENGTH	5

#define zchar_IOCTL_BASE	'W'
#define zchar_DEVICE_INFO	_IO(zchar_IOCTL_BASE, 0)
#define zchar_DEVICE_CONTROL	_IO(zchar_IOCTL_BASE, 1)

#ifdef __cplusplus
}
#endif
#endif				/* ZCHAR_H */
