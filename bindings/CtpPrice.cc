
#include "CtpPrice.h"

#include <node_buffer.h>

using namespace std;
using namespace v8;
using namespace xisca::bindings::javascript;

Persistent<Function> CtpPrice::constructor;

#define INTERNAL_TINY_BUF_SIZE 64

// Persistent Strings.
#define DECL_STR(strk) static Persistent<String> gstr##strk; static ExternalOneByteStringResourceImpl grStr_##strk(#strk, sizeof(#strk));
#define GSTR(strk) gstr##strk
#define GRSTR(strk) static_cast<String::ExternalOneByteStringResource*>(&grStr_##strk)
#define STR_GR(strk,isolate) String::NewExternalOneByte(isolate, GRSTR(strk)).ToLocalChecked()

DECL_STR(TradingDay);
DECL_STR(UserProductInfo);
DECL_STR(InterfaceProductInfo);
DECL_STR(ProtocolInfo);
DECL_STR(MacAddress);
DECL_STR(OneTimePassword);
DECL_STR(ClientIPAddress);
DECL_STR(LoginRemark);
DECL_STR(BrokerID);
DECL_STR(Password);
DECL_STR(UserID);


DECL_STR(LoginTime);
DECL_STR(SystemName);
DECL_STR(FrontID);
DECL_STR(SessionID);
DECL_STR(MaxOrderRef);
DECL_STR(SHFETime);
DECL_STR(DCETime);
DECL_STR(CZCETime);
DECL_STR(FFEXTime);
DECL_STR(INETime);

//DECL_STR(connect);
DECL_STR(code);
DECL_STR(reason);
DECL_STR(requestId);
DECL_STR(last);
//DECL_STR(login);
//DECL_STR(disconnect);
//DECL_STR(hbWarning);

enum eventType {
	gEvtTypeUnknown = 0,
	gEvtTypeFirst = 0,
	gEvtTypeConnect = 1,
	gEvtTypeDisconnect = 2,
	gEvtTypeHeartBeatWarning = 3,
	gEvtTypeLogin = 4,
	gEvtTypeError = 5,
	gEvtTypeSubscribe = 6,

	gEvtTypeLast
};

// Persitent str lists.
static void initString(Persistent<String> &persistent, Isolate *isolate, const char* str) {
	persistent.Reset(isolate, String::NewFromUtf8(isolate, str));
}

static void intializeStrings(Isolate *isolate) {
#define INIT_STR(strk) initString(gstr##strk, isolate, #strk)
	INIT_STR(TradingDay);
	INIT_STR(UserProductInfo);
	INIT_STR(InterfaceProductInfo);
	INIT_STR(ProtocolInfo);
	INIT_STR(MacAddress);
	INIT_STR(OneTimePassword);
	INIT_STR(ClientIPAddress);
	INIT_STR(LoginRemark);

	INIT_STR(LoginTime);
	INIT_STR(SystemName);
	INIT_STR(FrontID);
	INIT_STR(SessionID);
	INIT_STR(MaxOrderRef);
	INIT_STR(SHFETime);
	INIT_STR(DCETime);
	INIT_STR(CZCETime);
	INIT_STR(FFEXTime);
	INIT_STR(INETime);

	//INIT_STR(connect);
	INIT_STR(code);
	INIT_STR(reason);
	INIT_STR(requestId);
	INIT_STR(last);
	//INIT_STR(login);
	//INIT_STR(disconnect);
	//INIT_STR(hbWarning);
}

static int optAssignString(Isolate *isolate,
	char *ptrDest,
	size_t maxDestLength, 
	Local<Object> object,
	const char *ptr) {
	Local<String> fieldName = String::NewFromUtf8(isolate, ptr);
	if (object->Has(fieldName)) {
		Local<Value> value = object->Get(fieldName);

		if (!value->IsString()) {
			char buf[INTERNAL_TINY_BUF_SIZE] = { 0 };
			std::string pattern = "Field '";
			fieldName->WriteOneByte((unsigned char *)buf, INTERNAL_TINY_BUF_SIZE);
			pattern += buf;
			pattern += "' is required to be a string.";

			// Then we should throw an error.
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, pattern.c_str())));

			return 1;
		}

		Local<String> str = value->ToString();
		str->WriteUtf8(ptrDest, maxDestLength);
	}

	return 0;
}

