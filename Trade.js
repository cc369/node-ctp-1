
const EventEmitter = require('eventemitter3');
const { CtpTrade: TradeAPI } = require('./bindings/build/Debug/ctp.node');
const iconvLite = require('iconv-lite');

class Trade extends EventEmitter {

  constructor() {
    super();
    this.api = new TradeAPI();

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

  qInstruments(conditions) {
    const reqId = ++this.reqId;
    this.api.queryInstruments(conditions || {}, reqId);
  }
}

module.exports = Trade;