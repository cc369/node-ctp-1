#include "CtpPrice.h"
#include "CtpTrade.h"

using v8::Local;
using v8::Object;
using namespace xisca::bindings::javascript;

void InitAll(Local<Object> exports) {
	v8::Isolate *isolate = exports->GetIsolate();
	intializeStrings(isolate);

	CtpPrice::Init(exports);
	CtpTrade::Init(exports);
}

NODE_MODULE(ctp, InitAll)