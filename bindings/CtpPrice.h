#pragma once

#include <node.h>
#include <node_object_wrap.h>

#include <ThostFtdcMdApi.h>

#ifndef NS_XISCA_BINDINGS
#define NS_XISCA_BINDINGS \
	namespace xisca { \
		namespace bindings { \

#define END_XISCA_BINDINGS \
		} \
	}

#endif 

NS_XISCA_BINDINGS

namespace javascript {

	class CtpPrice : public node::ObjectWrap, public CThostFtdcMdSpi {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:
		explicit CtpPrice(const char *pszFlowPath = "", const bool bIsUsingUdp = false, const bool bIsMulticast = false);
		~CtpPrice();

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static v8::Persistent<v8::Function> constructor;

	private:
		static void AddFrontAddress(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddNameServer(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Login(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Subscribe(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Unsubscribe(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void RegisterSpi(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Release(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Init(const v8::FunctionCallbackInfo<v8::Value>& args);

		CThostFtdcMdApi *m_api;
		void RegisterSpi();
		void Release();
		void Init();

		void AssertInitialized(v8::Isolate *isolate);
	};
}

END_XISCA_BINDINGS