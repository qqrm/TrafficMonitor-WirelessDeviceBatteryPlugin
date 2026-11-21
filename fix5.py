root = "/home/qqrm/wb-plugin-ui"
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
if 'ntdll.lib' not in s:
    s = s.replace('#pragma comment(lib, "gdiplus.lib")',
                  '#pragma comment(lib, "gdiplus.lib")\n#pragma comment(lib, "ntdll.lib")\n#pragma comment(lib, "synchronization.lib")')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("fix5")
