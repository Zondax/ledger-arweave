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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "zxmacros.h"
#include <stdbool.h>
#include "zxerror.h"

#if defined(TARGET_NANOS)
#include "lcx_rsa.h"
#endif

#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX)
#include "cx.h"
#endif

bool crypto_store_is_initialized();
bool is_sig_set();
bool crypto_store_slot_is_initialized(uint8_t slot);
bool same_masterseed(uint8_t slot);
zxerr_t crypto_initialize_slot();
zxerr_t crypto_uninitialized_slot(uint8_t slot);

zxerr_t crypto_store_signature(uint8_t *sig);
zxerr_t crypto_signature_part(uint8_t *sig, uint8_t index);

zxerr_t crypto_deriveMasterSeed();
zxerr_t crypto_init_primes();
zxerr_t crypto_init_keys();

zxerr_t crypto_store_init();
bool crypto_store_init_test();

cx_rsa_4096_public_key_t* crypto_store_get_pubkey();
cx_rsa_4096_private_key_t* crypto_store_get_privkey();

zxerr_t crypto_pubkey_part(uint8_t *key, uint8_t index);

#ifdef __cplusplus
}
#endif
