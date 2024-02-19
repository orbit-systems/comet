#pragma once
#define IC_H

#include "comet.h"
#include "mmu.h"

// interrupt handling and interrupt controller
void init_IC();

// interpret mmu response and push interrupt if applicable
void push_interrupt_from_MMU(mmu_response res);

// push interrupt
void push_interrupt(u8 code);

// pop interrupt and return to the exit point
void return_interrupt();

// pop interrupt while resuming execution at the current location
void resolve_interrupt();