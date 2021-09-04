import { DeviceModel } from '@zondax/zemu'

const Resolve = require('path').resolve

export const APP_SEED = 'equip will roof matter pink blind book anxiety banner elbow sun young'

const APP_PATH_S = Resolve('../app/output/app_s.elf')
const APP_PATH_X = Resolve('../app/output/app_x.elf')

export const models: DeviceModel[] = [
  { name: 'nanos', prefix: 'S', path: APP_PATH_S },
  { name: 'nanox', prefix: 'X', path: APP_PATH_X },
]

// //    "name": "Balances_Transfer",
// export const txBasic =
//   '050000ca1ef1d326bd379143d6e743f6c3b51b7058d07e02e4614dc027e05bdb226c6503d2029649d503ae1103008ed73e0db80b0000010000006fbd74e5e1d0a61d52ccfe9d4adaed16dd3a7caa37c6bc4d0c2fa12e8b2f40636fbd74e5e1d0a61d52ccfe9d4adaed16dd3a7caa37c6bc4d0c2fa12e8b2f4063'
//
// //     "name": "Staking_Nominate",
// export const txNomination =
//   '11052000d4a0bb1ba3381ee65f90ab026a3daf35e4e5c5902c135cf1a65e9fbfa76d7069006c7171268faa17251728bd032443d576a7e0560252c345955dfdeb1eaf5cf17c00fa8c11825409b90b12c2316d16bd4282d739236c81305a72fafaaa275d15c336000867e378b6e1744b8822b56bfa8f7b0336d5ba37a5c25df1dce2b8941fedf953002c5eff1021e6fd1b04f2b7f10988337fd562a13f4a64671aba1e36cf303fff2300a8f4deb55f57d79d90f4718e2ad7c6fbabca000a9709701f464a02166c29b4510030db7d4634e1cc582cb16822193d9fd58cbcd942e77e67a50e9968421cbd2a71002e099c012ad7f670630d5dbd3ef188c076b0aa7e24ee96f8cfcd1e2e9423cc6ed503ae11030000b80b0000010000006fbd74e5e1d0a61d52ccfe9d4adaed16dd3a7caa37c6bc4d0c2fa12e8b2f40636fbd74e5e1d0a61d52ccfe9d4adaed16dd3a7caa37c6bc4d0c2fa12e8b2f4063'
//
// //     "name": "Utility_Batch",
// export const txBatch =
//   '29000c110c31463758677235564d355269596d6b437853317857483647626342675341354a0101000000110c31463758677235564d355269596d6b437853317857483647626342675341354a0102000000110910d5038d2400b80b0000010000006fbd74e5e1d0a61d52ccfe9d4adaed16dd3a7caa37c6bc4d0c2fa12e8b2f40636fbd74e5e1d0a61d52ccfe9d4adaed16dd3a7caa37c6bc4d0c2fa12e8b2f4063'
