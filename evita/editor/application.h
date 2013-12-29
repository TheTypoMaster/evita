// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_application_h)
#define INCLUDE_evita_editor_application_h

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4625)
#include "base/callback.h"
#pragma warning(pop)
#include "base/location.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/vi_Frame.h"

class Buffer;
class CommandWindow;
class IoManager;

namespace base {
class MessageLoop;
}

namespace Command {
class Processor;
}

namespace editor {
class DomLock;
}

class Application : public common::Singleton<Application> {
  protected: typedef DoubleLinkedList_<Frame> Frames;
  protected: typedef DoubleLinkedList_<Buffer> Buffers;

  private: NewlineMode newline_mode_;
  private: uint code_page_;
  private: Buffers buffers_;
  private: Frames frames_;
  private: Frame* active_frame_;
  private: int idle_count_;
  private: bool is_quit_;
  private: std::unique_ptr<Command::Processor> command_processor_;
  private: std::unique_ptr<editor::DomLock> dom_lock_;
  private: std::unique_ptr<IoManager> io_manager_;
  private: std::unique_ptr<base::MessageLoop> message_loop_;

  // ctor/dtor
  friend class common::Singleton<Application>;
  private: Application();
  public: ~Application();

  public: const Buffers& buffers() const { return buffers_; }
  public: Buffers& buffers() { return buffers_; }
  public: const Frames& frames() const { return frames_; }
  public: Frames& frames() { return frames_; }
  public: editor::DomLock* dom_lock() const { return dom_lock_.get(); }
  public: const base::string16& title() const;
  public: const base::string16& version() const;

  // FIXME 2007-08-19 yosi@msn.com We should NOT use InternalAddBufer,
  // this method is just for listener.
  public: void InternalAddBuffer(Buffer* buffer);

  // [A]
  public: int Ask(int flags, int format_id, ...);

  // [C]
  public: bool CalledOnValidThread() const;
  public: bool CanExit() const;
  public: Frame* CreateFrame();
  public: Frame* CreateFrame(Buffer* buffer);

  // [D]
  public: Frame* DeleteFrame(Frame* frame);
  private: void DoIdle();

  // [E]
  public: void Execute(CommandWindow* window, uint32 key_code, uint32 repeat);
  public: void Exit(bool);

  // [F]
  public: Buffer* FindBuffer(const char16* name) const;
  public: Frame* FindFrame(HWND hwnd) const;
  public: Pane* FindPane(HWND hwnd, POINT point) const;

  // [G]
  public: Frame* GetActiveFrame() const { return active_frame_; }
  public: uint GetCodePage() const { return code_page_; }
  public: Frame* GetFirstFrame() const { return frames_.GetFirst(); }
  public: HIMAGELIST GetIconList() const;
  public: IoManager* GetIoManager() const { return io_manager_.get(); }
  public: Frame* GetLastFrame() const { return frames_.GetLast(); }
  public: NewlineMode GetNewline() const { return newline_mode_; }
  public: const char16* GetTitle() const;

  // [H]
  public: bool HasMultipleFrames() const {
    return GetFirstFrame() != GetLastFrame();
  }

  // [K]
  public: bool KillBuffer(Buffer* buffer, bool force = false);

  // [L]
  public: Buffer* Load(const char16* filename);

  // [N]
  public: Buffer* NewBuffer(const char16* buffer_name);

  // [O]
  public: bool OnIdle(uint hint);

  // [P]
  public: void PostTask(const tracked_objects::Location& from_here,
                        const base::Closure& task);

  public: void PostDomTask(const tracked_objects::Location& from_here,
                           const base::Closure& task);

  // [R]
  public: Buffer* RenameBuffer(Buffer* buffer, const char16* new_name);
  public: void Run();

  // [S]
  public: bool SaveBuffer(Frame* frame, Buffer* buffer, bool save_as = false);

  public: Frame* SetActiveFrame(Frame* frame) { 
    return active_frame_ = frame;
  }

  public: void ShowMessage(MessageLevel, uint);

  // [T]
  private: bool TryDoIdle();

  DISALLOW_COPY_AND_ASSIGN(Application);
};

#define ASSERT_CALLED_ON_UI_THREAD() \
  DCHECK(Application::instance()->CalledOnValidThread())


#endif //!defined(INCLUDE_evita_editor_application_h)
