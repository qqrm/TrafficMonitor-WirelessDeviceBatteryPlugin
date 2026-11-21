root = "/home/qqrm/wb-plugin-ui"
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
if "DebugLog" not in s:
    s = s.replace('#include <fstream>', '#include <fstream>\n#include <ctime>\n\nstatic void DebugLog(const std::wstring& dir, const std::wstring& msg)\n{\n    std::wofstream f(dir + L"wireless-plugin-debug.log", std::ios::app);\n    f << msg << L"\\n";\n}')
    # add logging in FetchData after parse
    s = s.replace('    snap.available = true;\n    PublishSnapshot(std::move(snap));',
                  '    snap.available = true;\n    { std::wstring m = L"poll: devices=" + std::to_wstring(snap.devices.size()); DebugLog(m_config_path.substr(0, m_config_path.find_last_of(L\'\\\\\\\\\')), m); }\n    PublishSnapshot(std::move(snap));')
    # log in GetDevice when missing
    s = s.replace('    return nullptr;\n}\n\nsize_t CWirelessDeviceBattery::DeviceCount',
                  '    { std::wstring m = L"GetDevice miss index=" + std::to_wstring(index) + L" visible=" + std::to_wstring(VisibleCount()); DebugLog(m_config_path.substr(0, m_config_path.find_last_of(L\'\\\\\\\\\')), m); }\n    return nullptr;\n}\n\nsize_t CWirelessDeviceBattery::DeviceCount')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("dbgpatch")
