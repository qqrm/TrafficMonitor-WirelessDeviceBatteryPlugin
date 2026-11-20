#include "pch.h"
#include "WirelessDeviceBatteryItem.h"
#include "WirelessDeviceBattery.h"
#include "PluginDraw.h"

const wchar_t* CWirelessDeviceBatteryItem::GetItemName() const { return L"Wireless Device Battery"; }
const wchar_t* CWirelessDeviceBatteryItem::GetItemId() const { return L"WirelessDeviceBattery"; }
const wchar_t* CWirelessDeviceBatteryItem::GetItemLableText() const { return g_plugin.GetName(); }
const wchar_t* CWirelessDeviceBatteryItem::GetItemValueText() const { return g_plugin.GetValue(); }
const wchar_t* CWirelessDeviceBatteryItem::GetItemValueSampleText() const { return L"100%"; }
int CWirelessDeviceBatteryItem::GetItemWidth() const { return 160; }
int CWirelessDeviceBatteryItem::GetItemWidthEx(void* hDC) const { return 160; }

void CWirelessDeviceBatteryItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    if (!pDC || w <= 0 || h <= 0) return;

    int percent = g_plugin.GetPercent();
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    COLORREF text_color = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);

    int logoSize = h - 2;
    DrawLogitechMark(pDC, CRect(x, y, x + logoSize, y + logoSize), RGB(205, 205, 205));

    int barX = x + logoSize + 4;
    int barW = w - logoSize - 4;
    int barH = h - 2;
    int barY = y + 1;
    int fillH = (barH * percent) / 100;
    DrawTranslucentRect(pDC, CRect(barX, barY + barH - fillH, barX + barW, barY + barH),
        RGB(255, 255, 255), 72);

    CString text;
    text.Format(L"%s %d%%", g_plugin.GetName(), percent);
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(text_color);
    CRect textRect(barX, y, barX + barW, y + h);
    pDC->DrawText(text, textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}