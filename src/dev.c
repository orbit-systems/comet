#include "comet.h"
#include "dev.h"

void (*dev_receive_table[NUM_PORTS])(u64) = {
    [IC_PORT]  = IC_receive,
    [IOC_PORT] = IOC_receive,
    [MMU_PORT] = MMU_receive,
    [SYSTIMER_PORT] = SYSTIMER_receive,
};

void dev_receive() {
    if (!comet.ioc.out_pin) return;
    if (dev_receive_table[comet.ioc.port] != NULL) {
        dev_receive_table[comet.ioc.port](port_data(comet.ioc.port));
    }
    comet.ioc.out_pin = false;
}

void IC_receive(u64 data) {
    enum {
        ioc_standby,
        ioc_bindint_waiting4port,
        ioc_bindint_waiting4int,
    };

    static u8 status = ioc_standby;
    static u16 bindport = 0;

    switch (status) {
    case ioc_standby:
        if (port_data(IOC_PORT) != 0) break;
        status = ioc_bindint_waiting4port;
        break;
    case ioc_bindint_waiting4port:
        bindport = (u16)data;
        status = ioc_bindint_waiting4int;
        break;
    case ioc_bindint_waiting4int:
        bind_port(bindport, (u8)data);
        break;
    }
}

void IOC_receive(u64 data) {
    enum {
        ic_standby,
        ic_waiting4ivt,
    };

    static u8 status = ic_standby;

    switch (status) {
    case ic_standby:
        if (port_data(IOC_PORT) != 0) break;
        status = ic_waiting4ivt;
        break;
    case ic_waiting4ivt:
        comet.ic.ivt_base_address = data;
        break;
    }
}

void MMU_receive(u64 data) {
    TODO("mmu IO");
}

void SYSTIMER_receive(u64 data) {
    TODO("system timer IO");
}