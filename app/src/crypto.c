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
#include "zxmacros.h"
#include "apdu_codes.h"
#include "parser.h"
#include "parser_common.h"
#include "b64url.h"
#include "crypto_store.h"
#include "cx.h"

#define MIN_BUFFER_LEN  256

zxerr_t crypto_getpubkey_part(uint8_t *buffer, uint16_t bufferLen, uint8_t index) {
    if (!crypto_store_is_initialized() || bufferLen < MIN_BUFFER_LEN) {
        return zxerr_invalid_crypto_settings;
    }

    zxerr_t err = crypto_pubkey_part(buffer, index);
    if (err != zxerr_ok){
        return err;
    }

    return zxerr_ok;
}

zxerr_t crypto_getsignature_part(uint8_t *buffer, uint16_t bufferLen, uint8_t index) {
    if (!is_sig_set() || bufferLen < MIN_BUFFER_LEN) {
        return zxerr_invalid_crypto_settings;
    }

    zxerr_t err = crypto_signature_part(buffer, index);
    if (err != zxerr_ok){
        return err;
    }

    return zxerr_ok;
}

zxerr_t crypto_sign(uint8_t *buffer, uint16_t signatureMaxlen, uint16_t *sigSize) {
    if (!crypto_store_is_initialized()) {
        return zxerr_invalid_crypto_settings;
    }

    if (signatureMaxlen < SHA384_DIGEST_LEN) {
        return zxerr_buffer_too_small;
    }

    uint8_t digest[SHA384_DIGEST_LEN] = {0};
    parser_error_t prs = parser_getDigest(digest, SHA384_DIGEST_LEN);
    if(prs != parser_ok){
        return zxerr_unknown;
    }

#ifdef APP_TESTING
    return zxerr_ok;
#endif

    uint8_t sig[RSA_MODULUS_LEN] = {0};

    cx_rsa_4096_private_key_t *rsa_privkey = crypto_store_get_privkey();
    if (rsa_privkey == NULL) {
        return zxerr_invalid_crypto_settings;
    }
    uint8_t digestsmall[CX_SHA256_SIZE] = {0};
    cx_hash_sha256(digest, SHA384_DIGEST_LEN, digestsmall, CX_SHA256_SIZE);

    zxerr_t error = zxerr_unknown;

    CATCH_CXERROR(cx_rsa_sign_no_throw((const cx_rsa_private_key_t *)rsa_privkey, CX_PAD_PKCS1_PSS, CX_SHA256, digestsmall, CX_SHA256_SIZE, sig, RSA_MODULUS_LEN));
    error = crypto_store_signature(sig);

catch_cx_error:
    rsa_privkey = NULL;
    MEMZERO(sig, RSA_MODULUS_LEN);

    if (error == zxerr_ok) {
        MEMCPY(buffer, digest, SHA384_DIGEST_LEN);
        *sigSize = SHA384_DIGEST_LEN;
    }
    return error;
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

    uint8_t hash_pubkey[CX_SHA256_SIZE];
    if (cx_hash_sha256(rsa_pubkey->n, rsa_pubkey->size, hash_pubkey, CX_SHA256_SIZE) != CX_SHA256_SIZE) {
        return zxerr_unknown;
    }

    answer_t *const answer = (answer_t *) buffer;

    if (b64url_encode(answer->addrStr, sizeof_field(answer_t, addrStr), hash_pubkey, CX_SHA256_SIZE) == 0) {
        MEMZERO(buffer, buffer_len);
        return zxerr_unknown;
    }
    *addrLen = sizeof(answer_t) - sizeof_field(answer_t, padding);
    return zxerr_ok;
}
