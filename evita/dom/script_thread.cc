// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_thread.h"

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/synchronization/waitable_event.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/threading/thread.h"
#pragma warning(pop)
#include "evita/dom/lock.h"
#include "evita/dom/public/api_event.h"
#include "evita/dom/script_controller.h"

#define DCHECK_CALLED_ON_HOST_THREAD() \
  DCHECK(thread_checker_.CalledOnValidThread());

#define DCHECK_CALLED_ON_SCRIPT_THREAD() \
  DCHECK(ScriptThread::instance()->CalledOnValidThread())

namespace dom {

namespace {
ScriptThread* script_thread;
}  // namespace

ScriptThread::ScriptThread(ViewDelegate* view_delegate,
                           base::MessageLoop* host_message_loop)
    : view_delegate_(view_delegate),
      host_message_loop_(host_message_loop),
      thread_(new base::Thread("script_thread")),
      view_event_handler_(nullptr) {
  thread_->Start();
  PostTask(FROM_HERE, base::Bind(base::IgnoreResult(&ScriptController::Start),
                                 base::Unretained(this)));
}

ScriptThread::~ScriptThread() {
  script_thread = nullptr;
}

ScriptThread* ScriptThread::instance() {
  DCHECK(script_thread);
  return script_thread;
}

bool ScriptThread::CalledOnValidThread() const {
  return CalledOnScriptThread();
}

bool ScriptThread::CalledOnScriptThread() const {
  return base::MessageLoop::current() == thread_->message_loop();
}

void ScriptThread::Evaluate(
    const base::string16& script_text,
    base::Callback<void(EvaluateResult)> callback) {
  DCHECK_CALLED_ON_HOST_THREAD();
  PostTask(FROM_HERE, base::Bind(&ScriptThread::EvaluateImpl,
      base::Unretained(this), script_text, callback));
}

void ScriptThread::EvaluateImpl(
    base::string16 script_text,
    base::Callback<void(EvaluateResult)> callback) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_) {
    return;
  }

  DOM_AUTO_LOCK_SCOPE();
  auto eval_result = ScriptController::instance()->Evaluate(script_text);
  host_message_loop_->PostTask(FROM_HERE, base::Bind(callback, eval_result));
}

void ScriptThread::PostTask(const tracked_objects::Location& from_here,
                            const base::Closure& task) {
  DCHECK_CALLED_ON_HOST_THREAD();
  thread_->message_loop()->PostTask(from_here, task);
}

void ScriptThread::Start(ViewDelegate* view_delegate,
                         base::MessageLoop* host_message_loop) {
  DCHECK(!script_thread);
  script_thread = new ScriptThread(view_delegate, host_message_loop);
}
// ViewDelegate
#define DEFINE_VIEW_DELEGATE_1(name, type1) \
  void ScriptThread::name(type1 param1) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!host_message_loop_) \
      return; \
    host_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        param1)); \
  }

#define DEFINE_VIEW_DELEGATE_2(name, type1, type2) \
  void ScriptThread::name(type1 param1, type2 param2) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!host_message_loop_) \
      return; \
    host_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        param1, param2)); \
  }

#define DEFINE_VIEW_DELEGATE_3(name, type1, type2, type3) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!host_message_loop_) \
      return; \
    host_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        param1, param2, param3)); \
  }

#define DEFINE_VIEW_DELEGATE_4(name, type1, type2, type3, type4) \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!host_message_loop_) \
      return; \
    host_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        p1, p2, p3, p4)); \
  }

#define DEFINE_VIEW_DELEGATE_5(name, type1, type2, type3, type4, type5) \
  void ScriptThread::name(type1 p1, type2 p2, type3 p3, type4 p4, type5 p5) { \
    DCHECK_CALLED_ON_SCRIPT_THREAD(); \
    if (!host_message_loop_) \
      return; \
    host_message_loop_->PostTask(FROM_HERE, base::Bind( \
        &ViewDelegate::name, \
        base::Unretained(view_delegate_), \
        p1, p2, p3, p4, p5)); \
  }

DEFINE_VIEW_DELEGATE_2(AddWindow, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(ChangeParentWindow, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_1(CreateDialogBox, DialogBoxId)
DEFINE_VIEW_DELEGATE_1(CreateEditorWindow, const EditorWindow*)
DEFINE_VIEW_DELEGATE_2(CreateTableWindow, WindowId, Document*)
DEFINE_VIEW_DELEGATE_1(CreateTextWindow, const TextWindow*)
DEFINE_VIEW_DELEGATE_1(DestroyWindow, WindowId)
DEFINE_VIEW_DELEGATE_2(DoFind, DialogBoxId, text::Direction)
DEFINE_VIEW_DELEGATE_1(FocusWindow, WindowId)
DEFINE_VIEW_DELEGATE_3(GetFilenameForLoad, WindowId, const base::string16&,
                       ViewDelegate::GetFilenameForLoadCallback)
DEFINE_VIEW_DELEGATE_3(GetFilenameForSave, WindowId, const base::string16&,
                       ViewDelegate::GetFilenameForSaveCallback)
DEFINE_VIEW_DELEGATE_2(LoadFile, Document*, const base::string16&)
DEFINE_VIEW_DELEGATE_1(MakeSelectionVisible, WindowId)
DEFINE_VIEW_DELEGATE_5(MessageBox, WindowId, const base::string16&,
                       const::base::string16&, int,
                       MessageBoxCallback)
DEFINE_VIEW_DELEGATE_3(Reconvert, WindowId, text::Posn, text::Posn);
DEFINE_VIEW_DELEGATE_1(RealizeDialogBox, const Form*)
DEFINE_VIEW_DELEGATE_1(RealizeWindow, WindowId)
DEFINE_VIEW_DELEGATE_1(ReleaseCapture, WindowId)
DEFINE_VIEW_DELEGATE_2(SaveFile, Document*, const base::string16&)
DEFINE_VIEW_DELEGATE_1(SetCapture, WindowId)
DEFINE_VIEW_DELEGATE_1(ShowDialogBox, DialogBoxId)
DEFINE_VIEW_DELEGATE_2(SplitHorizontally, WindowId, WindowId)
DEFINE_VIEW_DELEGATE_2(SplitVertically, WindowId, WindowId)

void ScriptThread::ComputeOnTextWindow(WindowId window_id,
                                     TextWindowCompute* data,
                                     base::WaitableEvent* null_event) {
  DCHECK(!null_event);
  DCHECK(data);
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  base::WaitableEvent event(true, false);
  DOM_AUTO_UNLOCK_SCOPE();
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::ComputeOnTextWindow,
      base::Unretained(view_delegate_), window_id,
      base::Unretained(data), base::Unretained(&event)));
  event.Wait();
}

