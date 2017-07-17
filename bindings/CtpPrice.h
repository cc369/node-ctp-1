#pragma once

// Inlcude the uv
#include <uv.h>

#include <node.h>
#include <node_object_wrap.h>

#include <ThostFtdcMdApi.h>

#include "readerwriterqueue.h"

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

		int AssertInitialized(v8::Isolate *isolate);

		// v8::Isolate *mp_isolate;
		//void SetIsolate(v8::Isolate *isolate);
		//v8::Isolate *GetIsolate();

		uv_async_t async;
		/*uv_async_t m_async_wait;*/
		bool mb_init;



		// Event handlers
		v8::Persistent<v8::Function> m_handler;
	public:
		virtual void OnFrontConnected() override;
		virtual void OnFrontDisconnected(int reason) override;
		virtual void OnHeartBeatWarning(int nTimeLapse) override;
		virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, 
			CThostFtdcRspInfoField *pRspInfo, 
			int nRequestID, 
			bool bIsLast) override;

		virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
		virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	public:
		/**
		 * Always run this functio in main thread of Javascript.
		 *
		 * Parse and trigger target javascript event
		 */
		void HandleEventQueue();

	public:
		struct ctp_message {
			int error_code;
			std::string error;
			int request_id;
			bool is_last;

			// Message type
			int type;

			// Data pointer
			void *pointer;
		};

	private:
		moodycamel::ReaderWriterQueue<ctp_message> queue;

	private:
#define DECL_EVENT(event) void HandleEvent##event(v8::Isolate *isolate, v8::Local<v8::Function> &cb, ctp_message *msg)
		DECL_EVENT(Connect);
		DECL_EVENT(Disconnect);
		DECL_EVENT(HeartBeatWarning);
		DECL_EVENT(Login);
		DECL_EVENT(Error);
		DECL_EVENT(Subscribe);
	};
}

END_XISCA_BINDINGS