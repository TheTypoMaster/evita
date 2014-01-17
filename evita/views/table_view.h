// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_view_table_view_h)
#define INCLUDE_view_table_view_h

#include "evita/content/content_window.h"

#include <memory>

#include "base/basictypes.h"
#include "evita/gc/member.h"
#include "evita/view/window_id.h"

namespace dom {
class Document;
}

namespace view {

class TableModel;

class TableView : public CommandWindow_<TableView, content::ContentWindow> {
  DECLARE_CASTABLE_CLASS(TableView, content::ContentWindow);

  private: typedef content::ContentWindow BaseWindow;

  private: gc::Member<dom::Document> document_;
  private: HWND list_view_;
  private: std::unique_ptr<TableModel> model_;
  private: int modified_tick_;

  public: TableView(WindowId window_id, dom::Document* document);
  public: virtual ~TableView();

  private: std::unique_ptr<TableModel> CreateModel();
  private: void Redraw();

  // content::ContentWindow
  private: virtual base::string16 GetTitle(size_t max_length) const;
  private: virtual Command::KeyBindEntry* MapKey(uint key_code) override;
  private: virtual void MakeSelectionVisible() override;
  private: virtual void UpdateStatusBar() const override;

  // widgets::Widget
  private: virtual void Hide() override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  private: virtual void Show() override;
  private: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(TableView);
};

}  // namespace view

#endif //!defined(INCLUDE_view_table_view_h)
