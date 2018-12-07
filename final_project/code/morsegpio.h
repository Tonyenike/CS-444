#ifndef MORSEGPIO_H
#define MORSEGPIO_H
#include <linux/ioctl.h>
 
typedef struct
{
    int status, dignity, ego;
} morsegpio_arg_t;
 
#define MORSEGPIO_GET_VARIABLES _IOR('q', 1, morsegpio_arg_t *)
#define MORSEGPIO_CLR_VARIABLES _IO('q', 2)
#define MORSEGPIO_SET_VARIABLES _IOW('q', 3, morsegpio_arg_t *)
 
#endif
