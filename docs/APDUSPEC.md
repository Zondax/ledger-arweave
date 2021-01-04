# Arweave App

## General structure

The general structure of commands and responses is as follows:

### Commands

| Field   | Type     | Content                | Note |
| :------ | :------- | :--------------------- | ---- |
| CLA     | byte (1) | Application Identifier | 0x44 |
| INS     | byte (1) | Instruction ID         |      |
| P1      | byte (1) | Parameter 1            |      |
| P2      | byte (1) | Parameter 2            |      |
| L       | byte (1) | Bytes in payload       |      |
| PAYLOAD | byte (L) | Payload                |      |

### Response

| Field   | Type     | Content     | Note                     |
| ------- | -------- | ----------- | ------------------------ |
| ANSWER  | byte (?) | Answer      | depends on the command   |
| SW1-SW2 | byte (2) | Return code | see list of return codes |

---

## Command definition

### GET_VERSION

#### Command

| Field | Type     | Content                | Expected |
| ----- | -------- | ---------------------- | -------- |
| CLA   | byte (1) | Application Identifier | 0x44     |
| INS   | byte (1) | Instruction ID         | 0x00     |
| P1    | byte (1) | Parameter 1            | ignored  |
| P2    | byte (1) | Parameter 2            | ignored  |
| L     | byte (1) | Bytes in payload       | 0        |

#### Response

| Field   | Type     | Content          | Note                            |
| ------- | -------- | ---------------- | ------------------------------- |
| TEST    | byte (1) | Test Mode        | 0xFF means test mode is enabled |
| MAJOR   | byte (1) | Version Major    |                                 |
| MINOR   | byte (1) | Version Minor    |                                 |
| PATCH   | byte (1) | Version Patch    |                                 |
| LOCKED  | byte (1) | Device is locked |                                 |
| SW1-SW2 | byte (2) | Return code      | see list of return codes        |

---

### GET_ADDRESS

#### Command

| Field   | Type     | Content                   | Expected           |
| ------- | -------- | ------------------------- | ------------------ |
| CLA     | byte (1) | Application Identifier    | 0x44               |
| INS     | byte (1) | Instruction ID            | 0x01               |
| P1      | byte (1) | Request User confirmation | No = 0             |
| P2      | byte (1) | Parameter 2               | Ignored        |
| L       | byte (1) | Bytes in payload          | expected = 0       |

#### Response

| Field      | Type      | Content           | Note                     |
| ---------- | --------- | ----------------- | ------------------------ |
| ADDR       | byte (43) | Address           | Encoded as B64URL        |
| SW1-SW2    | byte (2)  | Return code       | see list of return codes |

---

### SIGN

This command shows the transaction content on screen and signs (if accepted) the transaction.
The RSA signature is stored in secure flash and should be retrieved using GET_SIG commands.

#### Command

| Field | Type     | Content                | Expected  |
| ----- | -------- | ---------------------- | --------- |
| CLA   | byte (1) | Application Identifier | 0x44      |
| INS   | byte (1) | Instruction ID         | 0x02      |
| P1    | byte (1) | Payload desc           | 0 = init  |
|       |          |                        | 1 = add   |
|       |          |                        | 2 = last  |
| P2    | byte (1) | ----                   | not used  |
| L     | byte (1) | Bytes in payload       | (depends) |

The first packet/chunk is empty (there is no derivation path)

All other packets/chunks contain data chunks that are described below

##### First Packet

*empty*

##### Other Chunks/Packets

| Field | Type     | Content | Expected |
| ----- | -------- | ------- | -------- |
| Data  | bytes... | Message |          |

Data is defined as:

| Field   | Type    | Content    | Expected |
| ------- | ------- | ---------- | -------- |
| Message | bytes.. | tx to sign |          |

| Field            | Type    | Content                         | Expected    |
| ---------------- | ------- | ------------------------------- | ----------- |
| format_len       | 2 bytes |                                 |             |
| format           | ? bytes | ASCII string                    |             |
| owner_len        | 2 bytes |                                 |             |
| owner            | ? bytes | Bytes - Show as b64Url          | 512 bytes?? |
| target_len       | 2 bytes |                                 |             |
| target           | ? bytes | Bytes - Show as b64Url          |             |
| quantity_len     | 2 bytes |                                 |             |
| quantity         | ? bytes | ASCII string                    |             |
| reward_len       | 2 bytes |                                 |             |
| reward           | ? bytes | ASCII string                    |             |
| last_tx_len      | 2 bytes |                                 |             |
| last_tx          | ? bytes |                                 |             |
| .                |         |                                 |             |
| tag_count        | 2 bytes |                                 |             |
| .                |         | will repeat multiple times      |             |
| -- tag_name_len  | 2 bytes |                                 |             |
| -- tag_name      | ? bytes |                                 |             |
| -- tag_value_len | 2 bytes |                                 |             |
| -- tag_value     | ? bytes |                                 |             |
| .                |         |                                 |             |
| data_size_len    | 2 bytes |                                 |             |
| data_size        | ? bytes | Size encoded as an ASCII number |             |
| data             | ? bytes |                                 |             |

#### Response

| Field       | Type            | Content     | Note                            |
| ----------- | --------------- | ----------- | ------------------------       |  
| Hash         | byte (48)      | Deephash    |  Deephash of tx blob   |
| SW1-SW2     | byte (2)        | Return code | see list of return codes       |

### GET_SIG

This command can be taken to get the RSA signature of the ledger out. 
It requires to have a successful SIGN command (see above) to have happened before.
Only index = 0 or index = 1 are allowed, that return the first and second part of the RSA-4096 signature.
After index = 1 is retrieved the signature in flash is zeroized.

#### Command

| Field   | Type     | Content                   | Expected           |
| ------- | -------- | ------------------------- | ------------------ |
| CLA     | byte (1) | Application Identifier    | 0x44               |
| INS     | byte (1) | Instruction ID            | 0x10               |
| P1      | byte (1) | Request User confirmation | Ignored           |
| P2      | byte (1) | Parameter 2               | Index = 0 or 1       |
| L       | byte (1) | Bytes in payload          | expected = 0       |

#### Response

| Field      | Type      | Content           | Note                     |
| ---------- | --------- | ----------------- | ------------------------ |
| SIG_PART       | byte (256) | Public key part | Signature bytes [256 * i ... 256 *(i+1)] |
| SW1-SW2    | byte (2)  | Return code       | see list of return codes |

---

### GET_PK

This command can be taken to get the "owner" (the RSA public key) of the ledger out.
Only index = 0 or index = 1 are allowed, that return the first and second part of the RSA-4096 public key.

#### Command

| Field   | Type     | Content                   | Expected           |
| ------- | -------- | ------------------------- | ------------------ |
| CLA     | byte (1) | Application Identifier    | 0x44               |
| INS     | byte (1) | Instruction ID            | 0x20               |
| P1      | byte (1) | Request User confirmation | Ignored             |
| P2      | byte (1) | Parameter 2               | Index = 0 or 1     |
| L       | byte (1) | Bytes in payload          | expected = 0       |

#### Response

| Field      | Type      | Content           | Note                     |
| ---------- | --------- | ----------------- | ------------------------ |
| PK_PART       | byte (256) | Public key part | Pubkey bytes [256 * i ... 256 *(i+1)] |
| SW1-SW2    | byte (2)  | Return code       | see list of return codes |

---