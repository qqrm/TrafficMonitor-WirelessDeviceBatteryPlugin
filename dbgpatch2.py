root = "/home/qqrm/wb-plugin-ui"
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
# One universal debug log at a fixed path (independent of config dir).
s = s.replace('static void DebugLog(const std::wstring& dir, const std::wstring& msg)\n{\n    std::wofstream f(dir + L"wireless-plugin-debug.log", std::ios::app);\n    f << msg << L"\\n";\n}',
'''static void DebugLog(const std::wstring& msg)
{
    const wchar_t* p = L"C:\\\\Users\\\\Alex Cat\\\\AppData\\\\Local\\\\Temp\\\\opencode\\\\wireless-plugin-debug.log";
    std::wofstream f(p, std::ios::app);
    f << msg << L"\\n";
}''')
s = s.replace("DebugLog(m_config_path.substr(0, m_config_path.find_last_of(L'\\\\')), m);", "DebugLog(m);")
# Add log to DataRequired and OnInitialize
s = s.replace('void CWirelessDeviceBattery::DataRequired()\n{', 'void CWirelessDeviceBattery::DataRequired()\n{\n    DebugLog(L"DataRequired called");')
s = s.replace('void CWirelessDeviceBattery::OnInitialize(ITrafficMonitor* pApp)\n{', 'void CWirelessDeviceBattery::OnInitialize(ITrafficMonitor* pApp)\n{\n    DebugLog(L"OnInitialize called");')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("dbgpatch2")
