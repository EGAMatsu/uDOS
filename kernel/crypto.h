#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#define CryptoARC4Encode _Zcyrc4e
uint8_t *CryptoARC4Encode(void *bitstream, size_t len, void *key, size_t keylen);

#endif
