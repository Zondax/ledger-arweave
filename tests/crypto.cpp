/*******************************************************************************
*   (c) 2020 ZondaX GmbH
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

#include <gmock/gmock.h>
#include <fmt/core.h>
#include <zxmacros.h>
#include <zxformat.h>
#include "parser.h"

extern "C" {
#include "sha512.h"
}

TEST(SHA384, API_Check) {
    uint8_t input[] = {'A', 'B', 'C', 'a', 'b', 'c'};
    uint8_t messageDigest[64];

    SHA384(input, sizeof(input), messageDigest);

    char s[100];
    array_to_hexstr(s, sizeof(s), messageDigest, 48);
    std::cout << s << std::endl;

    EXPECT_STREQ(s, "f15ef1fd005dbb68209eeae4de4e13116fb652e4841b03b8820e6a423a2225f6849253340611a3ad9301aab3718dee7a");
}

TEST(SHA384, API_Check2) {
    uint8_t input[] = {2};
    uint8_t messageDigest[64];

    crypto_sha384(input, sizeof(input), messageDigest, sizeof(messageDigest));

    char s[100];
    array_to_hexstr(s, sizeof(s), messageDigest, 48);
    std::cout << s << std::endl;

    EXPECT_STREQ(s, "db475240477c5e4497a2c5724ca485f5b1f2c2fc0602b92bae234238ec8d4e873a7148c739593e95a7f4dfe7c6e69f69");
}

TEST(SHA384, hashBlob) {
    uint8_t input[] = {'2'};
    uint8_t messageDigest[64];

    hashBlob(messageDigest, input, sizeof(input));

    char s[100];
    array_to_hexstr(s, sizeof(s), messageDigest, 48);
    std::cout << s << std::endl;

    EXPECT_STREQ(s, "8e876ef8a0878b73ee3285bd34ad7d6c86fae855658d125a6eb4dbf572b3fed84bf236a7c8d1d5c78f853c908b02d8f9");
}
