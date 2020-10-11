/** ******************************************************************************
 *  (c) 2020 Zondax GmbH
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
 ******************************************************************************* */

import Arweave from 'arweave';

const arweave = Arweave.init({
    host: 'arweave.net',// Hostname or IP address for a Arweave host
    port: 443,          // Port
    protocol: 'https',  // Network protocol http or https
    timeout: 20000,     // Network request timeouts in milliseconds
    logging: false,     // Enable network request logging
});

let jwtKey = await arweave.wallets.generate();

let transaction = await arweave.createTransaction({
    target: '1seRanklLU_1VTGkEk7P0xAwMJfA7owA1JHW5KyZKlY',
    quantity: arweave.ar.arToWinston('10.5'),
    reward: 12345,
    last_tx: 'A9Ic6RPOCXrpU1OzSFah3GfyUrCnFlAZ53NjPGjkCjVDgbQ6T1SU8ArIYWevd2aj',
    data: '<html><head><meta charset="UTF-8"><title>Hello world!</title></head><body></body></html>'
}, jwtKey)

transaction.addTag('Content-Type', 'text/html');
transaction.addTag('key2', 'value2');

// Force data_root calculation
let signatureData = await transaction.getSignatureData()

function encodeu16(u16Value) {
    return new Uint8Array([
        u16Value >> 8,
        u16Value & 0xFF
    ]);
}

function concatArray(a, b) {
    let tmp = new Uint8Array(a.length + b.length)
    tmp.set(a)
    tmp.set(b, a.length);
    return tmp;
}

function encodeWithLen(input) {
    return concatArray(encodeu16(input.length), input)
}

function encodeTx(tx) {
    let encodedTags = [];
    for (let i = 0; i < tx.tags.length; i += 1) {
        let tag = tx.tags[i];
        encodedTags.push(
            [
                encodeWithLen(tag.get("name", {decode: true, string: false})),
                encodeWithLen(tag.get("value", {decode: true, string: false}))
            ]
        );
    }

    let tmp = [
        encodeWithLen(Arweave.utils.stringToBuffer(tx.format.toString())),
        encodeWithLen(tx.get("owner", {decode: true, string: false})),
        encodeWithLen(tx.get("target", {decode: true, string: false})),
        encodeWithLen(Arweave.utils.stringToBuffer(tx.quantity.toString())),
        encodeWithLen(Arweave.utils.stringToBuffer(tx.reward.toString())),
        encodeWithLen(tx.get("last_tx", {decode: true, string: false})),

        // TODO: encode tags
        encodeu16(tx.tags.length), //tagList = tagCount .. TagName/TagValue,
        encodedTags,

        // TODO: encode data
        encodeWithLen(Arweave.utils.stringToBuffer(tx.data_size.toString())),
        encodeWithLen(tx.get("data_root", {decode: true, string: false})),
    ];

    console.log(tmp)

    let blob = new Uint8Array(0)
    for (let i = 0; i < tmp.length; i++) {
        blob = concatArray(blob, tmp[i]);
    }
    return blob;
}

console.log(transaction);

let encodedTx = encodeTx(transaction);
console.log(encodedTx)
console.log(`Digest to sign: ${signatureData.length}  ` + Buffer.from(signatureData).toString("hex"));

// await arweave.transactions.sign(transaction, jwtKey);
