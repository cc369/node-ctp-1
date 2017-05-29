
#include "CtpPrice.h"

using namespace v8;
using namespace xisca::bindings::javascript;

Persistent<Function> CtpPrice::constructor;
Persistent<String> TradingDay;
// Utility functions

//template<typename Type>
// Local<Type>
void GetObjectField(Handle<Object> object, Handle<Value> key) {
	auto value = object->Get(key);
	if (value->IsUndefined()) {
		// Ignore target value
	}
	else if (value->IsNull()) {

	} // Then just return the value
	else if (!value->IsString()) {

	}
}

CtpPrice::CtpPrice(const char *pszFlowPath, const bool bIsUsingUdp, const bool bIsMulticast)
	: m_api(nullptr)
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
}

void CtpPrice::RegisterSpi() {
	// Use this in constructor is not safe.
	m_api->RegisterSpi(this);
}

void CtpPrice::Init() {
	m_api->Init();
}

// -------------------------------- V8 Interfaces --------------------------------

void CtpPrice::Init(Local<Object> exports) {
	Isolate* isolate = exports->GetIsolate();

	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "CtpPrice"));
	tpl->InstanceTemplate()->SetInternalFieldCount(0);

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "registerSpi", RegisterSpi);
	NODE_SET_PROTOTYPE_METHOD(tpl, "release", Release);
	NODE_SET_PROTOTYPE_METHOD(tpl, "init", Init);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addFront", AddFrontAddress);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addNS", AddNameServer);

	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "CtpPrice"),
		tpl->GetFunction());
}

void CtpPrice::AssertInitialized(v8::Isolate *isolate) {
	if (!m_api) {
		isolate->ThrowException(Exception::Error(
			String::NewFromUtf8(isolate, "CtpPrice is not initialized.")
		));
	}
}

void CtpPrice::RegisterSpi(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->AssertInitialized(isolate);
	obj->RegisterSpi();
}

void CtpPrice::Release(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->AssertInitialized(isolate);
	obj->Release();
}

void CtpPrice::Init(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->AssertInitialized(isolate);
	obj->Init();
}

void CtpPrice::AddFrontAddress(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->AssertInitialized(isolate);
	obj->m_api->RegisterFront();
}

void CtpPrice::AddNameServer(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->AssertInitialized(isolate);
	obj->m_api->RegisterFront();
}


void CtpPrice::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		// double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
		CtpPrice* obj = new CtpPrice();
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

	// Check the number of arguments passed.
	if (args.Length() < 1) {
		// Throw an Error that is passed back to JavaScript
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Login function requires 2 arguments")));
		return;
	}

	if (!args[0]->IsObject()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "The first arguments for Login request should be an object")));
		return;
	}

	if (!args[1]->IsNumber()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "The second argument for Login should be a number of integer")));
		return;
	}

	// Copy contents from contents in object login;
	// The check is required, otherwise there will be a lot of errors.
	CThostFtdcReqUserLoginField login = { 0 };
	// Copy contents from your object to here.
	Local<Object> req = args[0]->ToObject;
	Local<String> key = String::NewFromUtf8(isolate, "TradingDay");

	auto value = req->Get(key);
	if (value->IsUndefined()) {
		// Ignore target value
	}
	else if (value->IsNull()) {

	}

	///交易日
	TThostFtdcDateType	TradingDay;
	///经纪公司代码
	TThostFtdcBrokerIDType	BrokerID;
	///用户代码
	TThostFtdcUserIDType	UserID;
	///密码
	TThostFtdcPasswordType	Password;
	///用户端产品信息
	TThostFtdcProductInfoType	UserProductInfo;
	///接口端产品信息
	TThostFtdcProductInfoType	InterfaceProductInfo;
	///协议信息
	TThostFtdcProtocolInfoType	ProtocolInfo;
	///Mac地址
	TThostFtdcMacAddressType	MacAddress;
	///动态密码
	TThostFtdcPasswordType	OneTimePassword;
	///终端IP地址
	TThostFtdcIPAddressType	ClientIPAddress;
	///登录备注
	TThostFtdcLoginRemarkType	LoginRemark;

	int result = obj->m_api->ReqUserLogin(&login, args[1]->ToNumber()->Int32Value);

	args.GetReturnValue().Set(Number::New(isolate, result));
}

void CtpPrice::Subscribe(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->value_ += 1;

	args.GetReturnValue().Set(Number::New(isolate, obj->value_));
}

void CtpPrice::Unsubscribe(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	obj->value_ += 1;

	args.GetReturnValue().Set(Number::New(isolate, obj->value_));
}
