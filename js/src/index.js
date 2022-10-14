/** ******************************************************************************
 *  (c) 2019-2020 Zondax GmbH
 *  (c) 2016-2017 Ledger
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

import {
  CHUNK_SIZE,
  CLA,
  ERROR_CODE,
  errorCodeToString,
  getVersion,
  INS,
  P1_VALUES,
  PAYLOAD_TYPE,
  processErrorResponse,
} from "./common";
import Arweave from "arweave";

function processGetRSAResponse(response) {
  let partialResponse = response;

  const errorCodeData = partialResponse.slice(-2);
  const returnCode = errorCodeData[0] * 256 + errorCodeData[1];

  const output = response.slice(0, 256);

  return {
    output,
    returnCode,
    errorMessage: errorCodeToString(returnCode),
  };
}

function processGetAddrResponse(response) {
  let partialResponse = response;

  const errorCodeData = partialResponse.slice(-2);
  const returnCode = errorCodeData[0] * 256 + errorCodeData[1];

  const address = Buffer.from(partialResponse.slice(0, -2)).toString();

  return {
    address,
    returnCode,
    errorMessage: errorCodeToString(returnCode),
  };
}

export default class ArweaveApp {
  static get ErrorCode() {
    return ERROR_CODE;
  }

  constructor(transport) {
    if (!transport) {
      throw new Error("Transport has not been defined");
    }
    this.transport = transport;
  }

  static encodeu16(u16Value) {
    return new Uint8Array([u16Value >> 8, u16Value & 0xff]);
  }

  static concatArray(a, b) {
    let tmp = new Uint8Array(a.length + b.length);
    tmp.set(a);
    tmp.set(b, a.length);
    return tmp;
  }

  static encodeWithLen(input) {
    return ArweaveApp.concatArray(ArweaveApp.encodeu16(input.length), input);
  }

  static flatten(input) {
    let blob = new Uint8Array(0);
    for (let i = 0; i < input.length; i++) {
      blob = ArweaveApp.concatArray(blob, input[i]);
    }
    return blob;
  }

  static encodeTx(tx) {
    let serializedTags = [];
    for (let i = 0; i < tx.tags.length; i++) {
      let currentTag = tx.tags[i];
      console.log(currentTag);

      let encodedKey = ArweaveApp.encodeWithLen(
        currentTag.get("name", { decode: true, string: false })
      );
      let encodedVal = ArweaveApp.encodeWithLen(
        currentTag.get("value", { decode: true, string: false })
      );

      serializedTags.push(encodedKey);
      serializedTags.push(encodedVal);
    }

    let flatSerializedTags = ArweaveApp.flatten(serializedTags);

    let tmp = [
      ArweaveApp.encodeWithLen(
        Arweave.utils.stringToBuffer(tx.format.toString())
      ),
      ArweaveApp.encodeWithLen(
        tx.get("owner", { decode: true, string: false })
      ),
      ArweaveApp.encodeWithLen(
        tx.get("target", { decode: true, string: false })
      ),
      ArweaveApp.encodeWithLen(
        Arweave.utils.stringToBuffer(tx.quantity.toString())
      ),
      ArweaveApp.encodeWithLen(
        Arweave.utils.stringToBuffer(tx.reward.toString())
      ),
      ArweaveApp.encodeWithLen(
        tx.get("last_tx", { decode: true, string: false })
      ),
      ArweaveApp.encodeu16(tx.tags.length),
      flatSerializedTags,
      ArweaveApp.encodeWithLen(
        Arweave.utils.stringToBuffer(tx.data_size.toString())
      ),
      ArweaveApp.encodeWithLen(
        tx.get("data_root", { decode: true, string: false })
      ),
    ];

    let blob = ArweaveApp.flatten(tmp);
    console.log(Buffer.from(blob).toString("hex"));
    return blob;
  }

  static prepareChunks(message) {
    const chunks = [];
    chunks.push(Buffer.alloc(20));
    const messageBuffer = Buffer.from(ArweaveApp.encodeTx(message));

    const buffer = Buffer.concat([messageBuffer]);
    for (let i = 0; i < buffer.length; i += CHUNK_SIZE) {
      let end = i + CHUNK_SIZE;
      if (i > buffer.length) {
        end = buffer.length;
      }
      chunks.push(buffer.slice(i, end));
    }
    return chunks;
  }

  async signGetChunks(message) {
    console.log("signGetChunks");
    return ArweaveApp.prepareChunks(message);
  }

  async getVersion() {
    return getVersion(this.transport)
      .then((response) => {
        return response;
      })
      .catch((err) => processErrorResponse(err));
  }

  async appInfo() {
    return this.transport.send(0xb0, 0x01, 0, 0).then((response) => {
      const errorCodeData = response.slice(-2);
      const returnCode = errorCodeData[0] * 256 + errorCodeData[1];

      const result = {};

      let appName = "err";
      let appVersion = "err";
      let flagLen = 0;
      let flagsValue = 0;

      if (response[0] !== 1) {
        // Ledger responds with format ID 1. There is no spec for any format != 1
        result.errorMessage = "response format ID not recognized";
        result.returnCode = 0x9001;
      } else {
        const appNameLen = response[1];
        appName = response.slice(2, 2 + appNameLen).toString("ascii");
        let idx = 2 + appNameLen;
        const appVersionLen = response[idx];
        idx += 1;
        appVersion = response.slice(idx, idx + appVersionLen).toString("ascii");
        idx += appVersionLen;
        const appFlagsLen = response[idx];
        idx += 1;
        flagLen = appFlagsLen;
        flagsValue = response[idx];
      }

      return {
        returnCode,
        errorMessage: errorCodeToString(returnCode),
        // //
        appName,
        appVersion,
        flagLen,
        flagsValue,
        // eslint-disable-next-line no-bitwise
        flagRecovery: (flagsValue & 1) !== 0,
        // eslint-disable-next-line no-bitwise
        flagSignedMcuCode: (flagsValue & 2) !== 0,
        // eslint-disable-next-line no-bitwise
        flagOnboarded: (flagsValue & 4) !== 0,
        // eslint-disable-next-line no-bitwise
        flagPINValidated: (flagsValue & 128) !== 0,
      };
    }, processErrorResponse);
  }

  async getAddressHash() {
    return this.transport
      .send(CLA, INS.GET_ADDRESS, P1_VALUES.ONLY_RETRIEVE, 0, Buffer.from([]), [
        0x9000,
      ])
      .then(processGetAddrResponse, processErrorResponse);
  }

  async getAddress() {
    let req1 = await this.getAddressHash();

    if (req1.returnCode !== 0x9000) {
      return req1;
    }

    let req2 = await this.getPubKeyPart(0);
    if (req2.returnCode !== 0x9000) {
      return req2;
    }

    let req3 = await this.getPubKeyPart(1);
    if (req3.returnCode !== 0x9000) {
      return req3;
    }

    let pk = Buffer.concat([
      Buffer.from(req2.output),
      Buffer.from(req3.output),
    ]);
    let owner = await Arweave.utils.bufferTob64Url(pk);

    return {
      address: req1.address,
      pubkey: pk,
      owner: owner,
      returnCode: 0x9000,
      errorMessage: errorCodeToString(0x9000),
    };
  }

  async getSignaturePart(partnum) {
    if (partnum == 0) {
      return this.transport
        .send(CLA, INS.GET_SIG, P1_VALUES.ONLY_RETRIEVE, 0, Buffer.from([]), [
          0x9000,
        ])
        .then(processGetRSAResponse, processErrorResponse);
    } else {
      return this.transport
        .send(CLA, INS.GET_SIG, P1_VALUES.ONLY_RETRIEVE, 1, Buffer.from([]), [
          0x9000,
        ])
        .then(processGetRSAResponse, processErrorResponse);
    }
  }

  async getPubKeyPart(partnum) {
    if (partnum == 0) {
      return this.transport
        .send(CLA, INS.GET_PK, P1_VALUES.ONLY_RETRIEVE, 0, Buffer.from([]), [
          0x9000,
        ])
        .then(processGetRSAResponse, processErrorResponse);
    } else {
      return this.transport
        .send(CLA, INS.GET_PK, P1_VALUES.ONLY_RETRIEVE, 1, Buffer.from([]), [
          0x9000,
        ])
        .then(processGetRSAResponse, processErrorResponse);
    }
  }

  async showAddress() {
    return this.transport
      .send(
        CLA,
        INS.GET_ADDRESS,
        P1_VALUES.SHOW_ADDRESS_IN_DEVICE,
        0,
        Buffer.from([]),
        [0x9000]
      )
      .then(processGetAddrResponse, processErrorResponse);
  }

  async signSendChunk(chunkIdx, chunkNum, chunk) {
    let payloadType = PAYLOAD_TYPE.ADD;
    if (chunkIdx === 1) {
      payloadType = PAYLOAD_TYPE.INIT;
    }
    if (chunkIdx === chunkNum) {
      payloadType = PAYLOAD_TYPE.LAST;
    }
    return this.transport
      .send(CLA, INS.SIGN, payloadType, 0, chunk, [0x9000, 0x6984, 0x6a80])
      .then((response) => {
        const errorCodeData = response.slice(-2);
        const returnCode = errorCodeData[0] * 256 + errorCodeData[1];
        let errorMessage = errorCodeToString(returnCode);

        if (returnCode === 0x6a80 || returnCode === 0x6984) {
          errorMessage = `${errorMessage} : ${response
            .slice(0, response.length - 2)
            .toString("ascii")}`;
        }

        let output = null;
        if (response.length > 2) {
          output = response.slice(0, 48);
        }
        console.log(output);
        return {
          output: output,
          returnCode: returnCode,
          errorMessage: errorMessage,
        };
      }, processErrorResponse);
  }

  async sign(message) {
    let req1 = await this.signGetDigest(message);

    if (req1.returnCode !== 0x9000) {
      return req1;
    }

    let req2 = await this.getSignaturePart(0);
    if (req2.returnCode !== 0x9000) {
      return req2;
    }

    let req3 = await this.getSignaturePart(1);
    if (req3.returnCode !== 0x9000) {
      return req3;
    }

    let signature = Buffer.concat([
      Buffer.from(req2.output),
      Buffer.from(req3.output),
    ]);

    return {
      deephash: req1.digest,
      signature: signature,
      returnCode: 0x9000,
      errorMessage: errorCodeToString(0x9000),
    };
  }

  async signGetDigest(message) {
    console.log("sign");

    return this.signGetChunks(message).then((chunks) => {
      return this.signSendChunk(1, chunks.length, chunks[0]).then(
        async (response) => {
          let result = {
            returnCode: response.returnCode,
            errorMessage: response.errorMessage,
            output: null,
          };

          for (let i = 1; i < chunks.length; i += 1) {
            // eslint-disable-next-line no-await-in-loop
            result = await this.signSendChunk(1 + i, chunks.length, chunks[i]);
            if (result.returnCode !== ERROR_CODE.NoError) {
              break;
            }
          }

          console.log(result);

          return {
            returnCode: result.returnCode,
            errorMessage: result.errorMessage,
            // ///
            digest: result.output,
          };
        },
        processErrorResponse
      );
    }, processErrorResponse);
  }

  async digestSendChunk(chunkIdx, chunkNum, chunk) {
    let payloadType = PAYLOAD_TYPE.ADD;
    if (chunkIdx === 1) {
      payloadType = PAYLOAD_TYPE.INIT;
    }
    if (chunkIdx === chunkNum) {
      payloadType = PAYLOAD_TYPE.LAST;
    }
    return this.transport
      .send(
        CLA,
        INS.GET_DIGEST,
        payloadType,
        0,
        chunk,
        [0x9000, 0x6984, 0x6a80]
      )
      .then((response) => {
        const errorCodeData = response.slice(-2);
        const returnCode = errorCodeData[0] * 256 + errorCodeData[1];
        let errorMessage = errorCodeToString(returnCode);

        if (returnCode === 0x6a80 || returnCode === 0x6984) {
          errorMessage = `${errorMessage} : ${response
            .slice(0, response.length - 2)
            .toString("ascii")}`;
        }

        let digest = null;
        if (response.length > 2) {
          digest = response.slice(0, 48);
        }

        return {
          digest: digest,
          returnCode: returnCode,
          errorMessage: errorMessage,
        };
      }, processErrorResponse);
  }

  async digest(message) {
    console.log("digest");

    return this.signGetChunks(message).then((chunks) => {
      return this.digestSendChunk(1, chunks.length, chunks[0]).then(
        async (response) => {
          let result = {
            returnCode: response.returnCode,
            errorMessage: response.errorMessage,
            digest: null,
          };

          for (let i = 1; i < chunks.length; i += 1) {
            // eslint-disable-next-line no-await-in-loop
            result = await this.digestSendChunk(
              1 + i,
              chunks.length,
              chunks[i]
            );
            if (result.returnCode !== ERROR_CODE.NoError) {
              break;
            }
          }

          return {
            digest: result.digest,
            // ///
            returnCode: result.returnCode,
            errorMessage: result.errorMessage,
          };
        },
        processErrorResponse
      );
    }, processErrorResponse);
  }
}
