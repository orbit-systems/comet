#pragma once
#define IO_H

#include "comet.h"
#include "mmu.h"
#include "ic.h"

// we will only have 256 ports lol
#define NUM_PORTS 256

// send information out of the CPU on a port
void send_out(u16 port, u64 data);

// send information to the CPU on a port
void send_in(u16 port, u64 data);

u64 bind_port(u16 port, u8 interrupt);

u64 port_data(u16 port);
