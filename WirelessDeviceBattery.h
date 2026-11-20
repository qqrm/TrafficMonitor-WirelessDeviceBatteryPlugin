#pragma once
#include "PluginInterface.h"
#include "WirelessDeviceBatteryItem.h"
#include <string>
#include <map>
#include <mutex>

#define g_plugin CWirelessDeviceBattery::Instance()

struct SettingData
{
};

struct BatterySnapshot
{
    int percent{};
    bool available{};
    bool asleep{};
    bool connected{};
    std::wstring value;
    std::wstring tooltip;
    std::wstring name;
};

class CWirelessDeviceBattery : public ITMPlugin
{
private:
    CWirelessDeviceBattery();
    ~CWirelessDeviceBattery();

public:
    static CWirelessDeviceBattery& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnInitialize(ITrafficMonitor* pApp) override;
    virtual void* GetPluginIcon() override;

    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;
    const CString& StringRes(UINT id);
    HICON GetIcon(UINT id);
    int DPI(int pixel);

    SettingData m_setting_data;

    // Thread-safe accessors. Return pointers into thread-local storage so the
    // caller (UI thread) never observes a partially updated std::wstring.
    const wchar_t* GetValue() const;
    const wchar_t* GetName() const;
    const wchar_t* GetTooltip() const;
    int GetPercent() const;
    bool IsAvailable() const;

private:
    static CWirelessDeviceBattery m_instance;
    CWirelessDeviceBatteryItem m_item;

    mutable std::mutex m_snapshot_mutex;
    BatterySnapshot m_snapshot;

    std::wstring m_config_path;
    std::map<UINT, CString> m_string_table;
    std::map<UINT, HICON> m_icons;
    ITrafficMonitor* m_app{};
    DWORD m_last_update{};
    ULONG_PTR m_gdiplus_token{};

    void FetchData();
    void PublishSnapshot(BatterySnapshot snapshot);
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif