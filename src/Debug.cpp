#include "Debug.h"

TDebug Debug;    // Global Instance


size_t TDebug::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while(size--) {
        n += write(*buffer++);
    }
    return n;
}
