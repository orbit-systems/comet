#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "core.h"
#include "physmem.h"
#include "system.h"
#include "refcountarena.h"
#include "common/fs.h"

int main(int argc, char** argv) {
    /* Load file from argv */
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        exit(-1);
    }

    FsFile* file = fs_open(argv[1], false, false);
    if (!file) {
        printf("Failed to open %s: %s\n", argv[1], strerror(errno));
        exit(-1);
    }

    string data = fs_read_entire(file, false);

    /* Allocate 1MB for messages */
    rca_set_global_arena(rca_create(1 << 20));

    /* Create new system */
    system_init();

    /* Create new core */
    CpuCore* core = core_init();

    system_install_core(core);

    /* Write loaded file to 0x0 */
    PhysMemUnit* pmu = system_get_pmu();
    physmem_write(pmu, 0, data.len, data.raw);

    pthread_t core_thread, system_thread;
    
    /* Create model threads */
    pthread_create(&core_thread, NULL, core_thread_main, core);
    pthread_create(&system_thread, NULL, system_thread_main, NULL);

    /* Busy wait until the system stops running */
    while (system_is_running()) 
        sched_yield();

    pthread_join(core_thread, NULL);
    pthread_join(system_thread, NULL);
    
    return 0;
}
