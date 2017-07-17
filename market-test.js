
const config = require('config');
const Market = require('./Market');

const market = new Market();

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
