
#include "CtpPrice.h"

using namespace std;
using namespace v8;
using namespace xisca::bindings::javascript;

Persistent<Function> CtpPrice::constructor;

#define INTERNAL_TINY_BUF_SIZE 64

// Persistent Strings.
#define DECL_STR(strk) static Persistent<String> gstr##strk
#define GSTR(strk) gstr##strk

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

// Persitent str lists.

static void initString(Persistent<String> persistent, Isolate *isolate, const char* str) {
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

}

static int optAssignString(Isolate *isolate,
	char *ptrDest,
	size_t maxDestLength, 
	Local<Object> object,
	const Persistent<String> &field) {
	Local<String> fieldName = field.Get(isolate);
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
		str->WriteOneByte((unsigned char *)ptrDest, maxDestLength);
	}

	return 0;
}

static int reqAssignString(Isolate *isolate,
	char *ptrDest,
	size_t maxDestLength,
	Local<Object> object,
	const Persistent<String> &field) {
	Local<String> fieldName = field.Get(isolate);
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
		str->WriteOneByte((unsigned char *)ptrDest, maxDestLength);
	}
	else {
		char buf[INTERNAL_TINY_BUF_SIZE] = { 0 };
		std::string pattern = "Field '";
		fieldName->WriteOneByte((unsigned char *)buf, INTERNAL_TINY_BUF_SIZE);
		pattern += buf;
		pattern += "' is required, but not provided.";

		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, pattern.c_str())));

		return 1;
	}

	return 0;
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
	: m_api(nullptr)
{
	m_api = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath, bIsUsingUdp, bIsMulticast);
	
	// Initialize the libuv callback.
	uv_async_init(uv_default_loop(), &async, run_in_main_thread);

	this->async.data = this;
}

CtpPrice::~CtpPrice() {
	Release();

	uv_close((uv_handle_t*) &async, NULL);
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

    // Initialize the CTP strs;

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

	obj->RegisterSpi();
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

	// obj->m_api->RegisterFront();
}

void CtpPrice::AddNameServer(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	if (obj->AssertInitialized(isolate)) {
		return;
	}

	// obj->m_api->RegisterFront();
}

void CtpPrice::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		// double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
		CtpPrice* obj = new CtpPrice();
		obj->SetIsolate(isolate);
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

	// Then I just get that key.
	Local<Object> req = args[0]->ToObject();

#define OPT_ASSIGN_LG(strk) if (optAssignString(isolate, login.strk, sizeof(login.strk), req, GSTR(strk))) { return; }
#define REQ_ASSIGN_LG(strk) if (reqAssignString(isolate, login.strk, sizeof(login.strk), req, GSTR(strk))) { return; }
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

	int result = obj->m_api->ReqUserLogin(&login, args[1]->ToNumber()->Int32Value);
	args.GetReturnValue().Set(Number::New(isolate, result));
}

void CtpPrice::Subscribe(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	// obj->value_ += 1;

	// args.GetReturnValue().Set(Number::New(isolate, obj->value_));
}

void CtpPrice::Unsubscribe(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	CtpPrice* obj = ObjectWrap::Unwrap<CtpPrice>(args.Holder());
	// obj->value_ += 1;

	// args.GetReturnValue().Set(Number::New(isolate, obj->value_));
}

void CtpPrice::OnFrontConnected() {
	this->async.data = this;

	// This function just append a simple message to the queue, 
	// and then wakeup the main thread.
	// This is because the uv can call the main thread only once, even there are 
	// multiply uv_async_send calls.
	// We then provide the private queue inside it.

	uv_async_send(&async);
}

void HandleEventQueue() {

}