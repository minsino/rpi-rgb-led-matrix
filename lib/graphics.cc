// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "graphics.h"
#include "utf8-internal.h"
#include <stdlib.h>
#include <functional>

namespace rgb_matrix {
  
Color HsvToRGB(HSVColor hsv)
{
  Color rgb(hsv.v, hsv.v, hsv.v); // Default to case where hsv.s == 0
  unsigned char region, remainder, p, q, t;

  if (hsv.s == 0) return rgb;

  region = hsv.h / 43;
  remainder = (hsv.h - (region * 43)) * 6; 

  p = (hsv.v * (255 - hsv.s)) >> 8;
  q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
  t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
      case 0:
          rgb.r = hsv.v; rgb.g = t; rgb.b = p;
          break;
      case 1:
          rgb.r = q; rgb.g = hsv.v; rgb.b = p;
          break;
      case 2:
          rgb.r = p; rgb.g = hsv.v; rgb.b = t;
          break;
      case 3:
          rgb.r = p; rgb.g = q; rgb.b = hsv.v;
          break;
      case 4:
          rgb.r = t; rgb.g = p; rgb.b = hsv.v;
          break;
      default:
          rgb.r = hsv.v; rgb.g = p; rgb.b = q;
          break;
  }

  return rgb;
}
  
int DrawText(Canvas *c, const Font &font,
             int x, int y, const Color &color,
             const char *utf8_text) {
  return DrawText(c, font, x, y, color, NULL, utf8_text);
}

int DrawText(Canvas *c, const Font &font,
             int x, int y, const Color &color, const Color *background_color,
             const char *utf8_text, int extra_spacing) {
  const int start_x = x;
  while (*utf8_text) {
    const uint32_t cp = utf8_next_codepoint(utf8_text);
    x += font.DrawGlyph(c, x, y, color, background_color, cp);
    x += extra_spacing;
  }
  return x - start_x;
}

// There used to be a symbol without the optional extra_spacing parameter. Let's
// define this here so that people linking against an old library will still
// have their code usable. Now: 2017-06-04; can probably be removed in a couple
// of months.
int DrawText(Canvas *c, const Font &font,
             int x, int y, const Color &color, const Color *background_color,
             const char *utf8_text) {
  return DrawText(c, font, x, y, color, background_color, utf8_text, 0);
}

int VerticalDrawText(Canvas *c, const Font &font, int x, int y,
                     const Color &color, const Color *background_color,
                     const char *utf8_text, int extra_spacing) {
  const int start_y = y;
  while (*utf8_text) {
    const uint32_t cp = utf8_next_codepoint(utf8_text);
    font.DrawGlyph(c, x, y, color, background_color, cp);
    y += font.height() + extra_spacing;
  }
  return y - start_y;
}

void DrawCircle(Canvas *c, int x0, int y0, int radius, const Color &color) {
  int x = radius, y = 0;
  int radiusError = 1 - x;

  while (y <= x) {
    c->SetPixel(x + x0, y + y0, color.r, color.g, color.b);
    c->SetPixel(y + x0, x + y0, color.r, color.g, color.b);
    c->SetPixel(-x + x0, y + y0, color.r, color.g, color.b);
    c->SetPixel(-y + x0, x + y0, color.r, color.g, color.b);
    c->SetPixel(-x + x0, -y + y0, color.r, color.g, color.b);
    c->SetPixel(-y + x0, -x + y0, color.r, color.g, color.b);
    c->SetPixel(x + x0, -y + y0, color.r, color.g, color.b);
    c->SetPixel(y + x0, -x + y0, color.r, color.g, color.b);
    y++;
    if (radiusError<0){
      radiusError += 2 * y + 1;
    } else {
      x--;
      radiusError+= 2 * (y - x + 1);
    }
  }
}

void DrawLine(Canvas *c, int x0, int y0, int x1, int y1, const Color &color) {
  int dy = y1 - y0, dx = x1 - x0, gradient, x, y, shift = 0x10;

  if (abs(dx) > abs(dy)) {
    // x variation is bigger than y variation
    if (x1 < x0) {
      std::swap(x0, x1);
      std::swap(y0, y1);
    }
    gradient = (dy << shift) / dx ;

    for (x = x0 , y = 0x8000 + (y0 << shift); x <= x1; ++x, y += gradient) {
      c->SetPixel(x, y >> shift, color.r, color.g, color.b);
    }
  } else if (dy != 0) {
    // y variation is bigger than x variation
    if (y1 < y0) {
      std::swap(x0, x1);
      std::swap(y0, y1);
    }
    gradient = (dx << shift) / dy;
    for (y = y0 , x = 0x8000 + (x0 << shift); y <= y1; ++y, x += gradient) {
      c->SetPixel(x >> shift, y, color.r, color.g, color.b);
    }
  } else {
    c->SetPixel(x0, y0, color.r, color.g, color.b);
  }
}

}//namespace
