#include "CtpPrice.h"

using v8::Local;
using v8::Object;
using namespace xisca::bindings::javascript;

void InitAll(Local<Object> exports) {
	CtpPrice::Init(exports);
}

NODE_MODULE(ctp, InitAll)