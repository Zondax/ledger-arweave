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

#include "cx.h"
#include "zxmacros.h"
#include <stdbool.h>
#include "zxerror.h"

#ifdef __cplusplus
extern "C" {
#endif

bool crypto_store_is_initialized();
bool is_sig_set();

zxerr_t crypto_store_signature(uint8_t *sig);
zxerr_t crypto_signature_part(uint8_t *sig, uint8_t index);

zxerr_t crypto_deriveMasterSeed();
zxerr_t crypto_init_primes();
zxerr_t crypto_init_keys();

zxerr_t crypto_store_init();

cx_rsa_4096_public_key_t* crypto_store_get_pubkey();
cx_rsa_4096_private_key_t* crypto_store_get_privkey();

#ifdef __cplusplus
}
#endif
