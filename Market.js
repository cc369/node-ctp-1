
let { CtpPrice: MarketAPI } = require('./bindings/build/Debug/ctp.node');

class Market extends MarketAPI {
}

module.exports = Market;