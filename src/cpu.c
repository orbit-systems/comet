#include "comet.h"

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
i32 forceinline mod32(i32 a, i32 b) {
    if (b == -1) return 0;
    i32 r = a % b;
    if (r < 0) {
        if (b >= 0) r += b;
        else        r -= b;
    }
    return r;
}
i16 forceinline mod16(i16 a, i16 b) {
    if (b == -1) return 0;
    i16 r = a % b;
    if (r < 0) {
        if (b >= 0) r += b;
        else        r -= b;
    }
    return r;
}
i8 forceinline mod8(i8 a, i8 b) {
    if (b == -1) return 0;
    i8 r = a % b;
    if (r < 0) {
        if (b >= 0) r += b;
        else        r -= b;
    }
    return r;
}

void run() {
    
}