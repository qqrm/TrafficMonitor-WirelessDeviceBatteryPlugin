#include "pch.h"
#include "WirelessDeviceBatteryItem.h"
#include "WirelessDeviceBattery.h"
#include "PluginDraw.h"
#include "DeviceBrandMasks.h"
#include "LogitechMark.h"
#include <algorithm>
#include <cmath>

namespace {
    constexpr COLORREF CRIMSON = RGB(220, 38, 38);

    COLORREF BrandColor(int brand)
    {
        switch (brand) {
        case 0: return RGB(0, 212, 170);   // Logitech green
        case 1: return RGB(255, 68, 68);   // Razer red
        case 2: return RGB(0, 255, 156);   // Keychron green
        case 3: return RGB(0, 212, 255);   // Flydigi cyan
        case 4: return RGB(139, 92, 246);  // MCHOSE violet
        default: return RGB(150, 150, 150);
        }
    }

    COLORREF LowBatteryColor(COLORREF base, int percent)
    {
        if (percent < 0) return RGB(150, 150, 150);   // unknown: neutral, never red
        if (percent >= 15) return base;
        if (percent <= 5) return CRIMSON;
        float t = (15.0f - float(percent)) / 10.0f;   // 15 -> 0, 5 -> 1
        auto lerp = [](int a, int b, float k) {
            return int((a + (b - a) * k) + 0.5f);
        };
        return RGB(lerp(GetRValue(base), GetRValue(CRIMSON), t),
                   lerp(GetGValue(base), GetGValue(CRIMSON), t),
                   lerp(GetBValue(base), GetBValue(CRIMSON), t));
    }

    void DrawBrandMark(CDC* pDC, const CRect& rect, int brand, COLORREF color)
    {
        if (!pDC || rect.IsRectEmpty()) return;
        DibSurface surface(pDC->GetSafeHdc(), DEVICE_MARK_SIZE, DEVICE_MARK_SIZE);
        if (!surface.IsValid()) return;
        const std::uint8_t* alpha = nullptr;
        switch (brand) {
        case 0: {
            DibSurface logi(pDC->GetSafeHdc(), LOGO_MARK_SIZE, LOGO_MARK_SIZE);
            if (logi.IsValid()) {
                for (int i = 0; i < LOGO_MARK_SIZE * LOGO_MARK_SIZE; ++i)
                    logi.Pixels()[i] = PremultipliedPixel(color, LOGO_MARK_ALPHA[i]);
                BlendSurface(pDC, rect, logi);
            }
            return;
        }
        case 1: alpha = RAZER_ALPHA; break;
        case 2: alpha = KEYCHRON_ALPHA; break;
        case 3: alpha = FLYDIGI_ALPHA; break;
        case 4: alpha = MCHOSE_ALPHA; break;
        default: return;
        }
        for (int i = 0; i < DEVICE_MARK_SIZE * DEVICE_MARK_SIZE; ++i)
            surface.Pixels()[i] = PremultipliedPixel(color, alpha[i]);
        BlendSurface(pDC, rect, surface);
    }
}

const wchar_t* CWirelessDeviceBatteryItem::GetItemName() const { return L"Wireless Battery"; }
const wchar_t* CWirelessDeviceBatteryItem::GetItemId() const
{
    static wchar_t ids[16][32];
    static bool init = false;
    if (!init) { for (int i = 0; i < 16; ++i) swprintf_s(ids[i], 32, L"WirelessBattery%d", i); init = true; }
    return ids[m_index % 16];
}
const wchar_t* CWirelessDeviceBatteryItem::GetItemLableText() const
{
    const DeviceSnapshot* d = g_plugin.GetDevice(m_index);
    static thread_local std::wstring s;
    s = d ? d->name : L"Battery";
    return s.c_str();
}
const wchar_t* CWirelessDeviceBatteryItem::GetItemValueText() const
{
    const DeviceSnapshot* d = g_plugin.GetDevice(m_index);
    static thread_local std::wstring s;
    if (!d) { s = L""; return s.c_str(); }
    s = d->percent < 0 ? L"?" : std::to_wstring(d->percent) + L"%";
    if (d->charging) s += L" \u26A1";
    return s.c_str();
}
const wchar_t* CWirelessDeviceBatteryItem::GetItemValueSampleText() const { return L"100% \u26A1"; }

int CWirelessDeviceBatteryItem::GetItemWidthEx(void* /*hDC*/) const { return GetItemWidth(); }

void CWirelessDeviceBatteryItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    if (!pDC || w <= 8 || h <= 8) return;
    const DeviceSnapshot* d = g_plugin.GetDevice(m_index);
    if (!d) return;

    COLORREF brand = BrandColor(d->brand);
    COLORREF color = LowBatteryColor(brand, d->percent);
    COLORREF text = dark_mode ? RGB(255, 255, 255) : RGB(20, 20, 20);

    int markSize = (h - 2) < DEVICE_MARK_SIZE ? (h - 2) : DEVICE_MARK_SIZE;
    CRect markRect(x, y + (h - markSize) / 2, x + markSize, y + (h + markSize) / 2);
    DrawBrandMark(pDC, markRect, d->brand, color);

    int barX = x + markSize + 4;
    int barW = w - markSize - 10;
    int barH = ((h - 12) / 2) > 3 ? ((h - 12) / 2) : 3;
    int barY = y + 2;
    DrawTranslucentRect(pDC, CRect(barX, barY, barX + barW, barY + barH),
                        RGB(128, 128, 128), 48);
    int pct = d->percent < 0 ? 0 : (d->percent > 100 ? 100 : d->percent);
    if (d->percent >= 0 && pct > 0) {
        DrawTranslucentRect(pDC,
            CRect(barX, barY, barX + (barW * pct) / 100, barY + barH), color, 220);
    }

    CString label;
    if (d->percent < 0) label = L"unknown";
    else label.Format(L"%d%%%s", pct, d->charging ? L" \u26A1" : L"");
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(d->percent < 0 ? RGB(150, 150, 150) : text);
    CFont font;
    font.CreateFontW(-MulDiv(9, g_plugin.DPI(96), 96), 0, 0, 0, FW_SEMIBOLD,
                     FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    CFont* old = pDC->SelectObject(&font);
    CRect textRect(barX, barY + barH, barX + barW, y + h);
    pDC->DrawText(label, textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    pDC->SelectObject(old);
}
