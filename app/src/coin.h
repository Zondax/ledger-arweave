/*******************************************************************************
*  (c) 2018-2021 Zondax GmbH
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

#define CLA                             0x44

#define HDPATH_LEN_DEFAULT   5

#define HDPATH_0_DEFAULT     (0x80000000u | 0x2cu)
#define HDPATH_1_DEFAULT     (0x80000000u | 0x1d8u)
#define HDPATH_2_DEFAULT     (0x80000000u | 0u)
#define HDPATH_3_DEFAULT     (0u)
#define HDPATH_4_DEFAULT     (0u)

#define RSA_MODULUS_LEN 512     // 4096 key
#define RSA_PRIME_LEN   256     // 4096 key
#define SHA384_DIGEST_LEN 48
#define RSA_MODULUS_HALVE 256

typedef enum {
    kind_unknown
} address_kind_e;

#define COIN_AMOUNT_DECIMAL_PLACES          12
#define COIN_DEFAULT_DENOM_REPR             "AR "
#define COIN_SUPPORTED_TX_VERSION           0

#define MENU_MAIN_APP_LINE1                 "Arweave"
#define MENU_MAIN_APP_LINE2                 "Ready"
#define MENU_MAIN_APP_LINE2_SECRET          "???"
#define APPVERSION_LINE1                    "Version"
#define APPVERSION_LINE2                    "v" APPVERSION

#define COIN_SECRET_REQUIRED_CLICKS         0

#define INS_GET_SIG                     0x10
#define INS_GET_PK                      0x20

#ifdef __cplusplus
}
#endif
