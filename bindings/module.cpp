#include "ctp.h"

using v8::Local;
using v8::Object;
using xisca::bindings::javascript;

void InitAll(Local<Object> exports) {
	CtpWrapper::Init(exports);
}

NODE_MODULE(ctp, InitAll)