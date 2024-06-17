#pragma once
#define DEV_H

#include "comet.h"
#include "io.h"

// run corresponding device handler
void dev_receive();

#define IC_PORT 0
void IC_receive(u64 data);

#define IOC_PORT 1
void IOC_receive(u64 data);

#define MMU_PORT 2
void MMU_receive(u64 data);

#define SYSTIMER_PORT 3
void SYSTIMER_receive(u64 data);

// some custom devices
#define GPU_PORT 10
void GPU_receive(u64 data);
