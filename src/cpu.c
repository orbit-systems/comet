#include "comet.h"
#include "mmu.h"
#include "ic.h"

// yeah
i64 forceinline mod64(i64 a, i64 b) {
    if (b == -1) return 0;
    i64 r = a % b;
    if (r < 0) {
        if (b >= 0) r += b;
        else        r -= b;
    }
    return r;
}

void run() {
    
}