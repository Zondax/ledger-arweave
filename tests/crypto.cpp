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
#include "hexutils.h"

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

TEST(Parser, parseBlob) {
    uint8_t inBuffer[1000];

    const char *tmp = "0001320200a5a15914a0d0072f6ca228e3fcd621165f2094ed3d276cabae259fcbe42f1b8165b0aba51c33e2ec22d90c40ebf114d9e107db612a5c18a9d4a1a39fb86e1d0e21dedb27d4e0962653f772b7ce19a28fa2eb8eeb27613cc2956471b61a7be70d115e400cf6a7da06ffa9b46e30954c716e1c3a07eb7d0ef2fc22010c02f5fbc8748a94abf9c4a6cdf74d6a0c7f8a1bdc0296b0ad8bf8f6caad34163067296d278d3873a661c7e178fa2c6b6ce0d88f455caafd1facdf2b56043f3e838a7968754d5553fd229772051e4c84f958cf0924f0c7a84144111d0cd7d96f9e19c3f2d84ed4f1a49d4fcda27173fbab0cc031308166d3408b661d8fb4e98e311791caf51b638e2b1d0270bf0068113075ef76542749514dad7f86cc53080385a384a831047d906afd28488c6b46c7c269bf4acbdf1eaa69b05090fe0a99f28933d4611007706470b4b2f880d4eaa211d71fe5eaed8b6c695161f365aee39b50afd701e13e6e3cf1acd8b83f6a1b25c762e16b6ab81eb3f6006c8cfe153f457b8074b6d8ad2fe36e2f4ba4d5233d37806eccc72a04882f2ac26c99966f50c3238d5254a1b3c9a1043e60e17f7769871479633b4c2f305f6b24a1f21385f8d38740e90e1de9979a4db469c684a996ddb33f0e877048f1183b74106d59866160a622e8e36e593d34cfffb8db5f9ab348a6b349d1ed3e42a27736e49843c1c3fd7c2e5a23570020d6c7916a79252d4ff55531a4124ecfd310303097c0ee8c00d491d6e4ac992a56000e313035303030303030303030303000053132333435003003d21ce913ce097ae95353b34856a1dc67f252b0a7165019e773633c68e40a354381b43a4f5494f00ac86167af7766a3000400084170702d4e616d650010536d6172745765617665416374696f6e000b4170702d56657273696f6e0005302e332e300008436f6e7472616374002b366554567238494b504e59624d48566370484658722d584e614c356854367a524a58696d63502d6f776d6f0005496e707574005a7b2266756e6374696f6e223a227472616e73666572222c22746172676574223a22682d42677231334f57554f6b524757726e4d54304c75554b664a68527373357066546478486d4e63587977222c22717479223a31353030307d000238380020190ba7ce66f0936fc93d4ee824e98bad332f8fcbffec125a174c1eca3567e4d7";
    auto inBufferLen = parseHexString(inBuffer, sizeof(inBuffer), tmp);

    parser_context_t ctx;
    auto err = parser_parse(&ctx, inBuffer, inBufferLen);
    EXPECT_EQ(err, parser_ok);

    err = parser_validate(&ctx);
    EXPECT_EQ(err, parser_ok);
}
