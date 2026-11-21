#pragma once
#include "PluginInterface.h"
#include "WirelessDeviceBatteryItem.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>

#define g_plugin CWirelessDeviceBattery::Instance()

struct SettingData
{
    std::vector<std::wstring> hidden_keys;   // VID:PID or stable keys to hide
    bool show_unknown{true};
    bool show_charging{true};
};

struct DeviceSnapshot
{
    std::wstring key;
    std::wstring name;
    std::wstring via;
    int percent{-1};                 // -1 = unknown, never 0
    bool charging{};
    bool asleep{};
    bool connected{};
    int brand{};                     // index into BrandAssets
};

struct BatterySnapshot
{
    std::vector<DeviceSnapshot> devices;
    std::wstring tooltip;
    bool available{};
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
    int DPI(int pixel) const;
    bool IsVisible(const std::wstring& key) const;

    SettingData m_setting_data;

    // Snapshot access (thread-safe).
    BatterySnapshot GetSnapshot() const;
    const DeviceSnapshot* GetDevice(size_t index) const;   // nullptr if hidden/out of range
    size_t DeviceCount() const;
    bool IsAvailable() const;
    const std::wstring& ConfigPath() const { return m_config_path; }
    void ApplyOptions(const SettingData& data);
    void RefreshDevices();   // force poll for the options dialog

private:
    static CWirelessDeviceBattery m_instance;
    std::vector<CWirelessDeviceBatteryItem> m_items;

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
    size_t VisibleCount() const;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
