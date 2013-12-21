// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_win_native_window_h)
#define INCLUDE_base_win_native_window_h

#include "base/base_export.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include <memory>

namespace base {
namespace win {

struct Point;
struct Size;

interface BASE_EXPORT MessageDelegate {
  virtual ~MessageDelegate();
  virtual LRESULT WindowProc(uint message, WPARAM wParam, LPARAM lParam) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// NaitiveWindow
//
class BASE_EXPORT NaitiveWindow {
  protected: HWND hwnd_;
  private: MessageDelegate* message_delegate_;

  private: explicit NaitiveWindow(const MessageDelegate& message_delegate);

  // For MessageDelegate-less naitive window.
  protected: explicit NaitiveWindow();

  // Make destructor of NaitiveWindow for std::unique_ptr<T>. You should not
  // call |delete| for NaitiveWindow.
  public: virtual ~NaitiveWindow();

  public: operator HWND() const {
    DCHECK(hwnd_);
    return hwnd_;
  }

  public: bool operator==(const NaitiveWindow* other) const {
    return this == other;
  }

  public: bool operator==(HWND hwnd) const {
    DCHECK(hwnd_);
    return hwnd_ == hwnd;
  }

  public: bool operator!=(const NaitiveWindow* other) const {
    return this != other;
  }

  public: bool operator!=(HWND hwnd) const {
    DCHECK(hwnd_);
    return hwnd_ == hwnd;
  }

  // [C]
  public: static std::unique_ptr<NaitiveWindow> Create(
      const MessageDelegate& message_delegate);
  public: static std::unique_ptr<NaitiveWindow> Create();
  public: bool CreateWindowEx(DWORD dwExStyle, DWORD dwStyle,
                              const base::char16* title, HWND parent_hwnd,
                              const Point& left_top,
                              const Size& size);

  // [D]
  public: void Destroy();
  public: LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  // [I]
  public: static void Init(HINSTANCE hInstance, HINSTANCE hResouce);
  public: static void Init(HINSTANCE hInstance);
  public: virtual bool IsRealized() const { return hwnd_; }

  // [M]
  protected: static NaitiveWindow* MapHwnToNaitiveWindow(HWND);

  // [S]
  public: LRESULT SendMessage(uint uMsg, WPARAM wParam = 0,
                              LPARAM lParam = 0) {
    return ::SendMessage(hwnd_, uMsg, wParam, lParam);
  }

  // [W]
  private: static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam);

  public: virtual LRESULT WindowProc(UINT message, WPARAM wParam,
                                     LPARAM lParam);

  DISALLOW_COPY_AND_ASSIGN(NaitiveWindow);
};

} // namespace win
} // namespace base

#endif //!defined(INCLUDE_base_win_native_window_h)
