#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include "LogitechMark.h"

class DibSurface
{
public:
    DibSurface(HDC reference, int width, int height)
        : m_width(width), m_height(height)
    {
        if (reference == nullptr || width <= 0 || height <= 0) return;
        m_dc = CreateCompatibleDC(reference);
        if (m_dc == nullptr) return;
        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = width;
        bi.bmiHeader.biHeight = -height;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        m_bitmap = CreateDIBSection(m_dc, &bi, DIB_RGB_COLORS, reinterpret_cast<void**>(&m_pixels), nullptr, 0);
        if (m_bitmap == nullptr || m_pixels == nullptr) return;
        m_previous = SelectObject(m_dc, m_bitmap);
        std::memset(m_pixels, 0, static_cast<size_t>(width) * static_cast<size_t>(height) * sizeof(uint32_t));
    }
    ~DibSurface()
    {
        if (m_dc && m_previous) SelectObject(m_dc, m_previous);
        if (m_bitmap) DeleteObject(m_bitmap);
        if (m_dc) DeleteDC(m_dc);
    }
    bool IsValid() const { return m_dc && m_bitmap && m_pixels; }
    HDC Dc() const { return m_dc; }
    uint32_t* Pixels() const { return m_pixels; }
    int Width() const { return m_width; }
    int Height() const { return m_height; }
private:
    int m_width{}, m_height{};
    HDC m_dc{};
    HBITMAP m_bitmap{};
    HGDIOBJ m_previous{};
    uint32_t* m_pixels{};
};

inline uint32_t PremultipliedPixel(COLORREF color, uint8_t alpha)
{
    uint32_t r = GetRValue(color) * alpha / 255U;
    uint32_t g = GetGValue(color) * alpha / 255U;
    uint32_t b = GetBValue(color) * alpha / 255U;
    return static_cast<uint32_t>(alpha) << 24 | r << 16 | g << 8 | b;
}

inline void BlendSurface(CDC* target, const CRect& dest, const DibSurface& source)
{
    if (!target || dest.IsRectEmpty() || !source.IsValid()) return;
    BLENDFUNCTION blend{};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    AlphaBlend(target->GetSafeHdc(), dest.left, dest.top, dest.Width(), dest.Height(),
        source.Dc(), 0, 0, source.Width(), source.Height(), blend);
}

// Same technique as DrawTranslucentPolygon in better-trafficmonitor-ai-usage-plugin.
inline void DrawTranslucentRect(CDC* pDC, const CRect& rect, COLORREF color, uint8_t opacity)
{
    if (!pDC || rect.IsRectEmpty()) return;
    DibSurface surface(pDC->GetSafeHdc(), rect.Width(), rect.Height());
    if (!surface.IsValid()) return;
    uint32_t pixel = PremultipliedPixel(color, opacity);
    for (int i = 0; i < surface.Width() * surface.Height(); ++i)
        surface.Pixels()[i] = pixel;
    BlendSurface(pDC, rect, surface);
}

// Logitech mark: white "C" with checkmark, drawn into a 32-bit DIB so
// AlphaBlend produces the same semi-transparent look as the other plugins.
inline void DrawLogitechMark(CDC* pDC, const CRect& rect, COLORREF color)
{
    if (!pDC || rect.IsRectEmpty()) return;
    DibSurface surface(pDC->GetSafeHdc(), LOGO_MARK_SIZE, LOGO_MARK_SIZE);
    if (!surface.IsValid()) return;
    for (int i = 0; i < LOGO_MARK_SIZE * LOGO_MARK_SIZE; ++i)
        surface.Pixels()[i] = PremultipliedPixel(color, LOGO_MARK_ALPHA[i]);
    BlendSurface(pDC, rect, surface);
}