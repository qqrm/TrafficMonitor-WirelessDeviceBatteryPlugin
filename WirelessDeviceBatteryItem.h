#pragma once
#include "PluginInterface.h"

class CWirelessDeviceBatteryItem : public IPluginItem
{
public:
    explicit CWirelessDeviceBatteryItem(int index) : m_index(index) {}

    const wchar_t* GetItemName() const override;
    const wchar_t* GetItemId() const override;
    const wchar_t* GetItemLableText() const override;
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override;

    bool IsCustomDraw() const override { return true; }
    int GetItemWidth() const override { return 132; }
    int GetItemWidthEx(void* hDC) const override;
    void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;

private:
    int m_index;
};
