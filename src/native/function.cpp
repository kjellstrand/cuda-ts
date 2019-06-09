#include "function.hpp"
#include "memory.hpp"
#include "stream.hpp"
#include "shared.hpp"

Napi::FunctionReference Function::constructor;

Napi::Object Function::Init(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Function", {
                                                         InstanceMethod("launchKernel", &Function::LaunchKernel),
                                                     });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Function", func);

  return exports;
}

Napi::Object Function::New()
{
  return constructor.New({});
}

Function::Function(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Function>(info)
{
}

Napi::Value Function::LaunchKernel(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 8)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Array array = info[0].As<Napi::Array>();

  std::vector<CUdeviceptr *> input(array.Length());
  for (int i = 0; i < (int)array.Length(); i++)
  {
    input[i] = &Napi::ObjectWrap<Memory>::Unwrap(array.Get(i).As<Napi::Object>())->m_ptr;
  }

  CUstream stream = 0;
  if (info[7].IsNumber())
  {
    if (info[7].As<Napi::Number>().Int32Value() != 0)
    {
      Napi::TypeError::New(env, "Invalid stream").ThrowAsJavaScriptException();
      return env.Null();
    }
  }
  else
  {
    stream = (Napi::ObjectWrap<Stream>::Unwrap(info[7].As<Napi::Object>()))->m_stream;
  }

  // Synchronization is not needed as we're using stream 0
  if (!validate(cuLaunchKernel(m_function,
                               info[1].As<Napi::Number>().Int32Value(), // Grid dimensions (block count)
                               info[2].As<Napi::Number>().Int32Value(),
                               info[3].As<Napi::Number>().Int32Value(),
                               info[4].As<Napi::Number>().Int32Value(), // Block dimensions (thread count)
                               info[5].As<Napi::Number>().Int32Value(),
                               info[6].As<Napi::Number>().Int32Value(),
                               0, // Shared mem bytes
                               stream,
                               (void **)input.data(),
                               NULL // Extra
                               ),
                env))
  {
    return env.Undefined();
  }

  return env.Undefined();
}
