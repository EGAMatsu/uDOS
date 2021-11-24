#include <stddef.h>
#include <stdint.h>
#include <Mm/Mm.h>
#include <Debug/Panic.h>
#include <Crypto.h>

uint8_t *CryptoARC4Encode(
    void *bitstream,
    size_t len,
    void *key,
    size_t keylen)
{
    uint8_t *_bitstream = (uint8_t *)bitstream, *_key = (uint8_t *)key;
    uint8_t S[256], *K, *ctext;
    size_t i, j, k;

    /* Initialize S */
    for(i = 0; i < 256; i++) {
        S[i] = i;
    }
    j = 0;
    for(i = 0; i < 256; i++) {
        uint8_t swp;
        j = (j + S[i] + _key[i % keylen]) % 256;
        swp = S[i];
        S[i] = S[j];
        S[j] = swp;
    }

    K = MmAllocate(len);
    if(K == NULL) {
        KePanic("Out of memory");
    }

    /* Produce an array that is later XOR'ed to create the ciphered text */
    i = 0;
    j = 0;
    for(k = 0; k < len; k++) {
        uint8_t swp;

        i = (i + 1) % 256;
        j = (j + S[i]) % 256;

        swp = S[i];
        S[i] = S[j];
        S[j] = swp;

        K[k] = S[(S[i] + S[j]) % 256];
    }

    /* XOR the bitstream and the K array to create the final ciphered text */
    ctext = MmAllocate(len);
    if(ctext == NULL) {
        KePanic("Out of memory");
    }
    for(i = 0; i < len; i++) {
        ctext[i] = _bitstream[i] ^ K[i];
    }
    MmFree(K);
    return ctext;
}
