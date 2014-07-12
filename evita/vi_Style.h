// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_vi_style_h)
#define INCLUDE_evita_vi_style_h

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "gfx/forward.h"
#include "evita/li_util.h"

namespace css {
class Style;
}

namespace gfx {
struct FontProperties;
}

class FontSet;

//////////////////////////////////////////////////////////////////////
//
// Font class
//
class Font {
  private: class Cache;
  friend class Cache;

  private: class FontImpl;

  private: struct SimpleMetrics {
    float ascent;
    float descent;
    float height;
    float fixed_width;
    float underline;
    float underline_thickness;
  };

  private: const std::unique_ptr<FontImpl> font_impl_;
  private: const SimpleMetrics metrics_;

  private: Font(const gfx::FontProperties& properties);
  private: ~Font();

  private: float ascent() const { return metrics_.ascent; }
  public: float descent() const { return metrics_.descent; }
  public: float height() const { return metrics_.height; }
  public: float underline() const { return metrics_.underline; }
  public: float underline_thickness() const {
    return metrics_.underline_thickness;
  }

  // [D]
  public: void DrawText(gfx::Canvas* canvas, const gfx::Brush& text_brush,
                        const gfx::RectF& rect, const base::char16* chars,
                        size_t num_chars) const;
  public: void DrawText(gfx::Canvas* canvas,const gfx::Brush& text_brush,
                        const gfx::RectF& rect,
                        const base::string16& string) const;

  // [G]
  public: static Font* Get(const gfx::FontProperties& properties);
  public: float GetCharWidth(char16) const;
  public: float GetTextWidth(const base::char16* pwch, size_t cwch) const;
  public: float GetTextWidth(const base::string16& string) const;

  // [H]
  public: bool HasCharacter(char16) const;

  DISALLOW_COPY_AND_ASSIGN(Font);
};

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
class FontSet {
  private: class Cache;
  friend class Cache;

  private: std::vector<Font*> fonts_;

  private: FontSet(const std::vector<Font*>& fonts);
  private: ~FontSet();

  public: Font* FindFont(base::char16 sample) const;
  public: static FontSet* Get(const css::Style& style);

  DISALLOW_COPY_AND_ASSIGN(FontSet);
};

#endif //!defined(INCLUDE_evita_vi_style_h)
