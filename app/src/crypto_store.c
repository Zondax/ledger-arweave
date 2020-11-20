/*******************************************************************************
*   (c) 2020 Zondax GmbH
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

#include "os.h"
#include "view.h"
#include "coin.h"
#include "crypto.h"
#include "app_main.h"
#include "crypto_store.h"

typedef struct {
    uint8_t masterSeed[32];
    uint8_t pq[RSA_PRIME_LEN * 2];
    cx_rsa_4096_public_key_t rsa_pub;
    cx_rsa_4096_private_key_t rsa_priv;
    uint8_t initialized;
} crypto_store_t;

crypto_store_t NV_CONST
N_crypto_store_impl __attribute__ ((aligned(64)));
#define N_crypto_store (*(NV_VOLATILE crypto_store_t *)PIC(&N_crypto_store_impl))

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

zxerr_t crypto_deriveMasterSeed() {
    uint8_t masterSeed[64];
    uint32_t master_path[] = {
            HDPATH_0_DEFAULT,
            HDPATH_1_DEFAULT,
            0x80000000u,
            0x80000000u,
            0x80000000u
    };

    // Generate random seed by using Ed25519 private key
    // FIXME: Review this
    os_perso_derive_node_bip32_seed_key(HDW_NORMAL,
                                        CX_CURVE_Ed25519,
                                        master_path, HDPATH_LEN_DEFAULT,
                                        masterSeed,
                                        NULL, NULL, 0);

    MEMCPY_NV(&N_crypto_store.masterSeed, masterSeed, 32);
    return zxerr_ok;
}

zxerr_t crypto_derivePrime(uint8_t *prime, uint8_t index) {
    // Now derive p_seed and q_seed from the master seed
    cx_sha3_t hash_sha3;
    uint8_t data_index[2] = {0, index};
    cx_shake256_init(&hash_sha3, RSA_PRIME_LEN * 8);
    cx_hash(&hash_sha3.header, 0, (void *) data_index, 2, NULL, 0);
    cx_hash(&hash_sha3.header, CX_LAST, (void *) &N_crypto_store.masterSeed, 32, prime, RSA_PRIME_LEN);

    zemu_log_stack("derivePrime::done");
    return zxerr_ok;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool crypto_store_is_initialized() {
    return N_crypto_store.initialized;
}

zxerr_t crypto_init_primes() {
    zemu_log_stack("initPrimes");

    uint8_t pq[RSA_PRIME_LEN * 2] = {219, 54, 0, 187, 240, 160, 180, 71, 123, 109, 211, 104, 207, 8, 169, 189, 43, 140, 158,
                                     58, 84, 98, 85, 147, 208, 165, 127, 107, 173, 67, 108, 118, 52, 159, 182, 108, 53, 105,
                                     131, 7, 149, 234, 123, 160, 66, 195, 108, 116, 27, 73, 30, 219, 34, 13, 227, 194, 168,
                                     128, 239, 226, 138, 76, 221, 118, 104, 195, 20, 48, 233, 189, 94, 58, 246, 167, 71,
                                     227, 40, 26, 83, 212, 155, 175, 31, 196, 167, 39, 198, 89, 165, 227, 5, 131, 49, 144,
                                     30, 183, 97, 58, 26, 33, 53, 114, 44, 194, 210, 198, 18, 38, 122, 243, 144, 141, 24,
                                     35, 123, 188, 96, 141, 67, 123, 38, 218, 244, 227, 42, 237, 20, 140, 79, 80, 117, 222,
                                     199, 3, 144, 195, 58, 183, 129, 50, 185, 134, 249, 124, 21, 135, 12, 54, 87, 86, 159,
                                     127, 181, 200, 234, 141, 38, 227, 113, 24, 136, 214, 254, 36, 206, 202, 92, 150, 187,
                                     174, 91, 66, 61, 78, 122, 187, 153, 77, 144, 155, 76, 109, 183, 48, 252, 149, 133, 155,
                                     229, 43, 16, 158, 145, 62, 98, 186, 21, 0, 239, 204, 78, 178, 58, 94, 216, 77, 66, 34,
                                     86, 36, 14, 91, 97, 61, 65, 132, 54, 190, 142, 223, 16, 103, 116, 108, 72, 243, 81,
                                     148, 61, 42, 39, 220, 0, 92, 154, 4, 59, 136, 154, 250, 42, 13, 219, 204, 32, 133, 225,
                                     101, 101, 186, 66, 214, 230, 96, 51, 1, 193, 109, 89, 96, 193, 182, 73, 77, 37, 13,
                                     111, 245, 245, 212, 215, 14, 241, 82, 221, 216, 132, 41, 252, 88, 225, 17, 129, 52,
                                     213, 42, 73, 69, 251, 181, 92, 230, 127, 214, 26, 143, 146, 151, 96, 157, 153, 171,
                                     116, 38, 143, 93, 11, 204, 13, 111, 20, 19, 221, 79, 59, 129, 129, 187, 137, 223, 216,
                                     165, 190, 100, 125, 147, 248, 32, 62, 191, 4, 46, 151, 99, 31, 146, 33, 51, 186, 197,
                                     232, 220, 16, 93, 136, 9, 37, 234, 130, 71, 162, 33, 120, 138, 252, 113, 216, 63, 51,
                                     248, 41, 31, 73, 3, 130, 52, 86, 238, 183, 57, 57, 23, 157, 60, 207, 92, 140, 62, 209,
                                     77, 107, 225, 211, 88, 148, 203, 204, 135, 100, 48, 136, 207, 110, 109, 152, 54, 120,
                                     54, 189, 210, 51, 125, 205, 130, 252, 86, 67, 14, 94, 21, 135, 39, 44, 63, 238, 3, 52,
                                     218, 240, 83, 186, 165, 220, 8, 53, 4, 126, 39, 72, 143, 65, 171, 207, 117, 26, 93,
                                     246, 190, 201, 93, 25, 233, 52, 223, 148, 150, 80, 242, 175, 1, 157, 122, 76, 206, 106,
                                     89, 75, 50, 116, 228, 103, 13, 253, 67, 120, 142, 85, 206, 114, 81, 139, 135, 233, 63,
                                     205, 229, 121, 147, 185, 155, 192, 236, 226, 86, 250, 77, 96, 189, 60, 52, 95, 20, 22,
                                     61, 213, 161, 57, 169, 240, 63, 98, 18, 61, 201, 134, 72, 128, 179, 0, 158, 206, 87};

    view_message_show("Arweave", "Finding Pseed");
    UX_WAIT_DISPLAYED();
    //crypto_derivePrime(pq, 0);;

    view_message_show("Arweave", "Finding Qseed");
    UX_WAIT_DISPLAYED();
    //crypto_derivePrime(pq + RSA_PRIME_LEN, 1);;

    // FIXME: Why setting high bit here?
    // https://github.com/LedgerHQ/openpgp-card-app/blob/f3bdb7908eafcf23982d6af4a8da4bd7901fb704/src/gpg_gen.c#L146-L147
    //*pq |= 0x80;
    //*(pq + RSA_PRIME_LEN) |= 0x80;

    // Obtain two prime numbers p, q inplace
    view_message_show("Arweave", "Finding P");
    UX_WAIT_DISPLAYED();
    //cx_math_next_prime(pq, RSA_PRIME_LEN);

    view_message_show("Arweave", "Finding Q");
    UX_WAIT_DISPLAYED();
    //cx_math_next_prime(pq + RSA_PRIME_LEN, RSA_PRIME_LEN);
    MEMCPY_NV(&N_crypto_store.pq, pq, RSA_PRIME_LEN * 2);

    return zxerr_ok;
}

zxerr_t crypto_init_keys() {
    zemu_log_stack("crypto::initKeys");

    cx_rsa_4096_public_key_t rsa_pub;
    cx_rsa_4096_private_key_t rsa_priv;
    const uint8_t exp_be_buf[] = {0x00, 0x01, 0x00, 0x01};    // default rsa value 65337

    view_message_show("Arweave", "gen pair");
    UX_WAIT_DISPLAYED();
    cx_rsa_generate_pair(RSA_MODULUS_LEN,
                         (cx_rsa_public_key_t * ) & rsa_pub,
                         (cx_rsa_private_key_t * ) & rsa_priv,
                         exp_be_buf, 4,
                         (const unsigned char *) &N_crypto_store.pq);

    view_message_show("Arweave", "store keys");
    UX_WAIT_DISPLAYED();
    SET_NV(&N_crypto_store.initialized, uint8_t, true)
    MEMCPY_NV(&N_crypto_store.rsa_pub, &rsa_pub, sizeof(rsa_pub));
    MEMCPY_NV(&N_crypto_store.rsa_priv, &rsa_priv, sizeof(rsa_priv));

    return zxerr_ok;
}

zxerr_t crypto_store_init() {
    // Derive master seed from mnemonic
    // Based on https://github.com/LedgerHQ/openpgp-card-app/blob/master/doc/developper/gpgcard3.0-addon.rst#how

    if (!crypto_store_is_initialized()) {

#ifdef APP_TESTING
        zemu_log_stack("-----");    // Crash device if testmode is enabled
        SET_NV(&N_crypto_store.initialized, uint8_t, true)
        return zxerr_ok;
#endif
        view_message_show("Arweave", "Initializing...");
        UX_WAIT_DISPLAYED();

        view_message_show("Arweave", "Init seed");
        UX_WAIT_DISPLAYED();
        crypto_deriveMasterSeed();

        view_message_show("Arweave", "Finding primes");
        UX_WAIT_DISPLAYED();
        zemu_log_stack("init::primes");
        crypto_init_primes();

        view_message_show("Arweave", "Finding keys");
        UX_WAIT_DISPLAYED();
        crypto_init_keys();

        zemu_log_stack("initialized");
    }
}

cx_rsa_4096_public_key_t *crypto_store_get_pubkey() {
    if (!crypto_store_is_initialized()) {
        return NULL;
    }
    return &N_crypto_store.rsa_pub;
}

cx_rsa_4096_private_key_t *crypto_store_get_privkey() {
    if (!crypto_store_is_initialized()) {
        return NULL;
    }
    return &N_crypto_store.rsa_priv;
}

