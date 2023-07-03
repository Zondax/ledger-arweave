/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018, 2019 Zondax GmbH
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
#include "crypto_store.h"
#include "view.h"

#include <os_io_seproxyhal.h>

extern bool device_initialized;

__attribute__((section(".boot"))) int
main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    view_init();
    os_boot();

    BEGIN_TRY
    {
        TRY
        {
            app_init();
            device_initialized = crypto_store_init_test();
            if (!device_initialized) {
                view_initialize_init(crypto_store_init);
                view_initialize_show(0, NULL);
            } else {
                view_idle_show(0,NULL);
            }
            app_main();
        }
        CATCH_OTHER(e)
        {}
        FINALLY
        {}
    }
    END_TRY;
}
