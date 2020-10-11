/*******************************************************************************
*  (c) 2019 Zondax GmbH
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

#include <coin.h>
#include <zxtypes.h>
#include "parser_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "crypto.h"
#include "coin_script_hashes.h"

#define MAX_NUMBER_TAGS 16

typedef struct {
    uint16_t len;
    const uint8_t *ptr;
} parser_element_t;

typedef struct {
    parser_element_t key;
    parser_element_t value;
} parser_tag_t;

typedef struct {
    parser_element_t format;        // must be 2
    parser_element_t owner;
    parser_element_t target;
    parser_element_t quantity;
    parser_element_t reward;
    parser_element_t last_tx;

    // tags
    uint16_t tags_count;
    parser_tag_t tags[MAX_NUMBER_TAGS];

    parser_element_t data_size;
    parser_element_t data_root;
} parser_tx_t;

#ifdef __cplusplus
}
#endif
