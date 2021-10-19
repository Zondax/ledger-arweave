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

typedef struct {
    uint8_t sig[RSA_MODULUS_LEN];
} crypto_sig_t;

crypto_sig_t NV_CONST
N_crypto_sig_impl __attribute__ ((aligned(64)));
#define N_crypto_sig (*(NV_VOLATILE crypto_sig_t *)PIC(&N_crypto_sig_impl))

uint8_t signature_set;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

zxerr_t crypto_store_signature(uint8_t *sig){
    MEMCPY_NV(&N_crypto_sig.sig, sig, RSA_MODULUS_LEN);
    signature_set = 1;
    return zxerr_ok;
}

bool is_sig_set(){
    return signature_set == 1;
}

zxerr_t crypto_signature_part(uint8_t *sig, uint8_t index){
    if (signature_set != 1){
        return zxerr_invalid_crypto_settings;
    }
    uint8_t *start = (uint8_t *)&N_crypto_sig.sig;
    MEMCPY(sig, start + index*RSA_MODULUS_HALVE, RSA_MODULUS_HALVE);
    if (index == 1){
        uint8_t dummy[RSA_MODULUS_LEN];
        MEMZERO(dummy, RSA_MODULUS_LEN);
        MEMCPY_NV(&N_crypto_sig.sig, dummy, RSA_MODULUS_LEN);
        signature_set = 0;
    }
    return zxerr_ok;
}

zxerr_t crypto_pubkey_part(uint8_t *key, uint8_t index){
    if (!crypto_store_is_initialized()){
        return zxerr_invalid_crypto_settings;
    }
    cx_rsa_4096_public_key_t *rsa_pubkey = crypto_store_get_pubkey();
    uint8_t *start = rsa_pubkey->n;
    MEMCPY(key, start + index*RSA_MODULUS_HALVE, RSA_MODULUS_HALVE);
    return zxerr_ok;
}


zxerr_t crypto_deriveMasterSeed() {
    uint8_t masterSeed[64];
    uint32_t master_path[] = {
            HDPATH_0_DEFAULT,
            HDPATH_1_DEFAULT,
            0x80000000u,
            0x80000000u,
            0x80000000u
    };

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
    uint8_t data_index[2] = {0, index};
    uint8_t data[sizeof(data_index) + sizeof(N_crypto_store.masterSeed) + 1];

    //initialization of data to be hashed
    //Data: 0 || PRIME_INDEX (0 or 1) || MASTERSEED (32 bytes) || INDEX (0 - 255)
    MEMCPY(data, data_index, 2);
    MEMCPY(data + 2, &N_crypto_store.masterSeed, sizeof(N_crypto_store.masterSeed));

    uint16_t index_location = 2 + sizeof(N_crypto_store.masterSeed);

    //Number of times we need to hash: RSA_PRIME_LEN * 8 bits / 256 bits
    uint16_t num_hashes = (RSA_PRIME_LEN*8)/256;
    if(num_hashes > 255){
        return zxerr_buffer_too_small;
    }

    //We can have a remainder of a few bytes if not divisible by 32 bytes
    uint16_t remainder = (RSA_PRIME_LEN*8) % 256;

    uint8_t hash_index = 0;
    // Fill the random bytes of the prime
    for(;hash_index < num_hashes; hash_index ++, prime += 32){
        data[index_location] = hash_index;
        cx_hash_sha256(data, sizeof(data), prime, CX_SHA256_SIZE);
    }

    // Fill the remainder bytes of the prime
    if (remainder > 0){
        uint8_t final_hash[CX_SHA256_SIZE];
        data[index_location] = hash_index;
        cx_hash_sha256(data, sizeof(data), final_hash, CX_SHA256_SIZE);\
        MEMCPY(prime, final_hash, remainder / 8);
    }

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

    uint8_t pq[RSA_PRIME_LEN * 2];

    view_message_show("Arweave", "Finding Pseed");
    UX_WAIT_DISPLAYED();
    crypto_derivePrime(pq, 0);;

    view_message_show("Arweave", "Finding Qseed");
    UX_WAIT_DISPLAYED();
    crypto_derivePrime(pq + RSA_PRIME_LEN, 1);;

    *pq |= 0x80;
    *(pq + RSA_PRIME_LEN) |= 0x80;

    // Obtain two prime numbers p, q inplace
    view_message_show("Arweave", "Finding P");
    UX_WAIT_DISPLAYED();
    cx_math_next_prime(pq, RSA_PRIME_LEN);

    view_message_show("Arweave", "Finding Q");
    UX_WAIT_DISPLAYED();
    cx_math_next_prime(pq + RSA_PRIME_LEN, RSA_PRIME_LEN);
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
        return zxerr_ok;
    }else{
        //do nothing
        return zxerr_ok;
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

