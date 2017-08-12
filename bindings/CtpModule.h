#pragma once
#ifndef NS_XISCA_BINDINGS
#define NS_XISCA_BINDINGS \
	namespace xisca { \
		namespace bindings { \

#define END_XISCA_BINDINGS \
		} \
	}

#endif 

#if !defined(ZeroMemory) && !defined(_MSVC_VER)
#include <strings.h>
#define ZeroMemory bzero 
#endif

#include <uv.h>
#include <node.h>

#include <float.h>

NS_XISCA_BINDINGS

#define INTERNAL_TINY_BUF_SIZE 64

#define DECL_STR(strk) extern v8::Persistent<v8::String> gstr##strk;
#define GSTR(strk) gstr##strk
#define DECL_EVENT(event) void HandleEvent##event(v8::Isolate *isolate, v8::Local<v8::Function> &cb, ctp_message *msg)
#define INIT_STR(strk) initString(gstr##strk, isolate, #strk)

#define OPT_ASSIGN(strk,apply) if (optAssignString(isolate, apply.strk, sizeof(apply.strk), req, #strk)) { return; }
#define REQ_ASSIGN(strk,apply) if (reqAssignString(isolate, apply.strk, sizeof(apply.strk), req, #strk)) { return; }

namespace javascript {	
	int optAssignString(v8::Isolate *isolate,
		char *ptrDest,
		size_t maxDestLength,
		v8::Local<v8::Object> object,
		const char *ptr);

	int reqAssignString(v8::Isolate *isolate,
		char *ptrDest,
		size_t maxDestLength,
		v8::Local<v8::Object> object,
		const char *ptr);

	void assignToObject(v8::Isolate *isolate,
		v8::Local<v8::Object> &object,
		const char *ptrField,
		const char *ptrSource);

	void initString(v8::Persistent<v8::String> &persistent, v8::Isolate *isolate, const char* str);
	void intializeStrings(v8::Isolate *isolate);

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

	DECL_STR(InstrumentID);
	DECL_STR(ExchangeID);
	DECL_STR(ExchangeInstID);
	DECL_STR(ProductID);
}

END_XISCA_BINDINGS
