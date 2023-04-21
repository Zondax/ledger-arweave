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

#include <stdio.h>
#include <zxmacros.h>
#include <zxformat.h>
#include "parser_impl.h"
#include "parser.h"
#include "parser_txdef.h"
#include "coin.h"
#include "b64url.h"
#include "crypto_helper.h"

parser_error_t parser_parse(parser_context_t *ctx, const uint8_t *data, size_t dataLen) {
    if (dataLen < 1) {
        return parser_no_data;
    }
    CHECK_PARSER_ERR(parser_init(ctx, data, dataLen))
    return _read(ctx, &parser_tx_obj);
}

parser_error_t parser_validate(const parser_context_t *ctx) {
    CHECK_PARSER_ERR(_validateTx(ctx, &parser_tx_obj))
    // Iterate through all items to check that all can be shown and are valid
    uint8_t numItems = 0;
    CHECK_PARSER_ERR(parser_getNumItems(ctx, &numItems));

    char tmpKey[40];
    char tmpVal[40];

    for (uint8_t idx = 0; idx < numItems; idx++) {
        uint8_t pageCount = 0;
        CHECK_PARSER_ERR(parser_getItem(ctx, idx, tmpKey, sizeof(tmpKey), tmpVal, sizeof(tmpVal), 0, &pageCount))
    }

    return parser_ok;
}

typedef union {
    struct {
        uint8_t tagHash[48];
        uint8_t dataHash[48];
    };
    uint8_t blob[96];
} taggedHash_t;

typedef union {
    struct {
        uint8_t acc[48];
        uint8_t hash[48];
    };
    uint8_t pair[96];
} deepHash_t;

void hashTag(uint8_t hashResult[48], const char *tagName, uint16_t tagNumber) {
    uint8_t tagBuffer[50];
    MEMZERO(tagBuffer, sizeof(tagBuffer));
    snprintf((char *) tagBuffer, sizeof(tagBuffer), "%s%d", tagName, tagNumber);
    crypto_sha384(tagBuffer, strlen((char *) tagBuffer), hashResult, 48);
}

void hashBlob(uint8_t hashResult[48], const uint8_t *data, uint16_t dataLen) {
    taggedHash_t ctx;
    MEMZERO(&ctx, sizeof(taggedHash_t));

    hashTag(ctx.tagHash, "blob", dataLen);
    crypto_sha384(data, dataLen, ctx.dataHash, 48);
    crypto_sha384(ctx.blob, sizeof(taggedHash_t), hashResult, 48);
}

void hashAccumulate(deepHash_t *dh) {
    uint8_t temp[48];
    crypto_sha384(dh->pair, 96, temp, 48);
    MEMCPY(dh->acc, temp, 48);
}

void hashChunkWithAccumulator(deepHash_t *dh, const uint8_t *data, uint16_t dataLen) {
    hashBlob(dh->hash, data, dataLen);
    hashAccumulate(dh);
}

parser_error_t parser_getTagDigest(uint8_t *digest, uint16_t digestLen, parser_tag_t *tagObject) {
    MEMZERO(digest, digestLen);
    deepHash_t ctx;
    MEMZERO(&ctx, sizeof(taggedHash_t));
    // Bootstrap
    hashTag(ctx.acc, "list", 2);
    hashChunkWithAccumulator(&ctx, tagObject->key.ptr, tagObject->key.len);
    hashChunkWithAccumulator(&ctx, tagObject->value.ptr, tagObject->value.len);
    MEMCPY(digest, ctx.acc, 48);
    return parser_ok;
}

void parser_applyDigestTags(deepHash_t *dh) {
    deepHash_t ctx;
    MEMZERO(&ctx, sizeof(taggedHash_t));

    // Bootstrap
    hashTag(ctx.acc, "list", parser_tx_obj.tags_count);

    // Accumulate tag digests
    for (int i = 0; i < parser_tx_obj.tags_count; i++) {
        parser_getTagDigest(ctx.hash, 48, &parser_tx_obj.tags[i]);
        hashAccumulate(&ctx);
    }

    // Now combine and return
    MEMCPY(dh->hash, ctx.acc, 48);
    hashAccumulate(dh);
}

