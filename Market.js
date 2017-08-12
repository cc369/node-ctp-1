
const version = process.env.NODE_CTP_DEBUG ? 'Debug' : 'Release';
const EventEmitter = require('eventemitter3');
const { CtpPrice: MarketAPI } = require(`./bindings/build/${version}/ctp.node`);
const iconvLite = require('iconv-lite');

class Market extends EventEmitter {

  constructor() {
    super();
    this.api = new MarketAPI();

    this.api.register(this.cb.bind(this));
    this.bInitialized = false;

    this.reqId = 0;
  }

  cb(event, data, error) {
    if (error && error.reason) {
      console.log('There are reasons');
      error.reason = iconvLite.decode(error.reason, 'gb2312');
    }

    if (event === 'error' && data && data.reason) {
      data.reason = iconvLite.decode(data.reason, 'gb2312');
    }

    if (event === 'feed' && data && data.InstrumentID) {
      data.InstrumentID = iconvLite.decode(data.InstrumentID, 'gb2312');
    }

    this.emit(event, data, error);
  }

  /**
   * Add series of fronts
   * @param fronts {Array.<string>} Fronts array.
   */
  addFronts(fronts) {
    for (let i = 0; i < fronts.length; i++) {
      this.addFront(fronts[i]);
    }
  }

  /**
   * Add a front address
   * @param front {string} Front address
   */
  addFront(front) {
    if (front instanceof Array) {
      this.addFronts(front);
      return;
    }

    this.api.addFront(front);
  }

  /**
   * Initialize the API.
   * An API can't be initialize for more than once.
   */
  init() {
    if (this.bInitialized) {
      return;
    }

    this.api.init();
    this.bInitialized = true;
  }

  login(data) {
    const reqId = ++this.reqId;

    const myData = {};
    myData.BrokerID = data.broker;
    myData.UserID = data.account;
    myData.Password = data.password;

    this.api.login(myData, reqId);
  }

  subscribe(contracts) {
    this.api.subscribe(contracts);
  }
}

module.exports = Market;
