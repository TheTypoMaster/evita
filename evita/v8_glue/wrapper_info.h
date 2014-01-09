// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_wrapper_info_h)
#define INCLUDE_evita_v8_glue_wrapper_info_h

#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/gin_embedders.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
#include "gin/public/wrapper_info.h"
END_V8_INCLUDE

namespace v8_glue {

class AbstractScriptable;

// Note: The |WrapperInfo| class was called |ScriptWrapperInfo|. Although, it
// is too long to fit in line. So, we renamed shorter name.
class WrapperInfo {
  protected: typedef gin::ObjectTemplateBuilder ObjectTemplateBuilder;
  protected: typedef v8_glue::AbstractScriptable AbstractScriptable;

  private: gin::GinEmbedder const embedder_;
  private: const char* const class_name_;

  protected: WrapperInfo(const char* class_name);
  public: ~WrapperInfo() = default;

  public: const char* class_name() const { return class_name_; }
  public: gin::GinEmbedder embedder() const { return embedder_; }
  public: virtual WrapperInfo* inherit_from() const;
  public: virtual const char* singleton_name() const;
  public: virtual AbstractScriptable* singleton() const;
  public: gin::WrapperInfo* gin_wrapper_info() {
    return const_cast<gin::WrapperInfo*>(
        reinterpret_cast<const gin::WrapperInfo*>(&embedder_));
  }

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate);
  protected: ObjectTemplateBuilder
      CreateInstanceTemplateBuilder(v8::Isolate* isolate);
  public: static WrapperInfo* From(v8::Handle<v8::Object> object);
  public: v8::Handle<v8::FunctionTemplate> GetOrCreateConstructorTemplate(
      v8::Isolate* isolate);
  public: v8::Handle<v8::ObjectTemplate> GetOrCreateInstanceTemplate(
      v8::Isolate* isolate);
  public: void Install(v8::Isolate* isolate,
                       v8::Handle<v8::ObjectTemplate> global);
  protected: virtual void SetupInstanceTemplate(
    ObjectTemplateBuilder& builder);
  protected: virtual void SetupInstanceTemplate(v8::Isolate* isolate,
    v8::Handle<v8::ObjectTemplate> templ);

  DISALLOW_COPY_AND_ASSIGN(WrapperInfo);
};

template<typename T>
class Installer {
  public: static void Run(v8::Isolate* isolate,
                          v8::Handle<v8::ObjectTemplate> global) {
    T::static_wrapper_info()->Install(isolate, global);
  }
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_wrapper_info_h)
