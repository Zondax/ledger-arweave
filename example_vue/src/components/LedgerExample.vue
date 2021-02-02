<template>
  <div class="Ledger">
    <!--
        Commands
    -->
    <button @click="getVersion">
      Get Version
    </button>

    <button @click="appInfo">
      AppInfo
    </button>

    <button @click="getAddress">
      Get Address
    </button>

    <button @click="showAddress">
      Show Address
    </button>

    <button @click="signExampleTx">
      Sign Example TX
    </button>
    <!--
        Commands
    -->
    <ul id="ledger-status">
      <li v-for="item in ledgerStatus" :key="item.index">
        {{ item.msg }}
      </li>
    </ul>
  </div>
</template>

<script>
import TransportWebUSB from "@ledgerhq/hw-transport-webusb";
import ArweaveApp from "@zondax/ledger-arweave";
const Arweave = require('arweave').default;

export default {
  name: "Ledger",
  props: {},
  data() {
    return {
      deviceLog: [],
    };
  },
  computed: {
    ledgerStatus() {
      return this.deviceLog;
    },
  },
  methods: {
    log(msg) {
      this.deviceLog.push({
        index: this.deviceLog.length,
        msg,
      });
    },
    async getTransport() {
      let transport = null;

      this.log(`Trying to connect via WebUSB...`);
      try {
        transport = await TransportWebUSB.create();
      } catch (e) {
        this.log(e);
      }
      return transport;
    },
    async getVersion() {
      const transport = await this.getTransport();

      try {
        this.deviceLog = [];
        const app = new ArweaveApp(transport);

        // now it is possible to access all commands in the app
        this.log("Sending Request..");
        const response = await app.getVersion();
        if (response.returnCode !== ArweaveApp.ErrorCode.NoError) {
          this.log(`Error [${response.returnCode}] ${response.errorMessage}`);
          return;
        }

        this.log("Response received!");
        this.log(`App Version ${response.major}.${response.minor}.${response.patch}`);
        this.log(`Device Locked: ${response.deviceLocked}`);
        this.log(`Test mode: ${response.testMode}`);
        this.log("Full response:");
        this.log(response);
      } finally {
        transport.close();
      }
    },
    async appInfo() {
      const transport = await this.getTransport();
      try {
        this.deviceLog = [];
        const app = new ArweaveApp(transport);

        // now it is possible to access all commands in the app
        this.log("Sending Request..");
        const response = await app.appInfo();
        if (response.returnCode !== ArweaveApp.ErrorCode.NoError) {
          this.log(`Error [${response.returnCode}] ${response.errorMessage}`);
          return;
        }

        this.log("Response received!");
        this.log(response);
      } finally {
        transport.close();
      }
    },
    async getAddress() {
      const transport = await this.getTransport();
      try {
        this.deviceLog = [];
        const app = new ArweaveApp(transport);

        let response = await app.getVersion();
        this.log(`App Version ${response.major}.${response.minor}.${response.patch}`);
        this.log(`Device Locked: ${response.deviceLocked}`);
        this.log(`Test mode: ${response.testMode}`);

        // now it is possible to access all commands in the app
        this.log("Sending Request..");
        response = await app.getAddress();
        if (response.returnCode !== ArweaveApp.ErrorCode.NoError) {
          this.log(`Error [${response.returnCode}] ${response.errorMessage}`);
          return;
        }

        this.log("Response received!");
        this.log("Full response:");
        this.log(response);
      } finally {
        transport.close();
      }
    },
    async showAddress() {
      const transport = await this.getTransport();
      this.deviceLog = [];
      try {
        const app = new ArweaveApp(transport);

        let response = await app.getVersion();
        this.log(`App Version ${response.major}.${response.minor}.${response.patch}`);
        this.log(`Device Locked: ${response.deviceLocked}`);
        this.log(`Test mode: ${response.testMode}`);

        // now it is possible to access all commands in the app
        this.log("Sending Request..");
        this.log("Please click in the device");
        response = await app.showAddress();
        if (response.returnCode !== ArweaveApp.ErrorCode.NoError) {
          this.log(`Error [${response.returnCode}] ${response.errorMessage}`);
          return;
        }

        this.log("Response received!");
        this.log("Full response:");
        this.log(response);
      } finally {
        transport.close();
      }
    },
    async signExampleTx() {
      const transport = await this.getTransport();

      try {
        this.deviceLog = [];
        const app = new ArweaveApp(transport);

        let response = await app.getVersion();
        this.log(`App Version ${response.major}.${response.minor}.${response.patch}`);
        this.log(`Device Locked: ${response.deviceLocked}`);
        this.log(`Test mode: ${response.testMode}`);

        const arweave = Arweave.init({
          host: 'arweave.net',// Hostname or IP address for a Arweave host
          port: 443,          // Port
          protocol: 'https',  // Network protocol http or https
          timeout: 20000,     // Network request timeouts in milliseconds
          logging: false,     // Enable network request logging
        });

        let addr = await app.getAddress();
        let owner = addr.owner;

        let transactionAttributes = {
          target: '1seRanklLU_1VTGkEk7P0xAwMJfA7owA1JHW5KyZKlY',
          quantity: arweave.ar.arToWinston('10.5'),
          owner: owner,
          reward: 12345,
          last_tx: 'A9Ic6RPOCXrpU1OzSFah3GfyUrCnFlAZ53NjPGjkCjVDgbQ6T1SU8ArIYWevd2aj',
          data: '<html><head><meta charset="UTF-8"><title>Hello world!</title></head><body></body></html>'
        }

        let transaction = await arweave.createTransaction(transactionAttributes)

        transaction.addTag('App-Name', 'SmartWeaveAction');
        transaction.addTag('App-Version', '0.3.0');
        transaction.addTag('Contract', '6eTVr8IKPNYbMHVcpHFXr-XNaL5hT6zRJXimcP-owmo');
        transaction.addTag('Input', '{"function":"transfer","target":"h-Bgr13OWUOkRGWrnMT0LuUKfJhRss5pfTdxHmNcXyw","qty":15000}');

        //////////////////////////////////////
        // Let's try signing locally

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
        await arweave.transactions.sign(transaction, jwtKey);
        console.log(transaction);

        let signatureData = await transaction.getSignatureData();
        console.log(signatureData);
        let sig = await arweave.utils.b64UrlToBuffer(transaction.signature);
        console.log(sig);
        let v = await arweave.transactions.verify(transaction);
        console.log(v);

        //////////////////////////////////////
        // Let's try signing with a Ledger

        console.log("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")

        response = await app.sign(transaction);
        this.log("Signature received!");


        let id = await arweave.crypto.hash(response.signature);
        let sigjs = {
          signature: await arweave.utils.bufferTob64Url(response.signature),
          id: await arweave.utils.bufferTob64Url(id)
        };

        await transaction.setSignature(sigjs);
        console.log(transaction);

        let v2 = await arweave.transactions.verify(transaction);
        this.log(v2);

        let v3 = await arweave.crypto.verify(owner, signatureData, response.signature);
        console.log(v3);

      } finally {
        transport.close();
      }
    },
  },
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
h3 {
  margin: 40px 0 0;
}

button {
  padding: 5px;
  font-weight: bold;
  font-size: medium;
}

ul {
  padding: 10px;
  text-align: left;
  alignment: left;
  list-style-type: none;
  background: black;
  font-weight: bold;
  color: greenyellow;
}
</style>
