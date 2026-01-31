#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "core.h"
#include "common/fs.h"
#include "common/str.h"

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


}
