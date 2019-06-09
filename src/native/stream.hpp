#ifndef STREAM_H
#define STREAM_H

#include <napi.h>
#include <cuda.h>

class Stream : public Napi::ObjectWrap<Stream>
{
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Value CreateStream(const Napi::CallbackInfo &info);

  Stream(const Napi::CallbackInfo &info);

  CUstream m_stream;

private:
  static Napi::FunctionReference constructor;

  Napi::Value Destroy(const Napi::CallbackInfo &info);
};

#endif
