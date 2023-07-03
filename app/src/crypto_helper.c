/*******************************************************************************
*   (c) 2018 - 2022 Zondax AG
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
#if defined(LEDGER_SPECIFIC)
    #include "bolos_target.h"
    #include "cx.h"
#else
    #include <sha512.h>
#endif

#include "zxmacros.h"
#include "zxerror.h"

zxerr_t crypto_sha384(const unsigned char *in, unsigned int inLen, unsigned char *out, unsigned int outLen) {
#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2)
    cx_sha512_t ctx;
    cx_sha384_init(&ctx);
    if (cx_hash_no_throw(&ctx.header, CX_LAST, in, inLen, out, outLen) != CX_OK) {
        return zxerr_unknown;
    }
#else
    uint8_t tmp[64] = {0};
    // This function requires a full 64 bytes context (sha512)
    SHA384(in, inLen, tmp);
    // We can later trim and return only 48
    MEMCPY(out, tmp, 48);
#endif
    return zxerr_ok;
}
