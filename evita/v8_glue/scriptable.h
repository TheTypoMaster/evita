// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_scriptable_h)
#define INCLUDE_evita_v8_glue_scriptable_h

#include "evita/gc/collectable.h"
#include "evita/v8_glue/wrapper_info.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
END_V8_INCLUDE

namespace v8_glue {

namespace internal {

void* FromV8Impl(v8::Isolate* isolate,
                 v8::Handle<v8::Value> value,
                 WrapperInfo* info);

}  // namespace internal

// Note: The |AbstractScriptable| class was called |AbstractScriptWrappable|.
// Although, it is too long to fit in line. So, we renamed shorter name.
class AbstractScriptable
    : public gc::Collectable<AbstractScriptable> {
  private: v8::Persistent<v8::Object> wrapper_; // Weak

  protected: AbstractScriptable() = default;
  protected: virtual ~AbstractScriptable();

  public: bool has_script_reference() const {
    return !wrapper_.IsEmpty();
  }

  protected: virtual WrapperInfo* wrapper_info() const = 0;

  protected: static v8::Handle<v8::Function>
      GetConstructorImpl(v8::Isolate* isolate, WrapperInfo* info);

  protected: static v8::Handle<v8::FunctionTemplate> GetFunctionTemplateImpl(
      v8::Isolate* isolate, WrapperInfo* info);

  protected: virtual gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate);

  public: v8::Handle<v8::Object> GetWrapper(v8::Isolate* isolate);

  private: static void WeakCallback(
      const v8::WeakCallbackData<v8::Object, AbstractScriptable>& data);

  DISALLOW_COPY_AND_ASSIGN(AbstractScriptable);
};

template<typename T>
class Scriptable : public AbstractScriptable {
  protected: Scriptable() = default;
  protected: virtual ~Scriptable() = default;

  private: virtual WrapperInfo* wrapper_info() const override {
    return T::static_wrapper_info();
  }

  public: static v8::Handle<v8::Function> GetConstructor(
      v8::Isolate* isolate) {
    return GetConstructorImpl(isolate, T::static_wrapper_info());
  }

  // A helper function to get ObjecTemplateBuilder from
  // AbstractScriptable to avoid v8_glue::WrapperInfo<T> prefix.
  protected: gin::ObjectTemplateBuilder GetEmptyObjectTemplateBuilder(
      v8::Isolate* isolate) {
    return AbstractScriptable::GetObjectTemplateBuilder(isolate);
  }

  DISALLOW_COPY_AND_ASSIGN(Scriptable);
};

}  // namespace v8_glue

namespace gin {

// This converter handles any subclass of Wrappable.
template<typename T>
struct Converter<T*, typename base::enable_if<
    base::is_convertible<T*,
        v8_glue::AbstractScriptable*>::value>::type> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, T* val) {
    return val->GetWrapper(isolate);
  }

  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val, T** out) {
    auto const wrapper_info = T::static_wrapper_info();
    *out = static_cast<T*>(static_cast<v8_glue::AbstractScriptable*>(
        v8_glue::internal::FromV8Impl(isolate, val, wrapper_info)));
    return *out;
  }
};

}  // namespace gin

#endif //!defined(INCLUDE_evita_v8_glue_scriptable_h)