parser_error_t parser_getDigest(uint8_t *digest, uint16_t digestLen) {
    if (digestLen < SHA384_DIGEST_LEN) {
        return parser_unexpected_buffer_end;
    }
    MEMZERO(digest, digestLen);
    deepHash_t ctx;
    MEMZERO(&ctx, sizeof(taggedHash_t));

    // Bootstrap
    hashTag(ctx.acc, "list", 9);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.format.ptr, parser_tx_obj.format.len);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.owner.ptr, parser_tx_obj.owner.len);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.target.ptr, parser_tx_obj.target.len);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.quantity.ptr, parser_tx_obj.quantity.len);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.reward.ptr, parser_tx_obj.reward.len);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.last_tx.ptr, parser_tx_obj.last_tx.len);

    // Add tags digest
    parser_applyDigestTags(&ctx);

    hashChunkWithAccumulator(&ctx, parser_tx_obj.data_size.ptr, parser_tx_obj.data_size.len);
    hashChunkWithAccumulator(&ctx, parser_tx_obj.data_root.ptr, parser_tx_obj.data_root.len);

    MEMCPY(digest, ctx.acc, SHA384_DIGEST_LEN);
    return parser_ok;
}

parser_error_t parser_getNumItems(const parser_context_t *ctx, uint8_t *num_items) {
    *num_items = _getNumItems(ctx, &parser_tx_obj);
    return parser_ok;
}

parser_error_t parser_printTarget(const parser_element_t *v,
                                  char *outVal, uint16_t outValLen,
                                  uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outVal, outValLen);
    char uibuffer[200];
    b64url_encode(uibuffer, sizeof(uibuffer), v->ptr, v->len);

    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);
    return parser_ok;
}

parser_error_t parser_printQuantity(const parser_element_t *c,
                                   uint8_t decimalPlaces,
                                   bool trimTrailingZeros,
                                   const char postfix[],
                                   const char prefix[],
                                   char *outValue, uint16_t outValueLen,
                                   uint8_t pageIdx, uint8_t *pageCount) {
    char bufferUI[200];
    MEMZERO(outValue, outValueLen);
    MEMZERO(bufferUI, sizeof(bufferUI));
    *pageCount = 1;

    MEMCPY(bufferUI, c->ptr, c->len);

    //Format number
    if (intstr_to_fpstr_inplace(bufferUI, sizeof(bufferUI), decimalPlaces) == 0) {
        return parser_unexpected_value;
    }

    if (z_str3join(bufferUI, sizeof(bufferUI), prefix, postfix) != zxerr_ok) {
        return parser_unexpected_buffer_end;
    }

    if(trimTrailingZeros) {
        number_inplace_trimming(bufferUI, 1);
    }

    pageString(outValue, outValueLen, bufferUI, pageIdx, pageCount);

    return parser_ok;
}

parser_error_t parser_printSize(const parser_element_t *v,
                                    char *outVal, uint16_t outValLen,
                                    uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outVal, outValLen);

    char uibuffer[200];
    MEMZERO(uibuffer, sizeof(uibuffer));

    if (v->len > sizeof(uibuffer) - 1) {
        return parser_unexpected_buffer_end;
    }

    MEMCPY(uibuffer, v->ptr, v->len);
    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);
    return parser_ok;
}

parser_error_t parser_printReward(const parser_element_t *v,
                                  char *outVal, uint16_t outValLen,
                                  uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outVal, outValLen);

    char uibuffer[200];
    MEMZERO(uibuffer, sizeof(uibuffer));

    if (v->len > sizeof(uibuffer) - 1) {
        return parser_unexpected_buffer_end;
    }

    MEMCPY(uibuffer, v->ptr, v->len);
    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);

    return parser_ok;
}

parser_error_t parser_printLastTx(const parser_element_t *v,
                                  char *outVal, uint16_t outValLen,
                                  uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outVal, outValLen);
    char uibuffer[200];
    b64url_encode(uibuffer, sizeof(uibuffer), v->ptr, v->len);

    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);
    return parser_ok;
}

