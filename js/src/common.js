export const CLA = 0x44;
export const CHUNK_SIZE = 250;

export const INS = {
  GET_VERSION: 0x00,
  GET_PUBKEY: 0x01,
  SIGN: 0x02,
  GET_SIG_P1: 0x10,
  GET_SIG_P2: 0x11,

  GET_PK_P1: 0x20,
  GET_PK_P2: 0x21,
};

export const PAYLOAD_TYPE = {
  INIT: 0x00,
  ADD: 0x01,
  LAST: 0x02,
};

export const P1_VALUES = {
  ONLY_RETRIEVE: 0x00,
  SHOW_ADDRESS_IN_DEVICE: 0x01,
};

export const ERROR_CODE = {
  NoError: 0x9000,
};

export const PKLEN = 65;

const ERROR_DESCRIPTION = {
  1: "U2F: Unknown",
  2: "U2F: Bad request",
  3: "U2F: Configuration unsupported",
  4: "U2F: Device Ineligible",
  5: "U2F: Timeout",
  14: "Timeout",
  0x9000: "No errors",
  0x9001: "Device is busy",
  0x6802: "Error deriving keys",
  0x6400: "Execution Error",
  0x6700: "Wrong Length",
  0x6982: "Empty Buffer",
  0x6983: "Output buffer too small",
  0x6984: "Data is invalid",
  0x6985: "Conditions not satisfied",
  0x6986: "Transaction rejected",
  0x6a80: "Bad key handle",
  0x6b00: "Invalid P1/P2",
  0x6d00: "Instruction not supported",
  0x6e00: "App does not seem to be open",
  0x6f00: "Unknown error",
  0x6f01: "Sign/verify error",
};

export function errorCodeToString(statusCode) {
  if (statusCode in ERROR_DESCRIPTION) return ERROR_DESCRIPTION[statusCode];
  return `Unknown Status Code: ${statusCode}`;
}

function isDict(v) {
  return typeof v === "object" && v !== null && !(v instanceof Array) && !(v instanceof Date);
}

function printBIP44Item(v) {
  let hardened = v >= 0x8000000;
  return `${v & 0x7FFFFFFF}${hardened ? "'" : ""}`;
}

export function printBIP44Path(pathBytes) {
  if (pathBytes.length !== 20) {
    throw new Error("Invalid bip44 path");
  }

  let pathValues = [0, 0, 0, 0, 0];
  for (let i = 0; i < 5; i += 1) {
    pathValues[i] = pathBytes.readUInt32LE(4 * i);
    console.log(pathValues[i]);
  }

  return `m/${
    printBIP44Item(pathValues[0])
  }/${
    printBIP44Item(pathValues[1])
  }/${
    printBIP44Item(pathValues[2])
  }/${
    printBIP44Item(pathValues[3])
  }/${
    printBIP44Item(pathValues[4])
  }`;
}

export function processErrorResponse(response) {
  if (response) {
    if (isDict(response)) {
      if (Object.prototype.hasOwnProperty.call(response, "statusCode")) {
        return {
          returnCode: response.statusCode,
          errorMessage: errorCodeToString(response.statusCode),
        };
      }

      if (
        Object.prototype.hasOwnProperty.call(response, "returnCode") &&
        Object.prototype.hasOwnProperty.call(response, "errorMessage")
      ) {
        return response;
      }
    }
    return {
      returnCode: 0xffff,
      errorMessage: response.toString(),
    };
  }

  return {
    returnCode: 0xffff,
    errorMessage: response.toString(),
  };
}

export async function getVersion(transport) {
  return transport.send(CLA, INS.GET_VERSION, 0, 0).then((response) => {
    const errorCodeData = response.slice(-2);
    const returnCode = errorCodeData[0] * 256 + errorCodeData[1];

    let targetId = 0;
    if (response.length >= 9) {
      /* eslint-disable no-bitwise */
      targetId = (response[5] << 24) + (response[6] << 16) + (response[7] << 8) + (response[8] << 0);
      /* eslint-enable no-bitwise */
    }

    return {
      returnCode,
      errorMessage: errorCodeToString(returnCode),
      // ///
      testMode: response[0] !== 0,
      major: response[1],
      minor: response[2],
      patch: response[3],
      deviceLocked: response[4] === 1,
      targetId: targetId.toString(16),
    };
  }, processErrorResponse);
}

const HARDENED = 0x80000000;

export function serializePathv1(path) {
  if (typeof path !== "string") {
    throw new Error("Path should be a string (e.g \"m/44'/1'/5'/0/3\")");
  }

  if (!path.startsWith("m")) {
    throw new Error('Path should start with "m" (e.g "m/44\'/472\'/5\'/0/3")');
  }

  const pathArray = path.split("/");

  if (pathArray.length !== 6) {
    throw new Error("Invalid path. (e.g \"m/44'/1'/5'/0/3\")");
  }

  const buf = Buffer.alloc(20);

  for (let i = 1; i < pathArray.length; i += 1) {
    let value = 0;
    let child = pathArray[i];
    if (child.endsWith("'")) {
      value += HARDENED;
      child = child.slice(0, -1);
    }

    const childNumber = Number(child);

    if (Number.isNaN(childNumber)) {
      throw new Error(`Invalid path : ${child} is not a number. (e.g "m/44'/1'/5'/0/3")`);
    }

    if (childNumber >= HARDENED) {
      throw new Error("Incorrect child value (bigger or equal to 0x80000000)");
    }

    value += childNumber;

    buf.writeUInt32LE(value, 4 * (i - 1));
  }

  return buf;
}

export async function signSendChunkv1(app, chunkIdx, chunkNum, chunk) {
  let payloadType = PAYLOAD_TYPE.ADD;
  if (chunkIdx === 1) {
    payloadType = PAYLOAD_TYPE.INIT;
  }
  if (chunkIdx === chunkNum) {
    payloadType = PAYLOAD_TYPE.LAST;
  }
  return app.transport
    .send(CLA, INS.SIGN, payloadType, 0, chunk, [0x9000, 0x6984, 0x6a80])
    .then((response) => {
      const errorCodeData = response.slice(-2);
      const returnCode = errorCodeData[0] * 256 + errorCodeData[1];
      let errorMessage = errorCodeToString(returnCode);

      if (returnCode === 0x6a80 || returnCode === 0x6984) {
        errorMessage = `${errorMessage} : ${response.slice(0, response.length - 2).toString("ascii")}`;
      }

      let signature = null;
      if (response.length > 2) {
        signature = response.slice(0, response.length - 2);
      }

      return {
        signature,
        returnCode: returnCode,
        errorMessage: errorMessage,
      };
    }, processErrorResponse);
}
