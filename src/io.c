#include "io.h"

u8   binding[NUM_PORTS]  = {0};
bool is_bound[NUM_PORTS] = {false};
u64  ports[NUM_PORTS]    = {0};

// send information out of the CPU on a port
void send_out(u16 port, u64 data) {
    port = port % NUM_PORTS;
    comet.ioc.out_pin = true;
    comet.ioc.port = port;
    ports[port] = data;
}

// send information to the CPU on a port
void send_in(u16 port, u64 data) {
    port = port % NUM_PORTS;
    ports[port] = data;
    if (is_bound[port]) {
        push_interrupt(binding[port]);
    }
}

void bind_port(u16 port, u8 interrupt) {
    port = port % 256;
    is_bound[port] = true;
    binding[port] = interrupt;
}

u64 port_data(u16 port) {
    return ports[port % NUM_PORTS];
}