static int reqAssignString(Isolate *isolate,
	char *ptrDest,
	size_t maxDestLength,
	Local<Object> object,
	const char *ptr) {
	Local<String> fieldName = String::NewFromUtf8(isolate, ptr);
	if (object->Has(fieldName)) {
		Local<Value> value = object->Get(fieldName);

		if (!value->IsString()) {
			char buf[INTERNAL_TINY_BUF_SIZE] = { 0 };
			std::string pattern = "Field '";
			fieldName->WriteOneByte((unsigned char *)buf, INTERNAL_TINY_BUF_SIZE);
			pattern += buf;
			pattern += "' is required to be a string.";

			// Then we should throw an error.
			isolate->ThrowException(Exception::Error(
				String::NewFromUtf8(isolate, pattern.c_str())));

			return 1;
		}

		Local<String> str = value->ToString();
		str->WriteUtf8((char *)ptrDest, maxDestLength);
	}
	else {
		char buf[INTERNAL_TINY_BUF_SIZE] = { 0 };
		std::string pattern = "Field '";
		fieldName->WriteUtf8((char *)buf, INTERNAL_TINY_BUF_SIZE);
		pattern += buf;
		pattern += "' is required, but not provided.";

		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, pattern.c_str())));

		return 1;
	}

	return 0;
}

static void assignToObject(Isolate *isolate,
	Local<Object> &object,
	const char *ptrField,
	const char *ptrSource) {
	// It has content.
	if (*ptrSource != '\0') {
		object->Set(String::NewFromUtf8(isolate, ptrField), String::NewFromUtf8(isolate, ptrSource));
	}
}

// Utility functions

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
void run_in_main_thread(uv_async_t *async) {
	CtpPrice *price = (CtpPrice *)async->data;
	price->HandleEventQueue();
}

CtpPrice::CtpPrice(const char *pszFlowPath, const bool bIsUsingUdp, const bool bIsMulticast)
	: m_api(nullptr), mb_init(false)
{
	m_api = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath, bIsUsingUdp, bIsMulticast);
}

CtpPrice::~CtpPrice() {
	Release();
}

void CtpPrice::Release() {
	if (m_api) {
		m_api->Release();
		m_api = nullptr;
	}

	// Only be called when API is initialized.
	if (mb_init) {
		uv_close((uv_handle_t *)&async, NULL);
		// uv_close((uv_handle_t *)&m_async_wait, NULL);
	}

	printf("Released\n");
}

void CtpPrice::RegisterSpi() {
	// Use this in constructor is not safe.
	m_api->RegisterSpi(this);
}

void CtpPrice::Init() {
	printf("Initializing...\n");
	// Initialize the libuv callback.
	if (!mb_init) {
		// uv_async_init(uv_default_loop(), &m_async_wait, run_in_main_thread);
		uv_async_init(uv_default_loop(), &async, run_in_main_thread);

		this->async.data = this;

		m_api->Init();

		mb_init = true;
	}

	printf("Initialized.\n");
}

// -------------------------------- V8 Interfaces --------------------------------

void CtpPrice::Init(Local<Object> exports) {
	Isolate* isolate = exports->GetIsolate();

	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "CtpPrice"));
	tpl->InstanceTemplate()->SetInternalFieldCount(7);

    // Initialize the CTP strs;

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "register", RegisterSpi);
	NODE_SET_PROTOTYPE_METHOD(tpl, "release", Release);
	NODE_SET_PROTOTYPE_METHOD(tpl, "init", Init);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addFront", AddFrontAddress);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addNS", AddNameServer);
	NODE_SET_PROTOTYPE_METHOD(tpl, "login", Login);
	NODE_SET_PROTOTYPE_METHOD(tpl, "subscribe", Subscribe);

	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "CtpPrice"),
		tpl->GetFunction());
}

int CtpPrice::AssertInitialized(v8::Isolate *isolate) {
	if (!m_api) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "CtpPrice is not initialized.")
		));

		return 1;
	}

	return 0;
}