parser_error_t parser_printData(const parser_element_t *v,
                                char *outVal, uint16_t outValLen,
                                uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outVal, outValLen);
    char uibuffer[200];
    b64url_encode(uibuffer, sizeof(uibuffer), v->ptr, v->len);

    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);
    return parser_ok;
}

parser_error_t parser_printTag(const parser_tag_t *v,
                               char *outKey, uint16_t outKeyLen,
                               char *outVal, uint16_t outValLen,
                               uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outKey, outKeyLen);
    MEMZERO(outVal, outValLen);

    char uibuffer[200];
    MEMZERO(uibuffer, sizeof(uibuffer));
    MEMCPY(uibuffer, v->key.ptr, v->key.len);
    snprintf(outKey, outKeyLen, "%s", uibuffer);

    MEMZERO(uibuffer, sizeof(uibuffer));
    MEMCPY(uibuffer, v->value.ptr, v->value.len);
    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);

//    b64url_encode(uibuffer, sizeof(uibuffer), v->value.ptr, v->value.len);
//    pageString(outVal, outValLen, uibuffer, pageIdx, pageCount);
    return parser_ok;
}

parser_error_t parser_getItem(const parser_context_t *ctx,
                              uint16_t displayIdx,
                              char *outKey, uint16_t outKeyLen,
                              char *outVal, uint16_t outValLen,
                              uint8_t pageIdx, uint8_t *pageCount) {
    MEMZERO(outKey, outKeyLen);
    MEMZERO(outVal, outValLen);
    snprintf(outKey, outKeyLen, "? %d", displayIdx);
    snprintf(outVal, outValLen, "?");
    *pageCount = 0;

    uint8_t numItems;
    CHECK_PARSER_ERR(parser_getNumItems(ctx, &numItems))
    CHECK_APP_CANARY()

    if (displayIdx < 0 || displayIdx >= numItems) {
        return parser_no_data;
    }
    *pageCount = 1;

    switch (displayIdx) {
        case 0:
            snprintf(outKey, outKeyLen, "Target");
            CHECK_PARSER_ERR(parser_printTarget(&parser_tx_obj.target, outVal, outValLen, pageIdx, pageCount));
            return parser_ok;
        case 1:
            snprintf(outKey, outKeyLen, "Quantity");
             CHECK_PARSER_ERR(parser_printQuantity(&parser_tx_obj.quantity, COIN_AMOUNT_DECIMAL_PLACES, true, "", COIN_DEFAULT_DENOM_REPR,
             outVal, outValLen, pageIdx, pageCount));
            return parser_ok;
        case 2:
            snprintf(outKey, outKeyLen, "Reward");
            CHECK_PARSER_ERR(parser_printReward(&parser_tx_obj.reward, outVal, outValLen, pageIdx, pageCount));
            return parser_ok;
        case 3:
            snprintf(outKey, outKeyLen, "Last tx");
            CHECK_PARSER_ERR(parser_printLastTx(&parser_tx_obj.last_tx, outVal, outValLen, pageIdx, pageCount));
            return parser_ok;
        case 4:
            snprintf(outKey, outKeyLen, "Data size");
            CHECK_PARSER_ERR(parser_printSize(&parser_tx_obj.data_size, outVal, outValLen, pageIdx, pageCount));
            return parser_ok;
    }

    if (displayIdx == 5 && parser_tx_obj.data_root.len != 0) {
        snprintf(outKey, outKeyLen, "Data root");
        CHECK_PARSER_ERR(parser_printData(&parser_tx_obj.data_root, outVal, outValLen, pageIdx, pageCount));
        return parser_ok;
    }

    displayIdx -= CONST_NUM_UI_ITEMS + DATA_SIZE_NUM_UI_ITEMS + (parser_tx_obj.data_root.len != 0 ? DATA_ROOT_NUM_UI_ITEMS : 0);

    if (displayIdx >= MAX_NUMBER_TAGS) {
        return parser_unexpected_value;
    }

    return parser_printTag(&parser_tx_obj.tags[displayIdx],
                            outKey, outKeyLen,
                            outVal, outValLen,
                            pageIdx, pageCount);
}
