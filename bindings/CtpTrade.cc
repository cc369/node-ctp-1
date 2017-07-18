
#include "CtpTrade.h"

#include <node_buffer.h>

using namespace std;
using namespace v8;
using namespace xisca::bindings::javascript;

Persistent<Function> CtpTrade::constructor;

// Utility functions

enum eventType {
	gEvtTypeUnknown = 0,
	gEvtTypeFirst = 0,
	gEvtTypeConnect = 1,
	gEvtTypeDisconnect = 2,
	gEvtTypeHeartBeatWarning = 3,
	gEvtTypeLogin = 4,
	gEvtTypeError = 5,
	gEvtTypeQueryInstruments = 6,

	gEvtTypeLast
};

//template<typename Type>
// Local<Type>
static void GetObjectField(Handle<Object> object, Handle<Value> key) {
	auto value = object->Get(key);
	if (value->IsUndefined()) {
		// Ignore target value
	}
	else if (value->IsNull()) {

	} // Then just return the value
	else if (!value->IsString()) {

	}
}

// This function run in main thread.
// The only thing it would do is just call the main thread to start working.
static void run_in_main_thread(uv_async_t *async) {
	CtpTrade *price = (CtpTrade *)async->data;
	price->HandleEventQueue();
}

CtpTrade::CtpTrade(const char *pszFlowPath)
	: m_api(nullptr), mb_init(false)
{
	m_api = CThostFtdcTraderApi::CreateFtdcTraderApi(pszFlowPath);
}

CtpTrade::~CtpTrade() {
	Release();
}

void CtpTrade::Release() {
	if (m_api) {
		m_api->Release();
		m_api = nullptr;
	}

	// Only be called when API is initialized.
	if (mb_init) {
		uv_close((uv_handle_t *)&async, NULL);
		// uv_close((uv_handle_t *)&m_async_wait, NULL);
	}
}

void CtpTrade::RegisterSpi() {
	// Use this in constructor is not safe.
	m_api->RegisterSpi(this);
}

void CtpTrade::Init() {
	// Initialize the libuv callback.
	if (!mb_init) {
		// uv_async_init(uv_default_loop(), &m_async_wait, run_in_main_thread);
		uv_async_init(uv_default_loop(), &async, run_in_main_thread);

		this->async.data = this;

		m_api->Init();

		mb_init = true;
	}
}

// -------------------------------- V8 Interfaces --------------------------------

void CtpTrade::Init(Local<Object> exports) {
	Isolate* isolate = exports->GetIsolate();

	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "CtpTrade"));
	tpl->InstanceTemplate()->SetInternalFieldCount(7);

    // Initialize the CTP strs;

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "register", RegisterSpi);
	NODE_SET_PROTOTYPE_METHOD(tpl, "release", Release);
	NODE_SET_PROTOTYPE_METHOD(tpl, "init", Init);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addFront", AddFrontAddress);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addNS", AddNameServer);
	NODE_SET_PROTOTYPE_METHOD(tpl, "login", Login);
	NODE_SET_PROTOTYPE_METHOD(tpl, "queryInstruments", QueryInstruments);

	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "CtpTrade"),
		tpl->GetFunction());
}

int CtpTrade::AssertInitialized(v8::Isolate *isolate) {
	if (!m_api) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "CtpTrade is not initialized.")
		));

		return 1;
	}

	return 0;
}

void CtpTrade::RegisterSpi(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	// Check the number of arguments passed.
	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "`register` function requires 1 argument.")));
		return;
	}

	if (!args[0]->IsFunction()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "The first arguments for `register` should be a function")));
		return;
	}

	obj->RegisterSpi();
	obj->m_handler.Reset(isolate, Local<Function>::Cast(args[0]));
}

void CtpTrade::Release(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	obj->Release();
}

void CtpTrade::Init(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	obj->Init();
}

void CtpTrade::AddFrontAddress(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	// Check the number of arguments passed.
	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "AddFrontAddress function requires 1 argument.")));
		return;
	}

	if (!args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "The first arguments for AddFrontAddress should be a string")));
		return;
	}

	Local<String> str = args[0]->ToString();
	char *buffer = new char[str->Length() + 1];
	ZeroMemory(buffer, str->Length() + 1);
	str->WriteUtf8(buffer);
	obj->m_api->RegisterFront(buffer);

	delete[] buffer;
	// obj->m_api->RegisterFront();
}

void CtpTrade::AddNameServer(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	// Check the number of arguments passed.
	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "AddNameServer function requires 1 argument.")));
		return;
	}

	if (!args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "The first arguments for AddNameServer should be a string")));
		return;
	}

	Local<String> str = args[0]->ToString();
	char *buffer = new char[str->Length()];
	ZeroMemory(buffer, str->Length() + 1);
	str->WriteUtf8(buffer);
	obj->m_api->RegisterNameServer(buffer);

	delete[] buffer;
}

