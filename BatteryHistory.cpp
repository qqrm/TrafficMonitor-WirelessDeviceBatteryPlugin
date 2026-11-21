#include "pch.h"
#include "BatteryHistory.h"
#include <algorithm>

void BatteryHistory::Add(long long now, int percent)
{
    if (percent < 0 || percent > 100) return;
    if (!m_points.empty() && m_points.back().timestamp_unix_seconds == now)
    {
        m_points.back().percent = percent;
        return;
    }
    m_points.push_back({ now, percent });
    // Drop samples outside the window, then bound the buffer.
    const long long cutoff = now - kWindowSeconds;
    while (!m_points.empty() && m_points.front().timestamp_unix_seconds < cutoff)
        m_points.erase(m_points.begin());
    if (m_points.size() > kMaxPoints)
    {
        // Evenly decimate while keeping the newest sample.
        std::vector<BatteryHistoryPoint> kept;
        kept.reserve(kMaxPoints);
        const size_t last = m_points.size() - 1;
        for (size_t i = 0; i < kMaxPoints; ++i)
            kept.push_back(m_points[i * last / (kMaxPoints - 1)]);
        m_points.swap(kept);
    }
}

std::vector<BatteryHistoryPoint> BatteryHistory::Window(long long now, long long window_seconds) const
{
    std::vector<BatteryHistoryPoint> out;
    const long long cutoff = now - window_seconds;
    for (const auto& p : m_points)
        if (p.timestamp_unix_seconds >= cutoff && p.percent >= 0)
            out.push_back(p);
    return out;
}

void BatteryHistory::Clear()
{
    m_points.clear();
}
