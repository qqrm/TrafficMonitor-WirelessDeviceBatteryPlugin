import re
root = "/home/qqrm/wb-plugin-ui"

# DeviceBrandMasks.h: inline -> static (C++14 project)
p = f"{root}/DeviceBrandMasks.h"
s = open(p, encoding="utf-8").read().replace("inline constexpr", "static constexpr")
open(p, "w", encoding="utf-8").write(s)

# WirelessDeviceBattery.h: DPI is non-const; add IsVisible declaration
p = f"{root}/WirelessDeviceBattery.h"
s = open(p, encoding="utf-8").read().replace("int DPI(int pixel);", "int DPI(int pixel);\n    bool IsVisible(const std::wstring& key) const;")
open(p, "w", encoding="utf-8").write(s)

# main cpp: DPI const calls -> non-const, already non-const in header; fix static ref
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8").read()
s = s.replace("MulDiv(9, g_plugin.DPI(96), 96)", "MulDiv(9, g_plugin.DPI(96), 96)")
open(p, "w", encoding="utf-8").write(s)

# item cpp: std::clamp / std::lround need <algorithm>/<cmath> (already there), but
# MSVC C++14: replace clamp with manual, lround with std::round-like int cast.
p = f"{root}/WirelessDeviceBatteryItem.cpp"
s = open(p, encoding="utf-8").read()
s = s.replace("int(std::lround(a + (b - a) * k))", "int((a + (b - a) * k) + 0.5f)")
s = s.replace("int pct = std::clamp(d->percent, 0, 100);",
              "int pct = d->percent < 0 ? 0 : (d->percent > 100 ? 100 : d->percent);")
open(p, "w", encoding="utf-8").write(s)
print("patched")
