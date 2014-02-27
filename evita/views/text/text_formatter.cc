// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/text_formatter.h"

#include "base/logging.h"
#include "evita/css/style.h"
#include "evita/css/style_resolver.h"
#include "evita/css/style_selector.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"

namespace views {
namespace rendering {

namespace {

const float cxLeftMargin = 10.0f;
const int k_nTabWidth = 4;

float AlignHeightToPixel(const gfx::Graphics& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Graphics&, float width) {
  return width;
}

inline char16 toxdigit(int k) {
  if (k <= 9)
    return static_cast<char16>(k + '0');
 return static_cast<char16>(k - 10 + 'A');
}

Font* GetFont(const gfx::Graphics& gfx, const css::Style& style) {
  return FontSet::Get(gfx, style)->FindFont(gfx, 'x');
}

RenderStyle GetRenderStyle(const gfx::Graphics& gfx,
                           const css::Style& style) {
  return RenderStyle(style, GetFont(gfx, style));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EnumCI
//  Enumerator for characters and interval
//
class TextFormatter::EnumCI {
  private: Posn m_lBufEnd;
  private: Posn m_lBufStart;
  private: Posn m_lPosn;
  private: text::Buffer* m_pBuffer;
  private: text::Interval* m_pInterval;
  private: char16 m_rgwch[80];

  public: EnumCI(text::Buffer *pBuffer, Posn lPosn)
      : m_pBuffer(pBuffer),
        m_lPosn(lPosn) {
    m_pInterval = m_pBuffer->GetIntervalAt(m_lPosn);
    DCHECK(m_pInterval);
    fill();
  }

  public: const css::StyleResolver* style_resolver() const {
    return m_pBuffer->style_resolver();
  }

  public: bool AtEnd() const {
    return m_lBufStart == m_lBufEnd;
  }

  private: void fill() {
    auto const cwch = m_pBuffer->GetText(
        m_rgwch, m_lPosn, static_cast<Posn>(m_lPosn + lengthof(m_rgwch)));

    m_lBufStart = m_lPosn;
    m_lBufEnd   = m_lPosn + cwch;
  }

  public: char16 GetChar() const {
    if (AtEnd())
      return 0;
    DCHECK_GE(m_lPosn, m_lBufStart);
    DCHECK_LT(m_lPosn, m_lBufEnd);
    return m_rgwch[m_lPosn - m_lBufStart];
  }

  public: Posn GetPosn() const { return m_lPosn; }

  public: const css::Style& GetStyle() const {
    if (AtEnd())
      return m_pBuffer->GetDefaultStyle();
    DCHECK(m_pInterval);
    return m_pInterval->GetStyle();
  }

  public: void Next() {
    if (AtEnd())
      return;
    m_lPosn += 1;
    if (m_lPosn >= m_lBufEnd)
      fill();

    if (m_lPosn >= m_pInterval->GetEnd()) {
      text::Interval* pNext = m_pInterval->GetNext();
      if (pNext)
        m_pInterval = pNext;
    }
  }
};

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(const gfx::Graphics& gfx, TextBlock* text_block,
                             text::Buffer* buffer, Posn lStart,
                             const Selection& selection)
    : default_render_style_(GetRenderStyle(gfx, buffer->GetDefaultStyle())),
      default_style_(buffer->GetDefaultStyle()),
      m_gfx(gfx),
      selection_(selection), text_block_(text_block),
      m_oEnumCI(new EnumCI(buffer, lStart)) {
  DCHECK(!text_block_->rect().empty());
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::Format() {
  DCHECK(!text_block_->rect().empty());
  for (;;) {
    auto const pLine = FormatLine();
    DCHECK_GT(pLine->rect().height(), 0.0f);

    text_block_->Append(pLine);

    // Line must have at least one cell other than filler.
    DCHECK_GE(pLine->GetEnd(), pLine->GetStart());

    if (text_block_->GetHeight() >= text_block_->height()) {
      // TextBlock is filled up with lines.
      break;
    }

    if (auto const marker_cell = pLine->cells().back()->as<MarkerCell>()) {
      if (marker_cell->marker_name() == TextMarker::EndOfDocument) {
        // We have no more contents.
        break;
      }
    }
  }

  text_block_->Finish();
}

// Returns true if more contents is avaialble, otherwise returns false.
TextLine* TextFormatter::FormatLine() {
  DCHECK(!text_block_->rect().empty());
  auto const pLine = new TextLine();
  pLine->set_start(m_oEnumCI->GetPosn());

  auto x = text_block_->left();
  auto descent = 0.0f;
  auto ascent  = 0.0f;

  Cell* pCell;

  // Left margin
  {
    auto const cyMinHeight = 1.0f;

    pCell = new FillerCell(default_render_style_, cxLeftMargin, cyMinHeight);
    pLine->AddCell(pCell);
    x += cxLeftMargin;
  }

  for (;;) {
    if (m_oEnumCI->AtEnd()) {
      pCell = formatMarker(TextMarker::EndOfDocument);
      break;
    }

    auto const wch = m_oEnumCI->GetChar();

    if (wch == 0x0A) {
      pCell = formatMarker(TextMarker::EndOfLine);
      m_oEnumCI->Next();
      break;
    }

    auto const cx = pCell->m_cx;

    pCell = formatChar(pCell, x, wch);
    if (!pCell) {
      pCell = formatMarker(TextMarker::LineWrap);
      break;
    }

    m_oEnumCI->Next();

    if (pLine->last_cell() == pCell) {
      x -= cx;
    } else {
      pLine->AddCell(pCell);
    }

    x += pCell->m_cx;
    descent = std::max(pCell->GetDescent(), descent);
    ascent  = std::max(pCell->GetHeight() - pCell->GetDescent(), ascent);
  }

  // We have at least one cell.
  //   o end of buffer: End-Of-Buffer MarkerCell
  //   o end of line:   End-Of-Line MarkerCell
  //   o wrapped line:  Warp MarkerCEll
  DCHECK(pCell);
  pLine->AddCell(pCell);

  x += pCell->m_cx;
  descent = std::max(pCell->GetDescent(), descent);
  ascent  = std::max(pCell->GetHeight() - pCell->GetDescent(), ascent);

  pLine->Fix(text_block_->left(), text_block_->top() + text_block_->GetHeight(),
             ascent, descent);

  return pLine;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter::formatChar
//
Cell* TextFormatter::formatChar(Cell* pPrev, float x, char16 wch) {
  auto const lPosn = m_oEnumCI->GetPosn();
  auto style = m_oEnumCI->GetStyle();

  if (lPosn >= selection_.start && lPosn < selection_.end) {
    auto& selection_style = m_oEnumCI->style_resolver()->
        ResolveWithoutDefaults(selection_.active ?
            css::StyleSelector::active_selection() :
            css::StyleSelector::inactive_selection());
    style.OverrideBy(selection_style);
  }

  if (0x09 == wch) {
    auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'x');
    auto const cxTab = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(' ')) *
                          k_nTabWidth;
    auto const x2 = (x + cxTab - cxLeftMargin) / cxTab * cxTab;
    auto const cx = (x2 + cxLeftMargin) - x;
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x2 + cxM > text_block_->right())
      return nullptr;

    return new MarkerCell(RenderStyle(style, pFont), cx,
        AlignHeightToPixel(m_gfx, pFont->height()), lPosn, TextMarker::Tab);
  }

  auto const pFont = wch < 0x20 ?
      nullptr :
      FontSet::Get(m_gfx, style)->FindFont(m_gfx, wch);

  if (!pFont) {
    auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'u');
    base::string16 string;

    if (wch < 0x20) {
      string.push_back('^');
      string.push_back(static_cast<base::char16>(wch + 0x40));
    } else {
      string.push_back('u');
      string.push_back(toxdigit((wch >> 12) & 15));
      string.push_back(toxdigit((wch >> 8) & 15));
      string.push_back(toxdigit((wch >> 4) & 15));
      string.push_back(toxdigit((wch >> 0) & 15));
    }

    auto const cxUni = 6.0f + AlignWidthToPixel(m_gfx,
                                                pFont->GetTextWidth(string));
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x + cxUni + cxM > text_block_->right())
      return nullptr;

