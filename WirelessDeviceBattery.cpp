#include "pch.h"
#include "WirelessDeviceBattery.h"
#include "OptionsDlg.h"
#include "wireless_battery_ffi.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "synchronization.lib")
#pragma comment(lib, "lib\\x64\\wireless_battery.lib")
#include <sstream>
#include <algorithm>
#include <fstream>
#include <ctime>


CWirelessDeviceBattery CWirelessDeviceBattery::m_instance;

CWirelessDeviceBattery::CWirelessDeviceBattery()
    : m_items{}
{
    for (int i = 0; i < 16; ++i) m_items.emplace_back(i);
}

CWirelessDeviceBattery::~CWirelessDeviceBattery()
{
    for (auto& kv : m_icons)
        if (kv.second) DestroyIcon(kv.second);
    m_icons.clear();
    if (m_gdiplus_token) Gdiplus::GdiplusShutdown(m_gdiplus_token);
    SaveConfig();
}

CWirelessDeviceBattery& CWirelessDeviceBattery::Instance() { return m_instance; }

IPluginItem* CWirelessDeviceBattery::GetItem(int index)
{
    if (index < 0) return nullptr;
    size_t n = DeviceCount();
    if (size_t(index) >= n || size_t(index) >= m_items.size()) return nullptr;
    return &m_items[size_t(index)];
}

const wchar_t* CWirelessDeviceBattery::GetTooltipInfo() { return GetSnapshot().tooltip.c_str(); }

void CWirelessDeviceBattery::DataRequired()
{
    DWORD now = GetTickCount();
    if (m_last_update != 0 && (now - m_last_update) < 60000) return;
    FetchData();
    m_last_update = now;
}

void CWirelessDeviceBattery::PublishSnapshot(BatterySnapshot snapshot)
{
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    m_snapshot = std::move(snapshot);
}

BatterySnapshot CWirelessDeviceBattery::GetSnapshot() const
{
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    return m_snapshot;
}

bool CWirelessDeviceBattery::IsVisible(const std::wstring& key) const
{
    return std::find(m_setting_data.hidden_keys.begin(), m_setting_data.hidden_keys.end(), key)
           == m_setting_data.hidden_keys.end();
}

size_t CWirelessDeviceBattery::VisibleCount() const
{
    size_t n = 0;
    for (const auto& d : m_snapshot.devices)
        if (IsVisible(d.key)
            && (m_setting_data.show_unknown || d.percent >= 0)
            && (m_setting_data.show_charging || !d.charging)) ++n;
    return n;
}

const DeviceSnapshot* CWirelessDeviceBattery::GetDevice(size_t index) const
{
    std::lock_guard<std::mutex> lock(m_snapshot_mutex);
    size_t n = 0;
    for (const auto& d : m_snapshot.devices) {
        if (!IsVisible(d.key)) continue;
        if (!m_setting_data.show_unknown && d.percent < 0) continue;
        if (!m_setting_data.show_charging && d.charging) continue;
        if (n++ == index) return &d;
    }
    return nullptr;
}

size_t CWirelessDeviceBattery::DeviceCount() const { return VisibleCount(); }

void CWirelessDeviceBattery::ApplyOptions(const SettingData& data)
{
    m_setting_data = data;
    SaveConfig();
}

void CWirelessDeviceBattery::RefreshDevices()
{
    m_last_update = 0;
    FetchData();
}

ITMPlugin::OptionReturn CWirelessDeviceBattery::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    RefreshDevices();
    CWnd* pParent = CWnd::FromHandle((HWND)hParent);
    COptionsDlg dlg(pParent);
    dlg.m_data = m_setting_data;
    dlg.m_snapshot = GetSnapshot();
    if (dlg.DoModal() == IDOK) {
        ApplyOptions(dlg.m_data);
        return OR_OPTION_CHANGED;
    }
    return OR_OPTION_UNCHANGED;
}

const wchar_t* CWirelessDeviceBattery::GetInfo(PluginInfoIndex index)
{
    static thread_local CString str;
    switch (index) {
    case TMI_NAME: return StringRes(IDS_PLUGIN_NAME).GetString();
    case TMI_DESCRIPTION: return StringRes(IDS_PLUGIN_DESCRIPTION).GetString();
    case TMI_AUTHOR: return L"Alexey Belikov";
    case TMI_COPYRIGHT: return L"GPL-3.0-or-later";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/qqrm/TrafficMonitor-WirelessDeviceBatteryPlugin";
    case TMI_VERSION: return L"1.1.0";
    default: break;
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

void* CWirelessDeviceBattery::GetPluginIcon() { return GetIcon(IDI_ICON1); }

void CWirelessDeviceBattery::LoadConfig(const std::wstring& config_dir)
{
    m_config_path = config_dir + L"WirelessDeviceBattery.ini";
    m_setting_data = SettingData{};
    std::wifstream in(m_config_path);
    if (!in) return;
    std::wstring line;
    bool in_section = false;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line == L"[devices]") { in_section = true; continue; }
        if (!line.empty() && line.front() == L'[') { in_section = false; continue; }
        auto eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring k = line.substr(0, eq), v = line.substr(eq + 1);
        if (in_section && k == L"hidden") {
            std::wstringstream ss(v);
            std::wstring key;
            while (std::getline(ss, key, L',')) if (!key.empty()) m_setting_data.hidden_keys.push_back(key);
        } else if (k == L"show_unknown") m_setting_data.show_unknown = (v == L"1" || v == L"true");
        else if (k == L"show_charging") m_setting_data.show_charging = (v == L"1" || v == L"true");
    }
}

