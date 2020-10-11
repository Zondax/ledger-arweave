# @zondax/ledger-arweave

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![npm version](https://badge.fury.io/js/%40zondax%2Fledger-arweave.svg)](https://badge.fury.io/js/%40zondax%2Fledger-arweave)

This package provides a basic client library to communicate with the Arweave App running in a Ledger Nano S/X

We recommend using the npmjs package in order to receive updates/fixes.

## Notes

Use `yarn install` to avoid issues.

## Minimal example

This is very simple. First you need to use one of the transport classes provided by Ledger.

```js
import TransportWebUSB from "@ledgerhq/hw-transport-webusb";
transport = await TransportWebUSB.create();
const app = new ArweaveApp(transport);
```

the `ArweaveApp` object will provide

```js
const response = await app.getVersion();

if (response.returnCode !== ArweaveApp.ERROR_CODE.NoError) {
      console.log(`Error [${response.returnCode}] ${response.errorMessage}`);
} else {
    console.log(response);
}
```