    auto const height = AlignHeightToPixel(m_gfx, pFont->height());
    return new UnicodeCell(RenderStyle(style, pFont), cxUni, height, lPosn,
                           string);
  }

  auto const cx = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(wch));
  RenderStyle render_style(style, pFont);
  if (pPrev) {
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (x + cx + cxM > text_block_->right()) {
      // We doesn't have enough room for a char in the line.
      return nullptr;
    }

    if (pPrev->Merge(render_style, cx)) {
      pPrev->as<TextCell>()->AddChar(wch);
      return pPrev;
    }
  }

  auto const height = AlignHeightToPixel(m_gfx, pFont->height());
  return new TextCell(render_style, cx, height, lPosn, base::string16(1u, wch));
}

Cell* TextFormatter::formatMarker(TextMarker marker_name) {
  auto const lPosn = m_oEnumCI->GetPosn();
  auto style = m_oEnumCI->GetStyle();
  style.OverrideBy(m_oEnumCI->style_resolver()->Resolve(
      css::StyleSelector::end_of_line_marker()));

  if (lPosn >= selection_.start && lPosn < selection_.end) {
    auto& selection_style = m_oEnumCI->style_resolver()->
        ResolveWithoutDefaults(selection_.active ?
            css::StyleSelector::active_selection() :
            css::StyleSelector::inactive_selection());
    style.OverrideBy(selection_style);
  }

  auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'x');
  auto const width = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('x'));
  auto const height = AlignHeightToPixel(m_gfx, pFont->height());
  return new MarkerCell(RenderStyle(style, pFont), width, height,
                        m_oEnumCI->GetPosn(), marker_name);
}

} // namespace rendering
} // namespace views
