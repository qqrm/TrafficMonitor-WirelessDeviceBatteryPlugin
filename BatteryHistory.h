#pragma once
#include <cstdint>
#include <vector>

// Per-device battery charge history, bounded and time-windowed like
// usage_core::history (better-trafficmonitor-ai-usage-plugin).
struct BatteryHistoryPoint
{
    long long timestamp_unix_seconds{};
    int percent{-1};                 // -1 = unknown, excluded from the trace
};

class BatteryHistory
{
public:
    static constexpr size_t kMaxPoints = 96;
    static constexpr long long kWindowSeconds = 4 * 60 * 60;   // 4 hours

    void Add(long long now, int percent);
    std::vector<BatteryHistoryPoint> Window(long long now, long long window_seconds) const;
    void Clear();

private:
    std::vector<BatteryHistoryPoint> m_points;
};
