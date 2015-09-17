#include "crypto.h"
#include <openssl\rand.h>

void generateNonce(unsigned char* buffer, int size)
{
    RAND_bytes(buffer, size);
}