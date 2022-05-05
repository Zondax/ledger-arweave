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

import Zemu, {DEFAULT_START_OPTIONS} from '@zondax/zemu'
import ArweaveApp from "@zondax/ledger-arweave";
import {APP_SEED, models} from './common'

const Arweave = require('arweave');

const defaultOptions = {
  ...DEFAULT_START_OPTIONS,
  startDelay: 5000,
  logging: true,
  custom: `-s "${APP_SEED}"`,
  X11: false,
  startTimeout: 300 * 1000,
  startText: "Arweave",
}

const owner = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

jest.setTimeout(1200 * 1000)

beforeAll(async () => {
  await Zemu.checkAndPullImage()
})

describe('Basic checks', function () {
  test.each(models)('can start and stop container', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({...defaultOptions, model: m.name})
    } finally {
      await sim.close()
    }
  })

  test.each(models)('main menu', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({...defaultOptions, model: m.name})
      await sim.navigateAndCompareSnapshots('.', `${m.prefix.toLowerCase()}-mainmenu`, [1, 0, 0, 4, -5])
    } finally {
      await sim.close()
    }
  })

  test.each(models)('get app version', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({...defaultOptions, model: m.name})
      const app = new ArweaveApp(sim.getTransport())
      const resp = await app.getVersion()

      console.log(resp)

      expect(resp.returnCode).toEqual(0x9000)
      expect(resp.errorMessage).toEqual('No errors')
      expect(resp).toHaveProperty('testMode')
      expect(resp).toHaveProperty('major')
      expect(resp).toHaveProperty('minor')
      expect(resp).toHaveProperty('patch')
    } finally {
      await sim.close()
    }
  })

  async function getFakeTx() {
    const arweave = Arweave.init({
      host: 'arweave.net',// Hostname or IP address for a Arweave host
      port: 443,          // Port
      protocol: 'https',  // Network protocol http or https
      timeout: 20000,     // Network request timeouts in milliseconds
      logging: false,     // Enable network request logging
    });

    const transactionAttributes = {
      target: '1seRanklLU_1VTGkEk7P0xAwMJfA7owA1JHW5KyZKlY',
      quantity: arweave.ar.arToWinston('10.5'),
      owner,
      reward: 12345,
      last_tx: 'A9Ic6RPOCXrpU1OzSFah3GfyUrCnFlAZ53NjPGjkCjVDgbQ6T1SU8ArIYWevd2aj',
      data: '<html><head><meta charset="UTF-8"><title>Hello world!</title></head><body></body></html>'
    }

    const transaction = await arweave.createTransaction(transactionAttributes)

    transaction.addTag('App-Name', 'SmartWeaveAction');
    transaction.addTag('App-Version', '0.3.0');
    transaction.addTag('Contract', '6eTVr8IKPNYbMHVcpHFXr-XNaL5hT6zRJXimcP-owmo');
    transaction.addTag('Input', '{"function":"transfer","target":"h-Bgr13OWUOkRGWrnMT0LuUKfJhRss5pfTdxHmNcXyw","qty":15000}');


    const jwtKey = {
      kty: 'RSA',
      n: 'paFZFKDQBy9soijj_NYhFl8glO09J2yrriWfy-QvG4FlsKulHDPi7CLZDEDr8RTZ4QfbYSpcGKnUoaOfuG4dDiHe2yfU4JYmU_dyt84Zoo-i647rJ2E8wpVkcbYae-cNEV5ADPan2gb_qbRuMJVMcW4cOgfrfQ7y_CIBDAL1-8h0ipSr-cSmzfdNagx_ihvcApawrYv49sqtNBYwZyltJ404c6Zhx-F4-ixrbODYj0Vcqv0frN8rVgQ_PoOKeWh1TVVT_SKXcgUeTIT5WM8JJPDHqEFEER0M19lvnhnD8thO1PGknU_NonFz-6sMwDEwgWbTQItmHY-06Y4xF5HK9RtjjisdAnC_AGgRMHXvdlQnSVFNrX-GzFMIA4WjhKgxBH2Qav0oSIxrRsfCab9Ky98eqmmwUJD-CpnyiTPUYRAHcGRwtLL4gNTqohHXH-Xq7YtsaVFh82Wu45tQr9cB4T5uPPGs2Lg_ahslx2Lha2q4HrP2AGyM_hU_RXuAdLbYrS_jbi9LpNUjPTeAbszHKgSILyrCbJmWb1DDI41SVKGzyaEEPmDhf3dphxR5YztMLzBfaySh8hOF-NOHQOkOHemXmk20acaEqZbdsz8Oh3BI8Rg7dBBtWYZhYKYi6ONuWT00z_-421-as0ims0nR7T5Conc25JhDwcP9fC5aI1c',
      e: 'AQAB',
      d: 'oWUA_cXaEDZZIAbCalxpr3gQsq4eOi19eVu2Q6LdeGr1oVsQr9Ormrg2UedtQeU0jj_uuNbjGTFcPgnIGJpL-7prg5hjqlkFjiqAgbT2a96VPf5tDFmHMIEMeHWfhv7VUAhMV5V-aEkLld14xiSlcZ5_KNkQ0jLf-WpVJNQ5yBhD38oRbnf-ppMFv4HgQb71-xvFm8baWfmTHz9D-TJ35-OHj8IgY_pEvVrNbzIGNE29h-zRkc2BV_cZfZwGMlnke-_awL6VXoi4Ro4Iv043NeTeWQkH_7kUC70YM7Idq6S15HB3HMdntJPjFehOq3UDNATzc2Cip8mh2zgwhCwP3Ie1WMR3kwvMzd5djKiDvILMyuX5xwoaipOR8kU9ojOuPhIaG6_ue7A0WNi6uE0UvU_I342zpP0HcjCAuqb17yjkW_n3_YZWz1yKQpcicKwwPY31Xl-EimRMzVAcmbvYLgqQfRAU--we4pnODuPSC0qhSnCHPpP-fSqEKW7hEC4Chfx3nO0H2C519MxVjqz_ofktd_PpnC0JmrphugWzPpJEBEF6O12K4gbDOaVsWLgWy3iZLaMpou4Ie6yuOmlhjUtyTnnAcTOKyiuxbnd2xotXJYSn7ISDpQhREbYiI_fwcwNIXKb6aGNd1Qc60bCJA472XQxki2QtNkFsara3igE',
      p: '2zYAu_CgtEd7bdNozwipvSuMnjpUYlWT0KV_a61DbHY0n7ZsNWmDB5Xqe6BCw2x0G0ke2yIN48KogO_iikzddmjDFDDpvV469qdH4ygaU9Sbrx_EpyfGWaXjBYMxkB63YToaITVyLMLSxhImevOQjRgje7xgjUN7Jtr04yrtFIxPUHXexwOQwzq3gTK5hvl8FYcMNldWn3-1yOqNJuNxGIjW_iTOylyWu65bQj1OeruZTZCbTG23MPyVhZvlKxCekT5iuhUA78xOsjpe2E1CIlYkDlthPUGENr6O3xBndGxI81GUPSon3ABcmgQ7iJr6Kg3bzCCF4WVlukLW5mAzAQ',
      q: 'wW1ZYMG2SU0lDW_19dTXDvFS3diEKfxY4RGBNNUqSUX7tVzmf9Yaj5KXYJ2Zq3Qmj10LzA1vFBPdTzuBgbuJ39ilvmR9k_ggPr8ELpdjH5IhM7rF6NwQXYgJJeqCR6IheIr8cdg_M_gpH0kDgjRW7rc5ORedPM9cjD7RTWvh01iUy8yHZDCIz25tmDZ4Nr3SM33NgvxWQw5eFYcnLD_uAzTa8FO6pdwINQR-J0iPQavPdRpd9r7JXRnpNN-UllDyrwGdekzOallLMnTkZw39Q3iOVc5yUYuH6T_N5XmTuZvA7OJW-k1gvTw0XxQWPdWhOanwP2ISPcmGSICzAJ7OVw',
      dp: 'I3qPDdv5NpRnLrRLv05oonxc6vMH2058eQWAJP5K7lAHXl4ZD1__XAfwhoe-poB0HkJsYw0U8Gr9-LhakwWzxklYYXk1K3gH1QnoAaTo9e5wMBzVKDSwzquhna_JIslF5s-PkRZkX0g88t_XKIkpORzSCWHIwrRMR1Ki1e2Jp4y5_YzGPLGgCWkEcD20d1sDsTxFRrpGA9SYqGcF52hiUpU7LrDoYyovHzst-_-OJyEbw8kGyYzsXFpVHOXJZnnvIBTXx8REfYs0pXX7CckfTg1hEo3JpC9pZa7b0tCF3fr0p6TpeCjNgLbyRd8vf4GNwaayj0woPv25RDczLuAdAQ',
      dq: 'viUprLUg5hQhLYF3j-9objcLWCxlWATKto4HrxEyHY_fbh1iNh8aQzM-_mpPSz3GPj6gvzohjOIuN5puqELcuaF25aBR1qkfwa7zhH2rorh7TSf5L_1NVTTKkWxkH9khACfZHG0RTJxSiH-1JEUhhE-3wAkXuFS8TN3f0TURbTryIxnvqq0PgKK0t-Ir8NxPi_DKm05F2muAT7dp6dT6vEo0Q_Z7UFrp8nx1K1boBQ3JxftijZJlztEO8LwtjEsNFr8GuNhSPjeDdZl1gl09MBuOCIoG1WTN-ZyQYlpOvzJ-yB-Ek1CHnd9WOHU9nBGrCU2mcBiPiA_YouNplF2Ixw',
      qi: '0Dg_6VwT_tKPTu79TzVmWXFE4dU_EtBsrjuP2KRC97L0LriS_luaO5IdwpeDS0y8N-SfI_Plc0I8yHSQVt-TW_AafhDjZ5Y_1PWpKpInb7a_WfFXVZAFVPORMNzqDNuR5QB_VJYqfpxLGTrrud9AexZQIAqfl-ANpw9q0_ZQ40ZEdNchT6nnTGpexGNBGSQQI8RyokqfLoFQ7JeLsTpffqYjW73TRFE-Bi4vWKN4n9Fr6gH7rvx1G3oydrVGGkttc6v8s8ZtA1x5FSwReIffPJToz6hye-7M17RI6mC6VmcI6fJz_5Tmdkgz6Nael12e-82cllMpmm45gOyFNG9l5g'
    }

    return {
      arweave: arweave,
      transaction,
      jwtKey,
      address: await arweave.wallets.jwkToAddress(jwtKey),
      digest: await transaction.getSignatureData()
    };
  }

  test.each(models)('fake tx', async function (m) {
    const exampleData = await getFakeTx();
    console.log(exampleData)

    const signatureData = await exampleData.transaction.getSignatureData()
    console.log(`Digest to sign: ${signatureData.length}  ` + Buffer.from(signatureData).toString("hex"));

    await exampleData.arweave.transactions.sign(exampleData.transaction, exampleData.jwtKey);
    console.log(exampleData.transaction);

    //const encodedTx = Buffer.from(ArweaveApp.encodeTx(exampleData.transaction))

    //console.log(encodedTx.toString("hex"))
  });

  test.each(models)('get address', async function (m) {
    const sim = new Zemu(m.path);
    try {
      await sim.start({...defaultOptions, model: m.name});
      const app = new ArweaveApp(sim.getTransport());

      const resp = await app.getAddress();
      console.log(resp)

      expect(resp.returnCode).toEqual(0x9000);
      expect(resp.errorMessage).toEqual("No errors");

      const expected_address_string = "47DEQpj8HBSa-_TImW-5JCeuQeRkm5NMpJWZG3hSuFU";
      expect(resp.address).toEqual(expected_address_string);

    } finally {
      await sim.close();
    }
  });

  test.each(models)('show address', async function (m) {
    const sim = new Zemu(m.path);
    try {
      await sim.start({...defaultOptions, model: m.name});
      const app = new ArweaveApp(sim.getTransport());

      const respRequest = app.showAddress();
      // Wait until we are not in the main menu
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      // Now navigate the address
      await sim.compareSnapshotsAndApprove(".", `${m.prefix.toLowerCase()}-show_address`);

      const resp = await respRequest;
      console.log(resp);

      expect(resp.returnCode).toEqual(0x9000);
      expect(resp.errorMessage).toEqual("No errors");

      const expected_address_string = "47DEQpj8HBSa-_TImW-5JCeuQeRkm5NMpJWZG3hSuFU";
      expect(resp.address).toEqual(expected_address_string);
    } finally {
      await sim.close();
    }
  });

  test.each(models)('show address - expert', async function (m) {
    const sim = new Zemu(m.path);
    try {
      await sim.start({...defaultOptions, model: m.name});
      const app = new ArweaveApp(sim.getTransport());

      // Enable expert mode
      await sim.clickRight();
      await sim.clickBoth();
      await sim.clickLeft();

      const respRequest = app.showAddress();
      // Wait until we are not in the main menu
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      // Now navigate the address
      await sim.compareSnapshotsAndApprove(".", `${m.prefix.toLowerCase()}-show_address_expert`);

      const resp = await respRequest;
      console.log(resp);

      expect(resp.returnCode).toEqual(0x9000);
      expect(resp.errorMessage).toEqual("No errors");

      const expected_address_string = "47DEQpj8HBSa-_TImW-5JCeuQeRkm5NMpJWZG3hSuFU";
      expect(resp.address).toEqual(expected_address_string);
    } finally {
      await sim.close();
    }
  });

  test.each(models)('sign - transfer', async function (m) {
    const sim = new Zemu(m.path);
    try {
      await sim.start({...defaultOptions, model: m.name});
      const app = new ArweaveApp(sim.getTransport());

      const exampleData = await getFakeTx();
      console.log(exampleData)

      // do not wait here..
      const signatureRequest = app.sign(exampleData.transaction);
      // Wait until we are not in the main menu
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

      await sim.compareSnapshotsAndApprove(".", `${m.prefix.toLowerCase()}-sign_transfer`);

      const resp = await signatureRequest;
      console.log(resp);

      expect(resp.returnCode).toEqual(0x6F01);
      //expect(resp.errorMessage).toEqual("No errors");

      // FIXME this is disabled as app is not signing anything on testing mode.
      /*
      const id = await Arweave.crypto.hash(resp.signature);
      const sigjs = {
        signature: await Arweave.utils.bufferTob64Url(resp.signature),
        id: await Arweave.utils.bufferTob64Url(id)
      };

      await exampleData.transaction.setSignature(sigjs);
      //Manually add owner again, it disappeared..
      await exampleData.transaction.setOwner(owner);
      console.log(exampleData.transaction);

      const v2 = await Arweave.transactions.verify(exampleData.transaction);
      console.log(v2);*/

      //This fails now, but the above transaction verify works.
      // let v3 = await arweave.crypto.verify(owner, signatureData, response.signature);
      // console.log(v3);
      // Prepare digest
    } finally {
      await sim.close();
    }
  });
});
