//
// Created by dingjing on 4/24/22.
//

#ifndef GRACEFUL_PARTITION_LINUXVERSION_H
#define GRACEFUL_PARTITION_LINUXVERSION_H

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

int get_linux_version(void);

#endif //GRACEFUL_PARTITION_LINUXVERSION_H
