/*******************************************************************************
*   (c) 2019 Zondax GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "crypto.h"
#include "coin.h"
#include "b64url.h"

#if defined(TARGET_NANOS) || defined(TARGET_NANOX)
#include "crypto_store.h"

void crypto_sha384(const unsigned char *in, unsigned int inLen, unsigned char *out, unsigned int outLen) {
    // FIXME: Review this
    cx_sha512_t ctx;
    cx_sha384_init(&ctx);
    cx_hash(&ctx.header, CX_LAST, in, inLen, out, outLen);
}

#include "cx.h"

typedef struct {
    // TODO: FIXME
} __attribute__((packed)) signature_t;

zxerr_t crypto_sign(uint8_t *buffer, uint16_t signatureMaxlen, const uint8_t *message, uint16_t messageLen, uint16_t *sigSize) {
    if (!crypto_store_is_initialized()) {
        return zxerr_invalid_crypto_settings;
    }

    // FIXME: complete this
    return zxerr_ok;
}

typedef struct {
    char addrStr[43];
    uint8_t padding[4];
} __attribute__((packed)) answer_t;

zxerr_t crypto_fillAddress(uint8_t *buffer, uint16_t buffer_len, uint16_t *addrLen) {
    MEMZERO(buffer, buffer_len);
    if (!crypto_store_is_initialized()) {
        return zxerr_invalid_crypto_settings;
    }

    if (buffer_len < sizeof(answer_t)) {
        zemu_log_stack("crypto_fillAddress: zxerr_buffer_too_small");
        return zxerr_buffer_too_small;
    }

    cx_rsa_4096_public_key_t *rsa_pubkey = crypto_store_get_pubkey();
    if (rsa_pubkey == NULL) {
        return zxerr_invalid_crypto_settings;
    }

    // FIXME: hash
    uint8_t hash_pubkey[CX_SHA256_SIZE];
    cx_hash_sha256(rsa_pubkey->n, rsa_pubkey->size, hash_pubkey, CX_SHA256_SIZE);

    answer_t *const answer = (answer_t *) buffer;

    b64url_encode(answer->addrStr, sizeof_field(answer_t, addrStr), hash_pubkey, CX_SHA256_SIZE);
    *addrLen = sizeof(answer_t) - sizeof_field(answer_t, padding);
    return zxerr_ok;
}

#else
#include <sha512.h>

void crypto_sha384(const unsigned char *in, unsigned int inLen, unsigned char *out, unsigned int outLen) {
    uint8_t tmp[64];
    // This function requires a full 64 bytes context (sha512)
    SHA384(in, inLen, tmp);
    // We can later trim and return only 48
    MEMCPY(out, tmp, 48);
}
#endif
