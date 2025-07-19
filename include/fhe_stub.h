#ifndef FHE_STUB_H
#define FHE_STUB_H

#include <stddef.h>

#define FHE_CIPHERTEXT_SIZE 64

typedef struct {
    char stub_encrypted[FHE_CIPHERTEXT_SIZE];
} fhe_ciphertext_t;

void fhe_initialize();
fhe_ciphertext_t fhe_encrypt(double plaintext);
double fhe_decrypt(fhe_ciphertext_t ciphertext);
fhe_ciphertext_t fhe_add(fhe_ciphertext_t a, fhe_ciphertext_t b);
fhe_ciphertext_t fhe_mul(fhe_ciphertext_t a, double scalar);
void attach_encrypted_density(TorusNode *n);

#endif // FHE_STUB_H
