import re
p = "/home/qqrm/wb-plugin-ui/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = re.sub(r'static void DebugLog\(const std::wstring& msg\)\n\{.*?\n\}\n', '', s, flags=re.S)
s = re.sub(r'\n    DebugLog\(L"[^"]*"\);', '', s)
s = re.sub(r'\n    \{ std::wstring m = L"poll: devices="[^;]+; DebugLog\(m\); \}', '', s)
s = re.sub(r'\n    \{ std::wstring m = L"GetDevice miss[^;]+; DebugLog\(m\); \}', '', s)
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("remaining DebugLog:", s.count("DebugLog"))
