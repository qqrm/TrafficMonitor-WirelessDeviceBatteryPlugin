import re
p = "/home/qqrm/wb-plugin-ui/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
# Remove any leftover DebugLog calls/definition (the source still has them; commit was on a stale copy)
s = re.sub(r'static void DebugLog\(const std::wstring& msg\)\n\{.*?\n\}\n', '', s, flags=re.S)
s = re.sub(r'[ \t]*DebugLog\(L"[^"]*"\);\n', '', s)
s = re.sub(r'[ \t]*\{ std::wstring m = L"poll: devices="[^;]+; DebugLog\(m\); \}\n', '', s)
s = re.sub(r'[ \t]*\{ std::wstring m = L"GetDevice miss[^;]+; DebugLog\(m\); \}\n', '', s)
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("remaining", s.count("DebugLog"))
