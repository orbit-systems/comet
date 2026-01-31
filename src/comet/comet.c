#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "core.h"
#include "common/fs.h"
#include "common/str.h"
#include "message.h"
#include "system.h"

int main(int argc, char** argv) {
    /* Load file from argv */
    // if (argc != 2) {
    //     printf("Usage: %s filename\n", argv[0]);
    //     exit(-1);
    // }

    // FsFile* file = fs_open(argv[1], false, false);
    // if (!file) {
    //     printf("Failed to open %s: %s\n", argv[1], strerror(errno));
    //     exit(-1);
    // }
    
    // string data = fs_read_entire(file, false);

    /* Create new system */
    system_init();

    /* Create new core */
    CpuCore* core = core_init();

    system_install_core(core);

    pthread_t core_thread;
    
    /* Create core thread */
    pthread_create(&core_thread, NULL, core_thread_main, core);

    /* Perform LOAD operation */
    u64 addr = 0x1234;
    system_enqueue_message((SystemMessage){.type = MSG_CORE_LOAD, .data=&addr, .length = 4});

    while (core->running) {
        system_process_message();
        sched_yield();
        core_enqueue_message(core, (SystemMessage){.type=MSG_SYS_STOP});
    }

    pthread_join(core_thread, NULL);

    return 0;
}
