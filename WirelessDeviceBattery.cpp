#include "pch.h"
#include "WirelessDeviceBattery.h"
#include "OptionsDlg.h"
#include "wireless_battery_ffi.h"
#include <gdiplus.h>
#include <sstream>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "lib\\x64\\wireless_battery.lib")
#pragma comment(lib, "ntdll.lib")
#include <vector>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////
CWirelessDeviceBattery CWirelessDeviceBattery::m_instance;

CWirelessDeviceBattery::CWirelessDeviceBattery()
{
}

CWirelessDeviceBattery::~CWirelessDeviceBattery()
{
    for (auto& kv : m_icons)
        if (kv.second) DestroyIcon(kv.second);
    m_icons.clear();
    if (m_gdiplus_token)
        Gdiplus::GdiplusShutdown(m_gdiplus_token);
    SaveConfig();
}

CWirelessDeviceBattery& CWirelessDeviceBattery::Instance()
{
    return m_instance;
}

IPluginItem* CWirelessDeviceBattery::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* CWirelessDeviceBattery::GetTooltipInfo()
{
    return GetTooltip();
}

void CWirelessDeviceBattery::DataRequired()
{
    DWORD now = GetTickCount();
    if (m_last_update != 0 && (now - m_last_update) < 60000)
        return;
    FetchData();
    m_last_update = now;
}

void CWirelessDeviceBattery::PublishSnapshot(BatterySnapshot snapshot)
{
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    m_snapshot = std::move(snapshot);
}

const wchar_t* CWirelessDeviceBattery::GetValue() const
{
    static thread_local std::wstring cache;
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    cache = m_snapshot.value;
    return cache.c_str();
}

const wchar_t* CWirelessDeviceBattery::GetName() const
{
    static thread_local std::wstring cache;
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    cache = m_snapshot.name;
    return cache.c_str();
}

const wchar_t* CWirelessDeviceBattery::GetTooltip() const
{
    static thread_local std::wstring cache;
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    cache = m_snapshot.tooltip;
    return cache.c_str();
}

int CWirelessDeviceBattery::GetPercent() const
{
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    return m_snapshot.percent;
}

bool CWirelessDeviceBattery::IsAvailable() const
{
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    return m_snapshot.available;
}

ITMPlugin::OptionReturn CWirelessDeviceBattery::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWnd* pParent = CWnd::FromHandle((HWND)hParent);
    COptionsDlg dlg(pParent);
    dlg.m_data = m_setting_data;
    if (dlg.DoModal() == IDOK)
    {
        m_setting_data = dlg.m_data;
        SaveConfig();
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

const wchar_t* CWirelessDeviceBattery::GetInfo(PluginInfoIndex index)
{
    static thread_local CString str;
    switch (index)
    {
    case TMI_NAME:
        return StringRes(IDS_PLUGIN_NAME).GetString();
    case TMI_DESCRIPTION:
        return StringRes(IDS_PLUGIN_DESCRIPTION).GetString();
    case TMI_AUTHOR:
        return L"qqrm";
    case TMI_COPYRIGHT:
        return L"Copyright (C) 2026 qqrm";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/qqrm/TrafficMonitor-WirelessDeviceBatteryPlugin";
        break;
    case TMI_VERSION:
        return L"1.0.0";
    default:
        break;
    }
    return L"";
}

void CWirelessDeviceBattery::OnInitialize(ITrafficMonitor* pApp)
{
    m_app = pApp;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&m_gdiplus_token, &input, nullptr);
    std::wstring config_dir = pApp->GetPluginConfigDir();
    LoadConfig(config_dir);
}

void* CWirelessDeviceBattery::GetPluginIcon()
{
    return GetIcon(IDI_ICON1);
}

void CWirelessDeviceBattery::LoadConfig(const std::wstring& config_dir)
{
    m_config_path = config_dir + L"KeyboardIndicator.ini";
    }

void CWirelessDeviceBattery::SaveConfig() const
{
    }

const CString& CWirelessDeviceBattery::StringRes(UINT id)
{
    auto iter = m_string_table.find(id);
    if (iter != m_string_table.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_table[id].LoadString(id);
        return m_string_table[id];
    }
}

HICON CWirelessDeviceBattery::GetIcon(UINT id)
{
    auto iter = m_icons.find(id);
    if (iter != m_icons.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, DPI(16), DPI(16), 0);
        m_icons[id] = hIcon;
        return hIcon;
    }
}

int CWirelessDeviceBattery::DPI(int pixel)
{
    if (m_app != nullptr)
    {
        int dpi = m_app->GetDPI(ITrafficMonitor::DPI_TASKBAR);
        return dpi * pixel / 96;
    }
    return pixel;
}