void CWirelessDeviceBattery::SaveConfig() const
{
    if (m_config_path.empty()) return;
    std::wofstream out(m_config_path, std::ios::trunc);
    out << L"[general]\n";
    out << L"show_unknown=" << (m_setting_data.show_unknown ? L"1" : L"0") << L"\n";
    out << L"show_charging=" << (m_setting_data.show_charging ? L"1" : L"0") << L"\n";
    out << L"[devices]\n";
    out << L"hidden=";
    for (size_t i = 0; i < m_setting_data.hidden_keys.size(); ++i) {
        if (i) out << L",";
        out << m_setting_data.hidden_keys[i];
    }
    out << L"\n";
}

const CString& CWirelessDeviceBattery::StringRes(UINT id)
{
    auto iter = m_string_table.find(id);
    if (iter != m_string_table.end()) return iter->second;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    m_string_table[id].LoadString(id);
    return m_string_table[id];
}

HICON CWirelessDeviceBattery::GetIcon(UINT id)
{
    auto iter = m_icons.find(id);
    if (iter != m_icons.end()) return iter->second;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id),
                                   IMAGE_ICON, DPI(16), DPI(16), 0);
    m_icons[id] = hIcon;
    return hIcon;
}

int CWirelessDeviceBattery::DPI(int pixel) const
{
    if (m_app != nullptr) return m_app->GetDPI(ITrafficMonitor::DPI_TASKBAR) * pixel / 96;
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
    if (k == std::wstring::npos) return -1;
    size_t colon = obj.find(L':', k);
    if (colon == std::wstring::npos) return -1;
    size_t i = colon + 1;
    while (i < obj.size() && (obj[i] == L' ' || obj[i] == L'\t')) ++i;
    if (i < obj.size() && obj[i] == L'n') return -1;   // null
    int sign = 1;
    if (i < obj.size() && obj[i] == L'-') { sign = -1; ++i; }
    int val = 0;
    while (i < obj.size() && obj[i] >= L'0' && obj[i] <= L'9') { val = val * 10 + (obj[i] - L'0'); ++i; }
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
    while (i < obj.size() && (obj[i] == L' ' || obj[i] == L'\t')) ++i;
    return obj.compare(i, 4, L"true") == 0;
}

static int BrandOf(const std::wstring& name, const std::wstring& via, unsigned vid)
{
    std::wstring n = name;
    std::transform(n.begin(), n.end(), n.begin(), [](wchar_t c) { return towlower(c); });
    if (n.find(L"logitech") != std::wstring::npos || n.find(L"mx ") != std::wstring::npos) return 0;
    if (n.find(L"razer") != std::wstring::npos) return 1;
    if (n.find(L"keychron") != std::wstring::npos) return 2;
    if (n.find(L"flydigi") != std::wstring::npos || n.find(L"apex") != std::wstring::npos) return 3;
    if (n.find(L"mchose") != std::wstring::npos) return 4;
    if (vid == 0x046d) return 0;
    if (vid == 0x37d7) return 3;
    if (vid == 0x3434) return 2;
    if (vid == 0x3837) return 4;
    return 5;
}

void CWirelessDeviceBattery::FetchData()
{
    BatterySnapshot snap;
    char* json = wb_poll_devices_json();
    if (!json) {
        snap.available = false;
        snap.tooltip = L"poll failed";
        PublishSnapshot(std::move(snap));
        return;
    }
    std::string bytes(json);
    wb_free_string(json);
    std::wstring wjson = Utf8ToWide(bytes);

    // Split top-level objects of the JSON array.
    std::vector<std::wstring> objs;
    size_t pos = 0;
    while (true) {
        size_t s = wjson.find(L'{', pos);
        if (s == std::wstring::npos) break;
        size_t e = wjson.find(L'}', s);
        if (e == std::wstring::npos) break;
        objs.push_back(wjson.substr(s, e - s + 1));
        pos = e + 1;
    }

    for (const auto& obj : objs) {
        DeviceSnapshot d;
        d.name = ExtractString(obj, L"name");
        d.via = ExtractString(obj, L"via");
        d.key = ExtractString(obj, L"key");
        d.percent = ExtractInt(obj, L"percent");
        d.charging = ExtractBool(obj, L"charging");
        d.asleep = ExtractBool(obj, L"asleep");
        d.connected = obj.find(L"\"connected\":false") == std::wstring::npos
                   && obj.find(L"\"connected\": false") == std::wstring::npos;
        size_t vp = obj.find(L"\"vid\":");
        unsigned vid = 0;
        if (vp != std::wstring::npos) {
            size_t i = vp + 7;
            while (i < obj.size() && (obj[i] == L' ' || obj[i] == L'\t')) ++i;
            while (i < obj.size() && obj[i] >= L'0' && obj[i] <= L'9') { vid = vid * 10 + (obj[i] - L'0'); ++i; }
        }
        d.brand = BrandOf(d.name, d.via, vid);

        std::wstring line = d.name;
        if (d.percent < 0) line += L" unknown";
        else {
            line += L" " + std::to_wstring(d.percent) + L"%";
            if (d.asleep || !d.connected) line += L" (last known)";
        }
        if (d.charging) line += L" \u26A1";
        if (!snap.tooltip.empty()) snap.tooltip += L"\n";
        snap.tooltip += line;
        snap.devices.push_back(std::move(d));
    }
    snap.available = true;
    PublishSnapshot(std::move(snap));
}

extern "C" __declspec(dllexport) ITMPlugin* TMPluginGetInstance()
{
    return &CWirelessDeviceBattery::Instance();
}