void CtpTrade::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		// double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
		CtpTrade* obj = new CtpTrade();
		// obj->SetIsolate(isolate);
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else {
		// Invoked as plain function `MyObject(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		Local<Object> result =
			cons->NewInstance(context, argc, argv).ToLocalChecked();
		args.GetReturnValue().Set(result);
	}
}



void CtpTrade::Login(const FunctionCallbackInfo<Value>& args) {

	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	// Check the number of arguments passed.
	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "Login function requires 2 arguments")));
		return;
	}

	if (!args[0]->IsObject()) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "The first arguments for Login request should be an object")));
		return;
	}

	if (!args[1]->IsNumber()) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "The second argument for Login should be a number of integer")));
		return;
	}

	// Copy contents from contents in object login;
	// The check is required, otherwise there will be a lot of errors.
	CThostFtdcReqUserLoginField login = { 0 };
	// Copy contents from your object to here.

	// Then I just get that key.
	Local<Object> req = args[0]->ToObject();

#define OPT_ASSIGN_LG(strk) OPT_ASSIGN(strk, login)
#define REQ_ASSIGN_LG(strk) REQ_ASSIGN(strk, login)
	//}

	OPT_ASSIGN_LG(TradingDay);
	REQ_ASSIGN_LG(BrokerID);
	REQ_ASSIGN_LG(UserID);
	REQ_ASSIGN_LG(Password);
	OPT_ASSIGN_LG(UserProductInfo);
	OPT_ASSIGN_LG(InterfaceProductInfo);
	OPT_ASSIGN_LG(ProtocolInfo);
	OPT_ASSIGN_LG(MacAddress);
	OPT_ASSIGN_LG(OneTimePassword);
	OPT_ASSIGN_LG(ClientIPAddress);
	OPT_ASSIGN_LG(LoginRemark);

	int result = obj->m_api->ReqUserLogin(&login, args[1]->Int32Value());
	args.GetReturnValue().Set(Number::New(isolate, result));
}

void CtpTrade::QueryInstruments(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpTrade* obj = ObjectWrap::Unwrap<CtpTrade>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "Subscribe function requires 2 arguments")));
		return;
	}

	if (!args[0]->IsObject()) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "The first arguments for Subscribe request should be an object")));
		return;
	}

	if (!args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "The first arguments for Subscribe request should be an integer")));
		return;
	}

	CThostFtdcQryInstrumentField qryInst;
	ZeroMemory(&qryInst, sizeof(qryInst));
	Local<Object> req = args[0]->ToObject();

#define OPT_ASSIGN_RQI(strk) OPT_ASSIGN(strk, qryInst)
	OPT_ASSIGN_RQI(InstrumentID);
	OPT_ASSIGN_RQI(ExchangeID);
	OPT_ASSIGN_RQI(ExchangeInstID);
	OPT_ASSIGN_RQI(ProductID);

	int32_t reqId = args[1]->Int32Value();
	obj->m_api->ReqQryInstrument(&qryInst, reqId);
}