static std::wstring Utf8ToWide(const std::string& s)
{
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    if (n > 0) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

static std::wstring ExtractString(const std::wstring& obj, const std::wstring& key)
{
    std::wstring search = L"\"" + key + L"\"";
    size_t k = obj.find(search);
    if (k == std::wstring::npos) return L"";
    size_t colon = obj.find(L':', k);
    if (colon == std::wstring::npos) return L"";
    size_t q1 = obj.find(L'"', colon);
    if (q1 == std::wstring::npos) return L"";
    size_t q2 = obj.find(L'"', q1 + 1);
    if (q2 == std::wstring::npos) return L"";
    return obj.substr(q1 + 1, q2 - q1 - 1);
}

static int ExtractInt(const std::wstring& obj, const std::wstring& key)
{
    std::wstring search = L"\"" + key + L"\"";
    size_t k = obj.find(search);
    if (k == std::wstring::npos) return 0;
    size_t colon = obj.find(L':', k);
    if (colon == std::wstring::npos) return 0;
    size_t i = colon + 1;
    while (i < obj.size() && (obj[i] == L' ' || obj[i] == L'\t')) i++;
    int sign = 1;
    if (i < obj.size() && obj[i] == L'-') { sign = -1; i++; }
    int val = 0;
    while (i < obj.size() && obj[i] >= L'0' && obj[i] <= L'9') { val = val * 10 + (obj[i] - L'0'); i++; }
    return val * sign;
}

static bool ExtractBool(const std::wstring& obj, const std::wstring& key)
{
    std::wstring search = L"\"" + key + L"\"";
    size_t k = obj.find(search);
    if (k == std::wstring::npos) return false;
    size_t colon = obj.find(L':', k);
    if (colon == std::wstring::npos) return false;
    size_t i = colon + 1;
    while (i < obj.size() && (obj[i] == L' ' || obj[i] == L'\t')) i++;
    return obj.compare(i, 4, L"true") == 0;
}

void CWirelessDeviceBattery::FetchData()
{
    BatterySnapshot snap;
    char* json = wb_poll_devices_json();
    if (json == nullptr)
    {
        snap.available = false;
        snap.value = L"poll failed";
        snap.tooltip = L"poll failed";
        snap.name = L"Device";
        snap.percent = 0;
        PublishSnapshot(std::move(snap));
        return;
    }
    std::string bytes(json);
    wb_free_string(json);

    std::wstring wjson = Utf8ToWide(bytes);
    std::vector<std::wstring> objs;
    size_t pos = 0;
    while (true)
    {
        size_t s = wjson.find(L'{', pos);
        if (s == std::wstring::npos) break;
        size_t e = wjson.find(L'}', s);
        if (e == std::wstring::npos) break;
        objs.push_back(wjson.substr(s, e - s + 1));
        pos = e + 1;
    }

    std::wstring result;
    std::wstring tooltip;
    bool first = true;
    for (const auto& obj : objs)
    {
        std::wstring name = ExtractString(obj, L"name");
        bool has_percent = obj.find(L"\"percent\":null") == std::wstring::npos
            && obj.find(L"\"percent\": null") == std::wstring::npos;
        int percent = has_percent ? ExtractInt(obj, L"percent") : -1;
        bool charging = ExtractBool(obj, L"charging");
        bool asleep = ExtractBool(obj, L"asleep");
        bool connected = obj.find(L"\"connected\":false") == std::wstring::npos
            && obj.find(L"\"connected\": false") == std::wstring::npos;

        std::wstring part;
        if (!has_percent || percent < 0)
            part = name + L" unknown";
        else if (asleep || !connected)
            part = name + L" " + std::to_wstring(percent) + L"% (last known)";
        else
            part = name + L" " + std::to_wstring(percent) + L"%";
        if (charging) part += L" \u26A1";

        if (!result.empty()) result += L", ";
        result += part;
        if (!tooltip.empty()) tooltip += L"\n";
        tooltip += part;

        if (first)
        {
            std::wstringstream ss(name);
            std::wstring w1, w2;
            ss >> w1 >> w2;
            snap.name = w1;
            if (!w2.empty()) snap.name += L" " + w2;
            snap.percent = (has_percent && percent >= 0) ? percent : 0;
            snap.available = has_percent && percent >= 0 && connected;
            snap.asleep = asleep;
            snap.connected = connected;
            first = false;
        }
    }
    if (result.empty()) result = L"No devices";
    if (tooltip.empty()) tooltip = result;

    snap.value = result;
    if (snap.available)
        snap.tooltip = snap.name + L" " + std::to_wstring(snap.percent) + L"%"
            + (snap.asleep || !snap.connected ? L" (last known)" : L"");
    else
        snap.tooltip = result;
    PublishSnapshot(std::move(snap));
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CWirelessDeviceBattery::Instance();
}
