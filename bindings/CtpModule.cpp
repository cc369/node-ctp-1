#include "CtpModule.h"

using namespace v8;
using namespace xisca::bindings;
using namespace xisca::bindings::javascript;

#define DEF_STR(strk) v8::Persistent<v8::String> javascript::gstr##strk;

DEF_STR(TradingDay);
DEF_STR(UserProductInfo);
DEF_STR(InterfaceProductInfo);
DEF_STR(ProtocolInfo);
DEF_STR(MacAddress);
DEF_STR(OneTimePassword);
DEF_STR(ClientIPAddress);
DEF_STR(LoginRemark);
DEF_STR(BrokerID);
DEF_STR(Password);
DEF_STR(UserID);

DEF_STR(LoginTime);
DEF_STR(SystemName);
DEF_STR(FrontID);
DEF_STR(SessionID);
DEF_STR(MaxOrderRef);
DEF_STR(SHFETime);
DEF_STR(DCETime);
DEF_STR(CZCETime);
DEF_STR(FFEXTime);
DEF_STR(INETime);

//DECL_STR(connect);
DEF_STR(code);
DEF_STR(reason);
DEF_STR(requestId);
DEF_STR(last);

DEF_STR(InstrumentID);
DEF_STR(ExchangeID);
DEF_STR(ExchangeInstID);
DEF_STR(ProductID);

void javascript::intializeStrings(Isolate *isolate) {
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

	INIT_STR(InstrumentID);
	INIT_STR(ExchangeID);
	INIT_STR(ExchangeInstID);
	INIT_STR(ProductID);
	//INIT_STR(login);
	//INIT_STR(disconnect);
	//INIT_STR(hbWarning);
}

// Persitent str lists.
void javascript::initString(Persistent<String> &persistent, Isolate *isolate, const char* str) {
	persistent.Reset(isolate, String::NewFromUtf8(isolate, str));
}

int javascript::optAssignString(Isolate *isolate,
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

int javascript::reqAssignString(Isolate *isolate,
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

void javascript::assignToObject(Isolate *isolate,
	Local<Object> &object,
	const char *ptrField,
	const char *ptrSource) {
	// It has content.
	if (*ptrSource != '\0') {
		object->Set(String::NewFromUtf8(isolate, ptrField), String::NewFromUtf8(isolate, ptrSource));
	}
}
