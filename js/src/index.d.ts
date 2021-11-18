import Transport from "@ledgerhq/hw-transport";
import Transaction from "arweave/web/lib/transaction";

export interface ResponseBase {
  errorMessage: string;
  returnCode: number;
}

export interface ResponseAddress extends ResponseBase {
  address: string;
  owner: string;
}

export interface ResponseVersion extends ResponseBase {
  testMode: boolean;
  major: number;
  minor: number;
  patch: number;
  deviceLocked: boolean;
  targetId: string;
}

export interface ResponseAppInfo extends ResponseBase {
  appName: string;
  appVersion: string;
  flagLen: number;
  flagsValue: number;
  flagRecovery: boolean;
  flagSignedMcuCode: boolean;
  flagOnboarded: boolean;
  flagPINValidated: boolean;
}

export interface ResponseDigest extends ResponseBase {
  digest: Buffer;
}

export interface ResponseSign extends ResponseBase {
  signature: Buffer;
}


export default class ArweaveApp {
  constructor(transport: Transport);

  getVersion(): Promise<ResponseVersion>;
  getAppInfo(): Promise<ResponseAppInfo>;
  getAddress(): Promise<ResponseAddress>;
  showAddress(): Promise<ResponseAddress>;

  digest(message: Buffer): Promise<ResponseDigest>;
  sign(message: Transaction | Buffer): Promise<ResponseSign>;
}
