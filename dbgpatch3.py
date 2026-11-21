root = "/home/qqrm/wb-plugin-ui"
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace('DebugLog(m_config_path.substr(0, m_config_path.find_last_of(L\'\\\\\')), m);', 'DebugLog(m);')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("dbgpatch3")
