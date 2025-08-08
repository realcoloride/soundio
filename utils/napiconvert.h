#pragma once
#include <include.h>

static auto napiBool(const Napi::CallbackInfo& info, bool boolean) {
    return Napi::Boolean::New(info.Env(), boolean);
}

static auto napiString(const Napi::CallbackInfo& info, const std::string& string) {
    return Napi::String::New(info.Env(), string);
}

template <typename T>
static auto napiNumber(const Napi::CallbackInfo& info, T number) {
    return Napi::Number::New(info.Env(), number);
}