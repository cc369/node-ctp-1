
const config = require('config');
const Market = require('./Market');
const Trade = require('./Trade');

const market = new Market();
const trade = new Trade();

const fronts = config.get('price.fronts');
market.addFronts(fronts);

// Then register
market.init();

market.on('connect', () => {
  console.log('Front connected!');

  market.login(config.get('price.account'));
});

///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
market.on('disconnect', (reason) => {
  console.log('disconnected, reason: ' + reason);
});

market.on('hb-warning', (nTimeLapse) => {
  console.log('heart beat warning, timeout: ' + nTimeLapse);
});

market.on('login', (...data) => {
  console.log('Login', data);
  market.subscribe(['wr1082']);
});

market.on('subscribe', (data) => {
  console.log('subs', data);
});

market.on('error', (data) => {
  console.log(data);
});

const fs = require('fs');
const feedStream = fs.createWriteStream('feeds.txt');
market.on('feed', (data) => {
  feedStream.write(`${JSON.stringify(data)}\n`);
});

// --------------------------------------
const tFronts = config.get('trade.fronts');
trade.addFronts(tFronts);
trade.init();

trade.on('connect', () => {
  console.log('Trade Front connected!');

  trade.login(config.get('trade.account'));
});

///@param nReason 错误原因
///        0x1001 网络读失败
///        0x1002 网络写失败
///        0x2001 接收心跳超时
///        0x2002 发送心跳失败
///        0x2003 收到错误报文
trade.on('disconnect', (reason) => {
  console.log('trade disconnected, reason: ' + reason);
});

trade.on('hb-warning', (nTimeLapse) => {
  console.log('heart beat warning, timeout: ' + nTimeLapse);
});

const instruments = [];

trade.on('login', (...data) => {
  console.log('Trade Login', data);
  trade.qInstruments();
});

trade.on('error', (data) => {
  console.log('Trade Error', data);
});

const writeStream = fs.createWriteStream('instruments.txt');

trade.on('query-instruments', (query, err) => {
  // instruments.push(query.InstrumentID);
  //if (query.InstrumentID.startsWith('au')) {
    instruments.push(query.InstrumentID);
    writeStream.write(JSON.stringify(query) + '\n');
  //}
  if (err.last) {
    console.log('Finished');
    market.subscribe(instruments);
  }
});