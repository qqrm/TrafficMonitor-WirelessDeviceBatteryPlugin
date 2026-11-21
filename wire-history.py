root = "/home/qqrm/wb-plugin-ui"

# BatterySnapshot: add per-device history map keyed by device key
p = f"{root}/WirelessDeviceBattery.h"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace('#include <mutex>', '#include <mutex>\n#include <unordered_map>\n#include "BatteryHistory.h"')
s = s.replace('    std::vector<DeviceSnapshot> devices;\n    std::wstring tooltip;',
              '    std::vector<DeviceSnapshot> devices;\n    std::unordered_map<std::wstring, BatteryHistory> histories;\n    std::wstring tooltip;')
s = s.replace('    const DeviceSnapshot* GetDevice(size_t index) const;',
              '    const DeviceSnapshot* GetDevice(size_t index) const;\n    const BatteryHistory* GetHistory(const std::wstring& key) const;')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)

# main cpp: add history accessor + record points in FetchData
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace('size_t CWirelessDeviceBattery::DeviceCount() const { return VisibleCount(); }',
              'size_t CWirelessDeviceBattery::DeviceCount() const { return VisibleCount(); }\n\nconst BatteryHistory* CWirelessDeviceBattery::GetHistory(const std::wstring& key) const\n{\n    std::lock_guard<std::mutex> lock(m_snapshot_mutex);\n    auto it = m_snapshot.histories.find(key);\n    return it == m_snapshot.histories.end() ? nullptr : &it->second;\n}')
# in FetchData, before PublishSnapshot, copy histories from old snapshot and add current point
old = '    snap.available = true;\n    PublishSnapshot(std::move(snap));'
new = '''    snap.available = true;
    {
        std::lock_guard<std::mutex> lock(m_snapshot_mutex);
        snap.histories = m_snapshot.histories;   // preserve per-device history
    }
    {
        time_t now = time(nullptr);
        for (const auto& d : snap.devices)
            snap.histories[d.key].Add(static_cast<long long>(now), d.percent);
    }
    PublishSnapshot(std::move(snap));'''
assert old in s
s = s.replace(old, new)
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)

# item cpp: draw the sparkline behind the bar
p = f"{root}/WirelessDeviceBatteryItem.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace('#include <cmath>', '#include <cmath>\n#include "BatteryHistory.h"')
old = '''    int barX = x + markSize + 4;
    int barW = w - markSize - 10;
    int barH = ((h - 12) / 2) > 3 ? ((h - 12) / 2) : 3;
    int barY = y + 2;'''
new = '''    int barX = x + markSize + 4;
    int barW = w - markSize - 10;
    int barH = ((h - 12) / 2) > 3 ? ((h - 12) / 2) : 3;
    int barY = y + 2;

    // Real-time charge sparkline (4h window), same translucent style as the
    // agent-limit cards. The filled area uses the brand color (crimson when low).
    const BatteryHistory* hist = g_plugin.GetHistory(d->key);
    if (hist && barW > 8 && barH > 2)
    {
        auto points = hist->Window(time(nullptr), BatteryHistory::kWindowSeconds);
        if (points.size() >= 2)
        {
            const long long newest = points.back().timestamp_unix_seconds;
            const long long oldest = points.front().timestamp_unix_seconds;
            const double span = static_cast<double>((newest > oldest ? newest - oldest : 1));
            std::vector<POINT> poly;
            poly.reserve(points.size() + 2);
            poly.push_back({ barX, barY + barH });
            for (const auto& pt : points)
            {
                double t = static_cast<double>(pt.timestamp_unix_seconds - oldest) / span;
                if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
                int px = barX + static_cast<int>((barW - 1) * t + 0.5);
                int py = barY + barH - static_cast<int>((barH - 1) * (pt.percent / 100.0) + 0.5);
                if (py < barY) py = barY;
                poly.push_back({ px, py });
            }
            poly.push_back({ barX + barW - 1, barY + barH });
            DrawTranslucentPolygon(pDC, CRect(barX, barY, barX + barW, barY + barH + 1),
                                   poly, color, 200);
        }
    }'''
assert old in s
s = s.replace(old, new)
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("wired history")
