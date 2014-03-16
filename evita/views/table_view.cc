// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_view.h"

#include <commctrl.h>
#include <algorithm>

#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
// TODO(yosi) We should not use |dom::Buffer|.
#include "evita/dom/text/buffer.h"
#include "evita/dom/text/document.h"
#include "evita/editor/dom_lock.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/controls/table_control.h"
#include "evita/views/icon_cache.h"
#include "evita/views/table_view_model.h"
#include "evita/vi_Frame.h"

extern HINSTANCE g_hInstance;

namespace views {

namespace {

float CellWidth(const TableViewModel::Cell& cell) {
  // TODO(yosi) We should get character width from ListView control.
  const auto kCharWidth = 6.0f;
  const auto kLeftMargin = 5.0f;
  const auto kRightMargin = 5.0f;
  return cell.text().length() * kCharWidth + kLeftMargin + kRightMargin;
}

std::vector<ui::TableColumn> BuildColumns(const TableViewModel::Row* row) {
  // TODO(yosi) We should specify minium/maximum column width from model.
  auto min_width = 200.0f;
  auto max_width = 300.0f;
  std::vector<ui::TableColumn> columns;
  for (auto& cell : row->cells()) {
    ui::TableColumn column;
    column.alignment = ui::TableColumn::Alignment::Left;
    column.column_id = static_cast<int>(columns.size());
    column.text = cell.text();
    column.width = std::max(std::min(CellWidth(cell), max_width), min_width);
    columns.push_back(column);
    min_width = 0.0f;
  }
  return std::move(columns);
}

void NotifyModelChanges(ui::TableModelObserver* observer,
                        const TableViewModel* new_model,
                        const TableViewModel* old_model) {
  std::vector<TableViewModel::Row*> removed_rows;
  auto max_row_id = 0;
  for (auto old_row : old_model->rows()) {
    max_row_id = std::max(max_row_id, old_row->row_id());
    if (auto new_row = new_model->FindRow(old_row->key())) {
      new_row->set_row_id(old_row->row_id());
      if (*new_row != *old_row)
        observer->DidChangeRow(new_row->row_id());
    } else {
      observer->DidRemoveRow(old_row->row_id());
      removed_rows.push_back(old_row);
    }
  }
  for (auto new_row : new_model->rows()) {
    if (new_row->row_id())
      continue;
    if (removed_rows.empty()) {
      ++max_row_id;
      new_row->set_row_id(max_row_id);
    } else {
      new_row->set_row_id(removed_rows.back()->row_id());
      removed_rows.pop_back();
    }
    observer->DidAddRow(new_row->row_id());
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableView
//
TableView::TableView(WindowId window_id, dom::Document* document)
    : ContentWindow(window_id),
      control_(nullptr),
      document_(document),
      model_(new TableViewModel()),
      should_update_model_(true) {
}

TableView::~TableView() {
  document_->buffer()->RemoveObserver(this);
}

std::vector<int> TableView::GetRowStates(
    const std::vector<base::string16>& keys) const {
  std::vector<int> states;
  for (auto key : keys) {
    auto const row = model_->FindRow(key);
    auto const state = row ? control_->GetRowState(row->row_id()) : 0;
    states.push_back(state);
  }
  return std::move(states);
}

void TableView::UpdateControl(std::unique_ptr<TableViewModel> new_model) {
  if (*new_model->header_row() == *model_->header_row()) {
    NotifyModelChanges(control_, new_model.get(), model_.get());
  } else {
    if (control_) {
      control_->DestroyWidget();
      control_ = nullptr;
    }
    columns_ = std::move(BuildColumns(new_model->header_row()));
    auto row_id = 0;
    for (auto row : new_model->rows()) {
      ++row_id;
      row->set_row_id(row_id);
    }
  }

  rows_.clear();
  row_map_.clear();
  for (auto row : new_model->rows()) {
    rows_.push_back(row);
    DCHECK(row->row_id());
    row_map_[row->row_id()] = row;
  }

  model_ = std::move(new_model);
  if (control_)
    return;

  // Adjust column width by contents
  for (auto row : model_->rows()) {
    auto column_runner = columns_.begin();
    for (auto& cell : row->cells()) {
      column_runner->width = std::max(column_runner->width, CellWidth(cell));
      ++column_runner;
    }
  }

  // Extend the right most column to fill window.
  auto width = 0.0f;
  for (auto& column : columns_) {
    width += column.width;
  }
  if (width < rect().width())
    columns_.back().width = rect().width() - width;

  control_ = new ui::TableControl(columns_, this, this);
  AppendChild(control_);
  control_->Realize(rect());
  control_->Show();
}

std::unique_ptr<TableViewModel> TableView::UpdateModelIfNeeded() {
  if (!should_update_model_)
    return std::unique_ptr<TableViewModel>();
  UI_ASSERT_DOM_LOCKED();
  should_update_model_ = false;
  std::unique_ptr<TableViewModel> model(new TableViewModel());
  auto const buffer = document_->buffer();
  auto position = buffer->ComputeEndOfLine(0);
  auto header_line = buffer->GetText(0, position);
  model->SetHeaderRow(header_line);
  for (;;) {
    ++position;
    if (position == buffer->GetEnd())
      break;
    auto const start = position;
    position = buffer->ComputeEndOfLine(position);
    auto const line = buffer->GetText(start, position);
    model->AddRow(line);
  }
  return std::move(model);
}

// text::BufferMutationObserver
void TableView::DidDeleteAt(Posn, size_t) {
  should_update_model_ = true;
}

void TableView::DidInsertAt(Posn, size_t) {
  should_update_model_ = true;
}

// views::ContentWindow
int TableView::GetIconIndex() const {
  return IconCache::instance()->GetIconForFileName(L"foo.txt");
}

// ui::TableControlObserver
void TableView::OnKeyPressed(const ui::KeyboardEvent& event) {
  ContentWindow::OnKeyPressed(event);
}

void TableView::OnMousePressed(const ui::MouseEvent& event) {
  ContentWindow::OnMousePressed(event);
}

void TableView::OnSelectionChanged() {
}

// ui::TableModel
int TableView::GetRowCount() const {
  return static_cast<int>(model_->row_count());
}

int TableView::GetRowId(int index) const {
  return rows_[static_cast<size_t>(index)]->row_id();
}

base::string16 TableView::GetCellText(int row_id, int column_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return base::string16();
  }
  return present->second->cell(static_cast<size_t>(column_id)).text();
}

// views::ContentWindow
base::string16 TableView::GetTitle() const {
  return L"*document list*";
}

void TableView::MakeSelectionVisible() {
}

void TableView::Redraw() {
  UI_ASSERT_DOM_LOCKED();
  auto new_model = UpdateModelIfNeeded();
  if (!new_model)
    return;
  UpdateControl(std::move(new_model));
}

void TableView::UpdateStatusBar() const {
  std::vector<base::string16> texts {
    base::StringPrintf(L"%d documents", GetRowCount())
  };
  Frame::FindFrame(*this)->SetStatusBar(texts);
}

// ui::Widget
void TableView::DidRealize() {
  ContentWindow::DidRealize();
  document_->buffer()->AddObserver(this);
}

// Resize |ui::TableControl| to cover all client area.
void TableView::DidResize() {
  ContentWindow::DidResize();
  if (control_)
    control_->ResizeTo(rect());
}

// views::Window
bool TableView::OnIdle(int) {
  if (!is_shown())
    return false;
  auto new_model = UpdateModelIfNeeded();
  if (new_model)
    UpdateControl(std::move(new_model));
  if (has_focus())
    control_->RequestFocus();
  return false;
}

}  // namespace views