void CtpTrade::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast) {
	ctp_message m;
	if (pRspInfo) {
		m.error_code = pRspInfo->ErrorID;
		m.error = pRspInfo->ErrorMsg;
	}
	else {
		// Success message
		m.error_code = 0;
	}

	m.type = gEvtTypeLogin;
	m.is_last = bIsLast;

	// For broadcast message only.
	m.request_id = nRequestID;
	m.pointer = new CThostFtdcRspUserLoginField(*pRspUserLogin);

	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

void CtpTrade::OnFrontConnected() {
	ctp_message m;
	// Fill the message here.

	// Success message
	m.error_code = 0;

	m.type = gEvtTypeConnect;
	m.is_last = true;

	// For broadcast message only.
	m.request_id = 0;
	m.pointer = nullptr;
	
	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

void CtpTrade::OnFrontDisconnected(int reason) {
	ctp_message m;
	// Fill the message here.

	// Success message
	m.error_code = reason;

	m.type = gEvtTypeDisconnect;
	m.is_last = true;

	// For broadcast message only.
	m.request_id = 0;
	m.pointer = nullptr;

	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

void CtpTrade::OnHeartBeatWarning(int nTimeLapse) {
	ctp_message m;
	// Fill the message here.

	// Success message
	m.error_code = nTimeLapse;

	m.type = gEvtTypeHeartBeatWarning;
	m.is_last = true;

	// For broadcast message only.
	m.request_id = 0;
	m.pointer = nullptr;

	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

void CtpTrade::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	ctp_message m;
	if (pRspInfo) {
		m.error_code = pRspInfo->ErrorID;
		m.error = pRspInfo->ErrorMsg;
	}
	else {
		// Success message
		m.error_code = 0;
	}

	m.type = gEvtTypeError;
	m.is_last = bIsLast;

	// For broadcast message only.
	m.request_id = nRequestID;
	m.pointer = nullptr;

	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

void CtpTrade::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	ctp_message m;
	if (pRspInfo) {
		m.error_code = pRspInfo->ErrorID;
		m.error = pRspInfo->ErrorMsg;
	}
	else {
		// Success message
		m.error_code = 0;
	}

	m.type = gEvtTypeQueryInstruments;
	m.is_last = bIsLast;

	// For broadcast message only.
	m.request_id = nRequestID;
	m.pointer = new CThostFtdcInstrumentField(*pInstrument);

	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}
//
//void CtpTrade::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
//	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
//	printf("Subscribe \n");
//
//	ctp_message m;
//	if (pRspInfo) {
//		m.error_code = pRspInfo->ErrorID;
//		m.error = pRspInfo->ErrorMsg;
//	}
//	else {
//		// Success message
//		m.error_code = 0;
//	}
//
//	m.type = gEvtTypeSubscribe;
//	m.is_last = bIsLast;
//
//	// For broadcast message only.
//	m.request_id = nRequestID;
//	m.pointer = new CThostFtdcSpecificInstrumentField(*pSpecificInstrument);
//
//	queue.enqueue(m);
//
//	this->async.data = this;
//
//	// This function just append a simple message to the queue, 
//	// and then wakeup the main thread.
//	// This is because the uv can call the main thread only once, even there are 
//	// multiply uv_async_send calls.
//	// We then provide the private queue inside it.
//
//	uv_async_send(&async);
//}

Local<Object> getErrorObj(Isolate *isolate, CtpTrade::ctp_message *msg) {
	Local<Object> error = Object::New(isolate);

	if (msg->error_code) {
		error->Set(String::NewFromUtf8(isolate, "code"), Int32::New(isolate, msg->error_code));
		Local<Object> buffer = node::Buffer::Copy(isolate, msg->error.c_str(), msg->error.size()).ToLocalChecked();
		error->Set(String::NewFromUtf8(isolate, "reason"), buffer);
	}

	error->Set(String::NewFromUtf8(isolate, "reqId"), Int32::New(isolate, msg->request_id));
	error->Set(String::NewFromUtf8(isolate, "last"), Boolean::New(isolate, msg->is_last));

	return error;
}

#define HandleEvent(event) void CtpTrade::HandleEvent##event(v8::Isolate *isolate, v8::Local<v8::Function> &cb, ctp_message *msg)
HandleEvent(Connect) {
	const unsigned argc = 1;
	Local<Value> argv[argc] = { String::NewFromUtf8(isolate, "connect") }; // A persistent object is useless here.
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

HandleEvent(Disconnect) {
	const unsigned argc = 2;
	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate, "disconnect"),
		Int32::New(isolate, msg->error_code)
	}; // A persistent object is useless here.
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

HandleEvent(HeartBeatWarning) {
	const unsigned argc = 2;
	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate, "hb-warning"),
		Int32::New(isolate, msg->error_code)
	}; // A persistent object is useless here.
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

#define ASSIGN_TO_OBJ(field) assignToObject(isolate, object, #field, ptr->field);

HandleEvent(Login) {
	// Handle the login object.
	Local<Object> object = Object::New(isolate);
	const CThostFtdcRspUserLoginField *ptr = (const CThostFtdcRspUserLoginField *)msg->pointer;
	if (ptr) {
		ASSIGN_TO_OBJ(TradingDay);
		ASSIGN_TO_OBJ(LoginTime);
		ASSIGN_TO_OBJ(BrokerID);
		ASSIGN_TO_OBJ(UserID);
		ASSIGN_TO_OBJ(SystemName);
		object->Set(String::NewFromUtf8(isolate, "FrontID"), Int32::New(isolate, ptr->FrontID));
		object->Set(String::NewFromUtf8(isolate, "SessionID"), Int32::New(isolate, ptr->SessionID));
		ASSIGN_TO_OBJ(MaxOrderRef);
		ASSIGN_TO_OBJ(SHFETime);
		ASSIGN_TO_OBJ(DCETime);
		ASSIGN_TO_OBJ(CZCETime);
		ASSIGN_TO_OBJ(FFEXTime);
		ASSIGN_TO_OBJ(INETime);

		delete ptr;
	}

	Local<Object> error = getErrorObj(isolate, msg);

	const unsigned argc = 3;
	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate, "login"),
		object,
		// Handle error here.
		error
	};
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

HandleEvent(Error) {
	Local<Object> error = getErrorObj(isolate, msg);

	const unsigned argc = 2;
	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate, "error"),
		error
	};
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