void CtpPrice::RegisterSpi(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
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

void CtpPrice::Release(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	obj->Release();
}

void CtpPrice::Init(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	obj->Init();
}

void CtpPrice::AddFrontAddress(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
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

void CtpPrice::AddNameServer(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
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

void CtpPrice::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		// double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
		CtpPrice* obj = new CtpPrice();
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

void CtpPrice::Login(const FunctionCallbackInfo<Value>& args) {

	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
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

#define OPT_ASSIGN_LG(strk) if (optAssignString(isolate, login.strk, sizeof(login.strk), req, #strk)) { return; }
#define REQ_ASSIGN_LG(strk) if (reqAssignString(isolate, login.strk, sizeof(login.strk), req, #strk)) { return; }
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

void CtpPrice::Subscribe(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "Subscribe function requires 1 argument")));
		return;
	}

	if (!args[0]->IsArray()) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "The first arguments for Subscribe request should be an array")));
		return;
	}

	Local<Array> arr = Local<Array>::Cast(args[0]);
	uint32_t length = arr->Length();

	char **instruments = new char *[length];
	for (uint32_t i = 0; i < length; i++) {
		Local<String> str = arr->Get(i)->ToString();
		instruments[i] = new char[str->Length() + 2];
		str->WriteUtf8(instruments[i]);
		instruments[i][str->Length()] = '\0';
		printf("Instrument %sf\n", instruments[i]);
	}
	
	obj->m_api->SubscribeMarketData(instruments, length);
	for (uint32_t i = 0; i < length; i++) {
		delete[] instruments[i];
	}
	delete[] instruments;
}

void CtpPrice::Unsubscribe(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	// obj->value_ += 1;

	// args.GetReturnValue().Set(Number::New(isolate, obj->value_));
}

void CtpPrice::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast) {
	printf("RspLogin \n");

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

void CtpPrice::OnFrontConnected() {
	printf("Front connected.\n");
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

void CtpPrice::OnFrontDisconnected(int reason) {
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

void CtpPrice::OnHeartBeatWarning(int nTimeLapse) {
	printf("Warning %d", nTimeLapse);
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


void CtpPrice::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	printf("RspError \n");

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

void CtpPrice::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	printf("Subscribe \n");

	ctp_message m;
	if (pRspInfo) {
		m.error_code = pRspInfo->ErrorID;
		m.error = pRspInfo->ErrorMsg;
	}
	else {
		// Success message
		m.error_code = 0;
	}

	m.type = gEvtTypeSubscribe;
	m.is_last = bIsLast;

	// For broadcast message only.
	m.request_id = nRequestID;
	m.pointer = new CThostFtdcSpecificInstrumentField(*pSpecificInstrument);

	queue.enqueue(m);

	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

Local<Object> getErrorObj(Isolate *isolate, CtpPrice::ctp_message *msg) {
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

#define HandleEvent(event) void CtpPrice::HandleEvent##event(v8::Isolate *isolate, v8::Local<v8::Function> &cb, ctp_message *msg)
HandleEvent(Connect) {
	const unsigned argc = 1;
	Local<Value> argv[argc] = { String::NewFromUtf8(isolate, "connect") }; // A persistent object is useless here.
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

HandleEvent(Disconnect) {
	const unsigned argc = 2;
	printf("reason %d\n", msg->error_code);

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

HandleEvent(Subscribe) {
	// Handle the login object.
	Local<Object> object = Object::New(isolate);
	const CThostFtdcSpecificInstrumentField *ptr = (const CThostFtdcSpecificInstrumentField *)msg->pointer;
	if (ptr) {
		ASSIGN_TO_OBJ(InstrumentID);
	}

	Local<Object> error = getErrorObj(isolate, msg);

	const unsigned argc = 3;
	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate, "subscribe"),
		object,
		// Handle error here.
		error
	};
	node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), cb, argc, argv);
}

#define ConnectToEvent(event) case gEvtType##event: HandleEvent##event(isolate, callback, pEle); break

// Pick data out fromd queue, and then pack them as Node values.
void CtpPrice::HandleEventQueue() {
	printf("Queue\n");
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
			ConnectToEvent(Subscribe);
		}

		// Pop the first element
		queue.pop();
		pEle = queue.peek();
	}
}