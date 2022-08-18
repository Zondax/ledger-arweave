/*******************************************************************************
*   (c) 2018, 2019 Zondax GmbH
*   (c) 2016 Ledger
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

#include "app_main.h"

#include <string.h>
#include <os_io_seproxyhal.h>
#include <os.h>
#include <ux.h>

#include "view.h"
#include "actions.h"
#include "tx.h"
#include "addr.h"
#include "crypto.h"
#include "coin.h"
#include "zxmacros.h"

#define REPLY_APDU 0x03
#define CONFIG_UI 0x00

__Z_INLINE void handleGetAddress(volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    *tx = 0;
    if(rx < APDU_MIN_LENGTH){
        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
    }

    if(G_io_apdu_buffer[OFFSET_DATA_LEN] != 0){
        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
    }

    uint8_t requireConfirmation = G_io_apdu_buffer[OFFSET_P1];
    MEMZERO(G_io_apdu_buffer,IO_APDU_BUFFER_SIZE);

    action_addrResponseLen = 0;
    zxerr_t err = app_fill_address();
    if (err != zxerr_ok || action_addrResponseLen == 0) {
        *tx = 0;
        THROW(APDU_CODE_EXECUTION_ERROR);
    }

    if (requireConfirmation) {
        view_review_init(addr_getItem, addr_getNumItems, app_reply_address);
        view_review_show(REPLY_APDU);

        *flags |= IO_ASYNCH_REPLY;
        return;
    }else{
        *tx = action_addrResponseLen;
        THROW(APDU_CODE_OK);
    }
}

__Z_INLINE void handleGetPubKeyPart(__Z_UNUSED volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    *tx = 0;
    if(rx < APDU_MIN_LENGTH){
        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
    }

    if(G_io_apdu_buffer[OFFSET_DATA_LEN] != 0){
        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
    }
    uint8_t index = G_io_apdu_buffer[OFFSET_P2];

    MEMZERO(G_io_apdu_buffer,IO_APDU_BUFFER_SIZE);
    zxerr_t err = crypto_getpubkey_part(G_io_apdu_buffer, IO_APDU_BUFFER_SIZE - 3, index);
    if (err != zxerr_ok){
        *tx = 0;
        THROW(APDU_CODE_CONDITIONS_NOT_SATISFIED);
    }else{
        *tx = 256;
        THROW(APDU_CODE_OK);
    }
}


__Z_INLINE void handleGetSigPart(__Z_UNUSED volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    *tx = 0;
    if(rx < APDU_MIN_LENGTH){
        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
    }

    if(G_io_apdu_buffer[OFFSET_DATA_LEN] != 0){
        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
    }

    uint8_t index = G_io_apdu_buffer[OFFSET_P2];

    MEMZERO(G_io_apdu_buffer,IO_APDU_BUFFER_SIZE);
    zxerr_t err = crypto_getsignature_part(G_io_apdu_buffer, IO_APDU_BUFFER_SIZE - 3, index);
    if (err != zxerr_ok){
        *tx = 0;
        THROW(APDU_CODE_CONDITIONS_NOT_SATISFIED);
    }else{
        *tx = 256;
        THROW(APDU_CODE_OK);
    }
}


__Z_INLINE void handleSign(volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    if (!process_chunk(tx, rx)) {
        THROW(APDU_CODE_OK);
    }

    zemu_log_stack("handleSign");

    const char *error_msg = tx_parse();
    if (error_msg != NULL) {
        int error_msg_length = strlen(error_msg);
        MEMCPY(G_io_apdu_buffer, error_msg, error_msg_length);
        *tx += (error_msg_length);
        THROW(APDU_CODE_DATA_INVALID);
    }

    CHECK_APP_CANARY()
    view_review_init(tx_getItem, tx_getNumItems, app_sign);
    view_review_show(REPLY_APDU);
    *flags |= IO_ASYNCH_REPLY;
}

void handleApdu(volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    uint16_t sw = 0;

    BEGIN_TRY
    {
        TRY
        {
            if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                THROW(APDU_CODE_CLA_NOT_SUPPORTED);
            }

            if (rx < APDU_MIN_LENGTH) {
                THROW(APDU_CODE_WRONG_LENGTH);
            }

            switch (G_io_apdu_buffer[OFFSET_INS]) {
                case INS_GET_VERSION: {
                    handle_getversion(flags, tx, rx);
                    break;
                }

                case INS_GET_ADDRESS: {
                    if( os_global_pin_is_validated() != BOLOS_UX_OK ) {
                        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
                    }
                    handleGetAddress(flags, tx, rx);
                    break;
                }

                case INS_SIGN: {
                    if( os_global_pin_is_validated() != BOLOS_UX_OK ) {
                        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
                    }
                    handleSign(flags, tx, rx);
                    break;
                }

                case INS_GET_SIG: {
                    if( os_global_pin_is_validated() != BOLOS_UX_OK ) {
                        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
                    }
                    handleGetSigPart(flags, tx, rx);
                    break;
                }

                case INS_GET_PK: {
                    if( os_global_pin_is_validated() != BOLOS_UX_OK ) {
                        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
                    }
                    handleGetPubKeyPart(flags, tx, rx);
                    break;
                }

                default:
                    THROW(APDU_CODE_INS_NOT_SUPPORTED);
            }
        }
        CATCH(EXCEPTION_IO_RESET)
        {
            THROW(EXCEPTION_IO_RESET);
        }
        CATCH_OTHER(e)
        {
            switch (e & 0xF000) {
                case 0x6000:
                case APDU_CODE_OK:
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
            }
            G_io_apdu_buffer[*tx] = sw >> 8;
            G_io_apdu_buffer[*tx + 1] = sw;
            *tx += 2;
        }
        FINALLY
        {
        }
    }
    END_TRY;
}