HandleEvent(QueryInstruments) {
	// Handle the login object.
	Local<Object> object = Object::New(isolate);
	const CThostFtdcInstrumentField *ptr = (const CThostFtdcInstrumentField *)msg->pointer;
	if (ptr) {
		ASSIGN_TO_OBJ(InstrumentID);
		ASSIGN_TO_OBJ(ExchangeID);
		ASSIGN_TO_OBJ(InstrumentName);
		ASSIGN_TO_OBJ(ExchangeInstID);
		ASSIGN_TO_OBJ(ProductID);
		object->Set(String::NewFromUtf8(isolate, "ProductClass"), Int32::New(isolate, ptr->ProductClass));
		object->Set(String::NewFromUtf8(isolate, "DeliveryYear"), Int32::New(isolate, ptr->DeliveryYear));
		object->Set(String::NewFromUtf8(isolate, "DeliveryMonth"), Int32::New(isolate, ptr->DeliveryMonth));
		object->Set(String::NewFromUtf8(isolate, "MaxMarketOrderVolume"), Int32::New(isolate, ptr->MaxMarketOrderVolume));
		object->Set(String::NewFromUtf8(isolate, "MinMarketOrderVolume"), Int32::New(isolate, ptr->MinMarketOrderVolume));
		object->Set(String::NewFromUtf8(isolate, "MaxLimitOrderVolume"), Int32::New(isolate, ptr->MaxLimitOrderVolume));
		object->Set(String::NewFromUtf8(isolate, "MinLimitOrderVolume"), Int32::New(isolate, ptr->MinLimitOrderVolume));
		object->Set(String::NewFromUtf8(isolate, "VolumeMultiple"), Int32::New(isolate, ptr->VolumeMultiple));
		object->Set(String::NewFromUtf8(isolate, "PriceTick"), Number::New(isolate, ptr->PriceTick));
		ASSIGN_TO_OBJ(CreateDate);
		ASSIGN_TO_OBJ(OpenDate);
		ASSIGN_TO_OBJ(ExpireDate);
		ASSIGN_TO_OBJ(StartDelivDate);
		ASSIGN_TO_OBJ(EndDelivDate);
		object->Set(String::NewFromUtf8(isolate, "InstLifePhase"), Int32::New(isolate, ptr->InstLifePhase));
		object->Set(String::NewFromUtf8(isolate, "IsTrading"), Boolean::New(isolate, ptr->IsTrading));
		object->Set(String::NewFromUtf8(isolate, "PositionType"), Int32::New(isolate, ptr->PositionType));
		object->Set(String::NewFromUtf8(isolate, "PositionDateType"), Int32::New(isolate, ptr->PositionDateType));

		object->Set(String::NewFromUtf8(isolate, "LongMarginRatio"), Number::New(isolate, ptr->LongMarginRatio));
		object->Set(String::NewFromUtf8(isolate, "ShortMarginRatio"), Number::New(isolate, ptr->ShortMarginRatio));
		object->Set(String::NewFromUtf8(isolate, "MaxMarginSideAlgorithm"), Int32::New(isolate, ptr->MaxMarginSideAlgorithm));
		ASSIGN_TO_OBJ(UnderlyingInstrID);
		object->Set(String::NewFromUtf8(isolate, "StrikePrice"), Number::New(isolate, ptr->StrikePrice));
		object->Set(String::NewFromUtf8(isolate, "OptionsType"), Int32::New(isolate, ptr->OptionsType));
		object->Set(String::NewFromUtf8(isolate, "UnderlyingMultiple"), Number::New(isolate, ptr->UnderlyingMultiple));
		object->Set(String::NewFromUtf8(isolate, "CombinationType"), Int32::New(isolate, ptr->CombinationType));

		delete ptr;
	}

	Local<Object> error = getErrorObj(isolate, msg);

	const unsigned argc = 3;
	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate, "query-instruments"),
		object,
		// Handle error here.
		error
	};
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

#define ConnectToEvent(event) case gEvtType##event: HandleEvent##event(isolate, callback, pEle); break

// Pick data out fromd queue, and then pack them as Node values.
void CtpTrade::HandleEventQueue() {
	Isolate *isolate = v8::Isolate::GetCurrent();
	HandleScope scope(isolate);

	ctp_message *pEle = this->queue.peek();

	v8::Local<v8::Function> callback(m_handler.Get(isolate));

	while (pEle) {
		// Nothing to handle.
		switch (pEle->type) {
			ConnectToEvent(Connect);
			ConnectToEvent(Disconnect);
			ConnectToEvent(HeartBeatWarning);
			ConnectToEvent(Error);
			ConnectToEvent(Login);
			ConnectToEvent(QueryInstruments);
			// ConnectToEvent(Subscribe);
		}

		// Pop the first element
		queue.pop();
		pEle = queue.peek();
	}
}