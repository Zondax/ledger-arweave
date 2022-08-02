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

#include <zxmacros.h>
#include "parser_impl.h"
#include "parser_txdef.h"
#include "app_mode.h"
#include "crypto.h"

parser_tx_t parser_tx_obj;

#define CHECK_KIND(KIND, EXPECTED_KIND) \
    if (KIND != EXPECTED_KIND) { return parser_rlp_error_invalid_kind; }

parser_error_t parser_init_context(parser_context_t *ctx,
                                   const uint8_t *buffer,
                                   uint16_t bufferSize) {
    ctx->offset = 0;
    ctx->buffer = NULL;
    ctx->bufferLen = 0;

    if (bufferSize == 0 || buffer == NULL) {
        // Not available, use defaults
        return parser_init_context_empty;
    }

    ctx->buffer = buffer;
    ctx->bufferLen = bufferSize;
    return parser_ok;
}

parser_error_t parser_init(parser_context_t *ctx, const uint8_t *buffer, uint16_t bufferSize) {
    CHECK_PARSER_ERR(parser_init_context(ctx, buffer, bufferSize))
    return parser_ok;
}

const char *parser_getErrorDescription(parser_error_t err) {
    switch (err) {
        // General errors
        case parser_ok:
            return "No error";
        case parser_no_data:
            return "No more data";
        case parser_init_context_empty:
            return "Initialized empty context";
        case parser_display_idx_out_of_range:
            return "display_idx_out_of_range";
        case parser_display_page_out_of_range:
            return "display_page_out_of_range";
        case parser_unexpected_error:
            return "Unexepected internal error";
            // Coin specific
        case parser_unexpected_tx_version:
            return "tx version is not supported";
        case parser_unexpected_type:
            return "Unexpected data type";
        case parser_unexpected_buffer_end:
            return "Unexpected buffer end";
        case parser_unexpected_value:
            return "Unexpected value";
        case parser_unexpected_number_items:
            return "Unexpected number of items";
        case parser_unexpected_characters:
            return "Unexpected characters";
        case parser_unexpected_field:
            return "Unexpected field";
        case parser_value_out_of_range:
            return "Value out of range";
        case parser_invalid_address:
            return "Invalid address format";
            /////////// Context specific
        case parser_context_mismatch:
            return "context prefix is invalid";
        case parser_context_unexpected_size:
            return "context unexpected size";
        case parser_context_invalid_chars:
            return "context invalid chars";
            // Required fields error
        case parser_required_nonce:
            return "Required field nonce";
        case parser_required_method:
            return "Required field method";
        default:
            return "Unrecognized error code";
    }
}

parser_error_t parser_readU16(parser_context_t *c, uint16_t *v) {
    CTX_CHECK_AVAIL(c, 2);
    *v = c->buffer[c->offset] * 256;
    *v += c->buffer[c->offset + 1];
    CTX_CHECK_AND_ADVANCE(c, 2)
    return parser_ok;
}

parser_error_t parser_readElement(parser_context_t *c, parser_element_t *v) {
    CHECK_PARSER_ERR(parser_readU16(c, &v->len))
    CTX_CHECK_AVAIL(c, v->len)
    v->ptr = c->buffer + c->offset;
    CTX_CHECK_AND_ADVANCE(c, v->len)
    return parser_ok;
}

parser_error_t parser_readTag(parser_context_t *c, parser_tag_t *v) {
    CHECK_PARSER_ERR(parser_readElement(c, &v->key))
    CHECK_PARSER_ERR(parser_readElement(c, &v->value))
    return parser_ok;
}


parser_error_t _read(parser_context_t *c, parser_tx_t *v) {
    CHECK_PARSER_ERR(parser_readElement(c, &v->format))
    if (v->format.len != 1 || v->format.ptr[0] != '2') {
        return parser_unexpected_tx_version;
    }

    CHECK_PARSER_ERR(parser_readElement(c, &v->owner))
    if (v->owner.len != 512) {
        return parser_unexpected_tx_version;
    }
    CHECK_PARSER_ERR(parser_readElement(c, &v->target))
    CHECK_PARSER_ERR(parser_readElement(c, &v->quantity))
    CHECK_PARSER_ERR(parser_readElement(c, &v->reward))
    CHECK_PARSER_ERR(parser_readElement(c, &v->last_tx))

    // tags
    CHECK_PARSER_ERR(parser_readU16(c, &v->tags_count))
    if (v->tags_count > MAX_NUMBER_TAGS) {
        return parser_value_out_of_range;
    }
    for (int i = 0; i < v->tags_count; i++) {
        CHECK_PARSER_ERR(parser_readTag(c, &v->tags[i]))
    }

    CHECK_PARSER_ERR(parser_readElement(c, &v->data_size))
    CHECK_PARSER_ERR(parser_readElement(c, &v->data_root))
    return parser_ok;
}

#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2)
parser_error_t _validateTx(__Z_UNUSED const parser_context_t *c, const parser_tx_t *v) {
    zxerr_t zxerr;
    uint8_t rsakey[RSA_MODULUS_HALVE];
    for(int i = 0; i < 2; i ++) {
        MEMZERO(rsakey, RSA_MODULUS_HALVE);
        zxerr = crypto_getpubkey_part(rsakey, RSA_MODULUS_HALVE, i);
        if (zxerr != zxerr_ok) {
            return parser_unexpected_error;
        }
        if (MEMCMP(v->owner.ptr + i *RSA_MODULUS_HALVE, rsakey, RSA_MODULUS_HALVE) != 0) {
            return parser_unexpected_error;
        }
    }
    return parser_ok;
}
#else
parser_error_t _validateTx(__Z_UNUSED const parser_context_t *c, const parser_tx_t *v) {
    //do nothing
    return parser_ok;
}
#endif

uint8_t _getNumItems(__Z_UNUSED const parser_context_t *c, const parser_tx_t *v) {
    if(!app_mode_expert())
        return (CONST_NUM_UI_ITEMS-2);

    return CONST_NUM_UI_ITEMS + v->tags_count;
}