void ScriptThread::GetTableRowStates(WindowId window_id,
    const std::vector<base::string16>& keys, int* states,
    base::WaitableEvent* null_event) {
  DCHECK(!null_event);
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  base::WaitableEvent event(true, false);
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::GetTableRowStates,
      base::Unretained(view_delegate_), window_id, keys,
      base::Unretained(states), base::Unretained(&event)));
  event.Wait();
}

void ScriptThread::RegisterViewEventHandler(
    ViewEventHandler* event_handler) {
  DCHECK_CALLED_ON_SCRIPT_THREAD();
  if (!host_message_loop_)
    return;
  view_event_handler_ = event_handler;
  host_message_loop_->PostTask(FROM_HERE, base::Bind(
      &ViewDelegate::RegisterViewEventHandler,
      base::Unretained(view_delegate_),
      base::Unretained(this)));
}

// ViewEventHandler
#define DEFINE_VIEW_EVENT_HANDLER_0(name) \
  void ScriptThread::name() { \
    DCHECK_CALLED_ON_HOST_THREAD(); \
    DCHECK(view_event_handler_); \
    PostTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_))); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_1(name, type1) \
  void ScriptThread::name(type1 param1) { \
    DCHECK_CALLED_ON_HOST_THREAD(); \
    DCHECK(view_event_handler_); \
    PostTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_2(name, type1, type2) \
  void ScriptThread::name(type1 param1, type2 param2) { \
    DCHECK_CALLED_ON_HOST_THREAD(); \
    DCHECK(view_event_handler_); \
    PostTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_3(name, type1, type2, type3) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3) { \
    DCHECK_CALLED_ON_HOST_THREAD(); \
    DCHECK(view_event_handler_); \
    PostTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2, param3)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_4(name, type1, type2, type3, type4) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3, \
                          type4 param4) { \
    DCHECK_CALLED_ON_HOST_THREAD(); \
    DCHECK(view_event_handler_); \
    PostTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2, param3, param4)); \
  }

#define DEFINE_VIEW_EVENT_HANDLER_5(name, type1, type2, type3, type4, type5) \
  void ScriptThread::name(type1 param1, type2 param2, type3 param3, \
                          type4 param4, type5 param5) { \
    DCHECK_CALLED_ON_HOST_THREAD(); \
    DCHECK(view_event_handler_); \
    PostTask(FROM_HERE, base::Bind( \
        &ViewEventHandler::name, \
        base::Unretained(view_event_handler_), \
        param1, param2, param3, param4, param5)); \
  }

DEFINE_VIEW_EVENT_HANDLER_1(DidDestroyWidget, WindowId)
DEFINE_VIEW_EVENT_HANDLER_2(DidDropWidget, WindowId, WindowId)
DEFINE_VIEW_EVENT_HANDLER_1(DidKillFocus, WindowId)
DEFINE_VIEW_EVENT_HANDLER_1(DidRealizeWidget, WindowId)
DEFINE_VIEW_EVENT_HANDLER_5(DidResizeWidget, WindowId, int, int, int, int)
DEFINE_VIEW_EVENT_HANDLER_1(DidSetFocus, WindowId)
DEFINE_VIEW_EVENT_HANDLER_0(DidStartHost)
DEFINE_VIEW_EVENT_HANDLER_1(DispatchFormEvent, const ApiFormEvent&)
DEFINE_VIEW_EVENT_HANDLER_1(DispatchMouseEvent, const domapi::MouseEvent&)
DEFINE_VIEW_EVENT_HANDLER_2(OpenFile, WindowId, const base::string16&)
DEFINE_VIEW_EVENT_HANDLER_1(QueryClose, WindowId)
DEFINE_VIEW_EVENT_HANDLER_1(RunCallback, base::Closure)

void ScriptThread::WillDestroyHost() {
  DCHECK_CALLED_ON_HOST_THREAD();
  DCHECK(view_event_handler_);
  view_delegate_ = nullptr;
  host_message_loop_ = nullptr;
  PostTask(FROM_HERE, base::Bind(
      &ViewEventHandler::WillDestroyHost,
      base::Unretained(view_event_handler_)));
}

}  // namespace dom
